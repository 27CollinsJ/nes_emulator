#include <iostream>
#include <memory>
#include <print>

import WRAM_M;
import CPUBus_M;
import CPU_M;
import Cartridge_M;
import Opcode_Table_M;

int main()
{
    auto loaded_cart = CartridgeFactory::from_assembly_file("test.S");
    if (loaded_cart == nullptr)
    {
        std::println(stderr, "Failed to load cart");
    }
    auto cart = std::move(loaded_cart);
    auto wram = WRAM();
    auto bus = CPUBus(wram, *cart);

    auto cpu = CPU(bus);

    cpu.init();

    for (int i = 0; i < 400; i++)
    {
        cpu.cycle();
    }

}