module;

#include <cstdint>
#include <fstream>
#include <iosfwd>
#include <print>
#include <unordered_map>

export module OPCODE_TABLE_M;
export enum Mnemonic
{
     LDA, STA, LDX, STX, LDY, STY, // Access
     TAX, TXA, TAY, TYA, // Transfer
     ADC, SBC, INC, DEC, INX, DEX, INY, DEY, // Arithmetic
     ASL, LSR, ROL, ROR, // Shift
     AND, ORA, EOR, BIT, // Bitwise
     CMP, CPX, CPY, // Compare
     BCC, BCS, BEQ, BNE, BPL, BMI, BVC, BVS, // Branch
     JMP, JSR, RTS, BRK, RTI, // Jump
     PHA, PLA, PHP, PLP, TXS, TSX, // Stack
     CLC, SEC, CLI, SEI, CLD, SED, CLV, // Flags
     NOP // Other
};

export enum AddressingMode
{
    ZERO_PAGE_X, ZERO_PAGE_Y,
    ABSOLUTE_X, ABSOLUTE_Y,
    INDIRECT_X, INDIRECT_Y, // SCARY
    IMPLICIT,
    ACCUMULATOR,
    IMMEDIATE,
    ZERO_PAGE,
    ABSOLUTE,
    RELATIVE,
    INDIRECT
};

export struct Instruction
{
    uint8_t opcode;
    Mnemonic mnemonic;
    AddressingMode addressing_mode;
};

export class OpcodeTable
{
    std::unordered_map<uint8_t, Instruction> map;




public:
    OpcodeTable(char* path)
    {
        std::unordered_map<uint8_t, Instruction> map = std::unordered_map<uint8_t, Instruction>;
        std::ifstream stream = std::ifstream(path);
        std::string token;
        uint8_t token_type = 0;

        uint8_t code;
        Instruction instr;
        while (stream >> token)
        {
            switch (token_type)
            {
                // opcode
            case 0:
                {
                    code = stoi(token);
                    break;
                }
                // mnemonic
            case 1:
                {
                    static std::unordered_map<std::string, Mnemonic> mnemonic_map = {
                        { "LDA", LDA },
                        { "STA", STA },
                        { "LDX", LDX },
                        { "STX", STX },
                        { "LDY", LDY },
                        { "STY", STY },
                        { "TAX", TAX },
                        { "TXA", TXA },
                        { "TAY", TAY },
                        { "TYA", TYA },
                        { "ADC", ADC },
                        { "SBC", SBC },
                        { "INC", INC },
                        { "DEC", DEC },
                        { "INX", INX },
                        { "DEX", DEX },
                        { "INY", INY },
                        { "DEY", DEY },
                        { "ASL", ASL },
                        { "LSR", LSR },
                        { "ROL", ROR },
                        { "ROR", ROR },
                        { "AND", AND },
                        { "ORA", ORA },
                        { "EOR", EOR },
                        { "BIT", BIT },
                        { "CMP", CMP },
                        { "CPX", CPX },
                        { "CPY", CPY },
                        { "BCC", BCC },
                        { "BCS", BCS },
                        { "BEQ", BEQ },







                    };
                }
            // mode
            case 2:
                {
                    static std::unordered_map<std::string, AddressingMode> mode_map = {
                        { "imp", IMPLICIT},
                        { "(ind,X)", INDIRECT_X},
                        { "zpg", ZERO_PAGE },
                        { "#", IMMEDIATE },
                        { "A", ACCUMULATOR },
                        { "abs", ABSOLUTE },
                        { "rel", RELATIVE },
                        { "(ind),Y", INDIRECT_Y },
                        { "zpg,X", ZERO_PAGE_X },
                        { "abs,Y", ABSOLUTE_Y },
                        { "abs,X", ABSOLUTE_X },
                        { "(ind)", INDIRECT },
                        { "zpg,Y", ZERO_PAGE_Y}
                    };
                    auto mode = mode_map[token];
                }

            default: { continue; }
            }



            token_type++;
            if (token_type == 6)
            {
                token_type = 0;
            }
        }
    }
};