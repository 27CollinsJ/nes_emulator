//
// Created by jonah on 01/10/2026.
//

module;

#include <cstdint>
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

    Instruction currentInstruction;
    CPUState state;
    // used for instructions where the current state part needs multiple cycles
    uint8_t microstate;

    uint8_t retreived_operand;
    // easier to just store both of these
    uint16_t retreived_address;

    uint16_t temporary_bytes[16];

    uint8_t read(uint16_t address)
    {
        return bus.read(address);
    }

public:
    CPU(CPUBus& bus) : bus(bus){}

    void init()
    {
        uint16_t resetVectorHigh = read(0xFFFD) << 8;
        uint8_t resetVectorLow = read(0xFFFC);
        PC = resetVectorHigh | resetVectorLow;
        state = FETCH;
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
        case FETCH: {
            OpcodeTable table = OpcodeTable::get_instance();
            currentInstruction = table.decode_opcode(read_PC());
            // this takes the full cycle so skip the rest
            break;
        }
        default: { std::println(stderr, "Not implemented"); }
        }
    }

private:
    // every instruction implementation returns false when the cycle needs to end and it still needs to continue
    // when it returns true we move on to the next step in execution


    bool implied()
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
        switch (microstate) {
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
        case 2:
            {
                retreived_operand = read(retreived_address);
            }
        }
    }
    // the others are harder and im lazy, not doing it yet



    bool lda()
    {
        A = retreived_operand;
        if (A == 0x00) { P = P | 0x02; }
        if (A & 0x80 != 0) { P = P | 0xF0; }
        return true;
    }

    bool ldx()
    {
        X = retreived_operand;
        if (X == 0x00) { P = P | 0x02; }
        if (X & 0x80 != 0) { P = P | 0xF0; }
        return true;
    }

    bool ldy()
    {
        Y = retreived_operand;
        if (Y == 0x00) { P = P | 0x02; }
        if (Y & 0x80 != 0) { P = P | 0xF0; }
        return true;
    }

    bool tax()
    {
        X = A;

        if (A == 0x00) { P = P | 0x02; }
        if (A & 0x80 != 0) { P = P | 0xF0; }

        return true;
    }

    bool txa()
    {
        A = X;

        if (A == 0x00) { P = P | 0x02; }
        if (A & 0x80 != 0) { P = P | 0xF0; }

        return true;
    }

    bool tay()
    {
        Y = A;

        if (A == 0x00) { P = P | 0x02; }
        if (A & 0x80 != 0) { P = P | 0xF0; }

        return true;
    }

    bool tya()
    {
        A = Y;

        if (A == 0x00) { P = P | 0x02; }
        if (A & 0x80 != 0) { P = P | 0xF0; }

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

};