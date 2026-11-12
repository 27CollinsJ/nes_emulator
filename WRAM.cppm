//
// Created by jonah on 01/10/2026.
//

module;

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <vector>

export module WRAM_M;

export class WRAM
{
    std::vector<uint8_t> data = std::vector<uint8_t>(2048);
    public:
    uint8_t read_byte(uint16_t addr)
    {
        return data[addr];
    }
    void write_byte(uint16_t addr, uint8_t value)
    {
        data[addr] = value;
    }
    void fill_random_noise()
    {
        int seed = static_cast<uint32_t>(std::time(nullptr));
        std::srand(seed);
        for (uint8_t& element : data)
        {
            element = std::rand() % 256;
        }
    }
};
