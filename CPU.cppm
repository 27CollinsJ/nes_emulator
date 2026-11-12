//
// Created by jonah on 01/10/2026.
//

module;

#include <cstdint>
#include <print>



export module CPU_M;

import OPCODE_TABLE_M;
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

    uint16_t retreived_operand;

    uint16_t temporary_bytes[16];

    uint8_t read(uint16_t address)
    {
        return bus.read(address);
    }

    static Instruction decode_opcode(uint8_t opcode)
    {
        return OPCODES[(opcode >> 4) & 0x0F][opcode & 0x0F];
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
            currentInstruction = decode_opcode(read_PC());
            // this takes the full cycle so skip the rest
            break;
        }
        default: std::println(stderr, "Not implemented");
        }
    }

};