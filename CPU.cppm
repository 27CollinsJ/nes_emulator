//
// Created by jonah on 01/10/2026.
//

module;

#include <cstdint>
#include <future>
#include <print>


export module CPU_M;

import Opcode_Table_M;
import CPUBus_M;

enum StatusRegisterMask
{
    LAST_OP_CARRY = 0x01,
    LAST_OP_ZERO = 0x02,
    INTERRUPT_DISABLE = 0x04,
    DECIMAL_MODE_UNUSED = 0x08,
    BRK_CAUSED_INTERRUPT = 0x10,
    UNUSED_ALWAYS_ON = 0x20,
    LAST_OP_OVERFLOW = 0x40,
    LAST_OP_NEGATIVE = 0x80
};


enum CPUState
{
    FETCH,
    ADDRESS,
    EXECUTE
};


export class CPU
{
    uint8_t A = 0;
    uint8_t X = 0;
    uint8_t Y = 0;
    uint8_t SP = 0;
    uint8_t P = 0;
    uint16_t PC = 0;

    CPUBus& bus;

    Instruction currentInstruction = {};
    CPUState state = FETCH;
    // used for instructions where the current state part needs multiple cycles
    uint8_t microstate = 0;

    uint8_t retreived_operand = 0;
    // easier to just store both of these
    uint16_t retreived_address = 0;

    uint16_t temporary_bytes[16] = {};

    bool was_page_cross = false;

    uint8_t read(uint16_t address)
    {
        return bus.read(address);
    }

public:
    CPU(CPUBus& bus) : bus(bus)
    {
    }

    void init()
    {
        uint16_t resetVectorHigh = read(0xFFFD) << 8;
        uint8_t resetVectorLow = read(0xFFFC);
        PC = resetVectorHigh | resetVectorLow;
        state = FETCH;
        microstate = 0;
    }

    uint8_t read_PC()
    {
        uint8_t return_value = read(PC);
        PC++;
        return return_value;
    }

    void cycle()
    {
        switch (state)
        {
        case FETCH:
            {
                OpcodeTable& table = OpcodeTable::get_instance();

                currentInstruction = table.decode_opcode(read_PC());
                // this takes the full cycle so skip the rest
                state = ADDRESS;
                break;
            }
        case ADDRESS:
            {
                auto mode_ptr = mode_lookup(currentInstruction.addressing_mode);
                // i hate this syntax
                // and thats with ligatures, probably even worse with standard monospace
                if (!(this->*mode_ptr)())
                {
                    microstate++;
                    break;
                }
                // it returned true so we're fine to fall through to the next bit
                // NI though so back to fetch
                state = EXECUTE;
                microstate = 0;
            }
        // same as above
        case EXECUTE:
            {
                auto mnemonic_ptr = mnemonic_lookup(currentInstruction.mnemonic);
                if (!(this->*mnemonic_ptr)())
                {
                    microstate++;
                    break;
                }
                state = FETCH;
                microstate = 0;
                break;
            }
        default: { std::println(stderr, "Not implemented"); }
        }
    }

private:
    // every instruction implementation returns false when the cycle needs to end and it still needs to continue
    // when it returns true we move on to the next step in execution
    // most of these are entirely inaccurate bus-wise, but who cares?

    bool implicit()
    {
        // nothing to see here, move along
        return true;
    }

    bool accumulator()
    {
        // nothing ever happens ig
        return true;
    }

    bool immediate()
    {
        // call the happening police
        // grab the first byte after the instruction
        retreived_operand = read_PC();
        return true;
    }

    bool zero_page()
    {
        switch (microstate)
        {
        case 0:
            {
                retreived_address = read_PC();
                retreived_operand = read(retreived_address);
                return false;
            }
        default:
            return true;
        }
    }

    bool zero_page_x()
    {
        switch (microstate)
        {
        case 0:
            {
                retreived_address = read_PC();
                return false;
            }
        case 1:
            {
                retreived_address = (retreived_address + X) & 0xFF;
                retreived_operand = read(retreived_address);
                return false;
            }
        default:
            return true;
        }
    }

    bool zero_page_y()
    {
        switch (microstate)
        {
        case 0:
            {
                retreived_address = read_PC();
                return false;
            }
        case 1:
            {
                retreived_address = (retreived_address + Y) & 0xFF;
                retreived_operand = read(retreived_address);
                return false;
            }
        default:
            return true;
        }
    }

    bool absolute()
    {
        switch (microstate)
        {
        case 0:
            {
                retreived_address = read_PC();
                return false;
            }
        case 1:
            {
                retreived_address = retreived_address | (read_PC() << 8);
                return false;
            }
        default:
            {
                retreived_operand = read(retreived_address);
                return true;
            }
        }
    }

    bool absolute_x()
    {
        switch (microstate)
        {
        case 0:
            {
                uint8_t low = read_PC();
                temporary_bytes[0] = 0;
                if (low + X > 0xFF)
                {
                    // marker to skip a cycle
                    temporary_bytes[0] = 1;
                }
                // we need to skip the cycle if its a write too
                OpcodeTable& table = OpcodeTable::get_instance();
                if (table.is_write(currentInstruction.mnemonic))
                {
                    temporary_bytes[0] = 1;
                }
                retreived_address = low;
                return false;
            }
        case 1:
            {
                retreived_address = (retreived_address | (read_PC() << 8)) + X;
                return false;
            }
        default:
            {
                if (temporary_bytes[0] == 1)
                {
                    temporary_bytes[0] = 0;
                    return false;
                }
                retreived_operand = read(retreived_address);
                return true;
            }
        }
    }

