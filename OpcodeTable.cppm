module;

#define OPCODE_TABLE_PATH "OpcodeTable.txt"

#include <cstdint>
#include <fstream>
#include <iosfwd>
#include <print>
#include <unordered_map>

export module Opcode_Table_M;
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
    // i'm going high budget here, singleton pattern

    static const OpcodeTable& get_instance()
    {
        static OpcodeTable instance(OPCODE_TABLE_PATH);
        return instance;
    }

    Instruction decode_opcode(uint8_t opcode)
    {
        // super duper unsafe
        return map[opcode];
    }
private:
    OpcodeTable(char* path)
    {
        std::ifstream stream = std::ifstream(path);
        std::string token;
        uint8_t token_type = 0;
        Instruction instr;
        while (stream >> token)
        {
            switch (token_type)
            {
                // opcode
            case 0:
                {
                    instr.opcode = std::stoi(token, nullptr, 16);
                    break;
                }
                // mnemonic
            case 1:
                {
                    // kill me now
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
                        { "BNE", BNE },
                        { "BPL", BPL },
                        { "BMI", BMI },
                        { "BVC", BVC },
                        { "BVS", BVS },
                        { "JMP", JMP },
                        { "JSR", JSR },
                        { "RTS", RTS },
                        { "BRK", BRK },
                        { "RTI", RTI },
                        { "PHA", PHA },
                        { "PLA", PLA },
                        { "PHP", PHP },
                        { "PLP", PLP },
                        { "TXS", TXS },
                        { "TSX", TSX },
                        { "CLC", CLC },
                        { "SEC", SEC },
                        { "CLI", CLI },
                        { "SEI", SEI },
                        { "CLD", CLD },
                        { "SED", SED },
                        { "CLV", CLV },
                        { "NOP", NOP }
                    };
                    auto mnemonic = mnemonic_map[token];
                    // they're saying its the least safe code ever made
                    instr.mnemonic = mnemonic;
                    break;
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
                    instr.addressing_mode = mode;
                    break;
                }

            default: { break; }
            }


            token_type++;
            if (token_type == 6)
            {
                token_type = 0;
                map[instr.opcode] = instr;
            }
        }
    }
};