    bool absolute_y()
    {
        switch (microstate)
        {
        case 0:
            {
                uint8_t low = read_PC();
                temporary_bytes[0] = 0;
                if (low + Y > 0xFF)
                {
                    // marker to skip a cycle
                    temporary_bytes[0] = 1;
                }
                // we need to skip the cycle if its a write too
                OpcodeTable& table = OpcodeTable::get_instance();
                if (table.is_write(currentInstruction.mnemonic))
                {
                    temporary_bytes[0] = 1;
                }
                retreived_address = low;
                return false;
            }
        case 1:
            {
                retreived_address = (retreived_address | (read_PC() << 8)) + Y;
                return false;
            }
        default:
            {
                if (temporary_bytes[0] == 1)
                {
                    temporary_bytes[0] = 0;
                    return false;
                }
                retreived_operand = read(retreived_address);
                return true;
            }
        }
    }

    bool relative()
    {
        retreived_operand = read_PC();
        return true;
    }

    bool indirect()
    {
        switch (microstate)
        {
        case 0:
            {
                retreived_address = read_PC();
                return false;
            }
        case 1:
            {
                retreived_address |= read_PC() << 8;
                return false;
            }
        case 2:
            {
                temporary_bytes[0] = read(retreived_address);
                return false;
            }
        default:
            {
                // the nes was poorly designed and is full of bugs. this bit fixes the page wrap bug
                retreived_address = (retreived_address & 0xFF00) | ((retreived_address + 1) & 0x00FF);
                retreived_address = read(retreived_address) << 8 | temporary_bytes[0];
                return true;
            }
        }
    }

    // the others are harder and im lazy, not doing it yet


    bool lda()
    {
        A = retreived_operand;
        if (A == 0x00) { P = P | 0x02; }
        if ((A & 0x80) != 0) { P = P | 0x80; }
        return true;
    }

    bool ldx()
    {
        X = retreived_operand;
        if (X == 0x00) { P = P | 0x02; }
        if ((X & 0x80) != 0) { P = P | 0x80; }
        return true;
    }

    bool ldy()
    {
        Y = retreived_operand;
        if (Y == 0x00) { P = P | 0x02; }
        if ((Y & 0x80) != 0) { P = P | 0x80; }
        return true;
    }

    bool tax()
    {
        X = A;

        if (A == 0x00) { P = P | 0x02; }
        if ((A & 0x80) != 0) { P = P | 0x80; }

        return true;
    }

    bool txa()
    {
        A = X;

        if (A == 0x00) { P = P | 0x02; }
        if ((A & 0x80) != 0) { P = P | 0x80; }

        return true;
    }

    bool tay()
    {
        Y = A;

        if (A == 0x00) { P = P | 0x02; }
        if ((A & 0x80) != 0) { P = P | 0x80; }

        return true;
    }

    bool tya()
    {
        A = Y;

        if (A == 0x00) { P = P | 0x02; }
        if ((A & 0x80) != 0) { P = P | 0x80; }

        return true;
    }

    bool sta()
    {
        bus.write(retreived_address, A);
        return true;
    }

    bool stx()
    {
        bus.write(retreived_address, X);
        return true;
    }

    bool sty()
    {
        bus.write(retreived_address, Y);
        return true;
    }

    bool nop()
    {
        return true;
    }

    bool sei()
    {
        // set interrupt disable flag
        P = P | 0x04;
        return true;
    }

    bool jmp()
    {
        PC = retreived_address;
        return true;
    }


    bool cal()
    {
        switch (retreived_operand)
        {
        case 1:
            {
                std::println("A: {}", A);
                break;
            }
        case 2:
            {
                std::println("X: {}", X);
                break;
            }
        case 3:
            {
                std::println("Y: {}", Y);
                break;
            }
        default: { std::println(stderr, "Invalid syscall {}", retreived_operand); }
        }
        return true;
    }

    // ahh scary function pointers
    // takes an addressing mode enum and gets the right function
    static bool (CPU::* mode_lookup(AddressingMode mode))()
    {
        switch (mode)
        {
        case IMPLICIT: { return &CPU::implicit; }
        case ACCUMULATOR: { return &CPU::accumulator; }
        case IMMEDIATE: { return &CPU::immediate; }
        case ZERO_PAGE: { return &CPU::zero_page; }
        case ABSOLUTE: { return &CPU::absolute; }
        case ZERO_PAGE_X: { return &CPU::zero_page_x; }
        case ZERO_PAGE_Y: { return &CPU::zero_page_y; }
        case ABSOLUTE_X: { return &CPU::absolute_x; }
        case ABSOLUTE_Y: { return &CPU::absolute_y; }

        default:
            {
                std::println(stderr, "Not implemented");
                return &CPU::implicit;
            }
        }
    }

    // same again
    static bool (CPU::* mnemonic_lookup(Mnemonic mnemonic))()
    {
        switch (mnemonic)
        {
        case LDA: { return &CPU::lda; }
        case LDX: { return &CPU::ldx; }
        case LDY: { return &CPU::ldy; }
        case TAX: { return &CPU::tax; }
        case TXA: { return &CPU::txa; }
        case TAY: { return &CPU::tay; }
        case TYA: { return &CPU::tya; }

        case SEI: { return &CPU::sei; }

        case NOP: { return &CPU::nop; }
        case CAL: { return &CPU::cal; }
        default:
            {
                std::println(stderr, "Not implemented");
                return &CPU::nop;
            }
        }
    }
};
