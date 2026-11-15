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
    auto loaded_cart = CartridgeFactory::from_file("SMB.nes");
    if (loaded_cart == nullptr)
    {
        std::println(stderr, "Failed to load cart");
    }
    auto smb = std::move(loaded_cart);
    auto wram = WRAM();
    auto bus = CPUBus(wram, *smb);

    auto cpu = CPU(bus);

    cpu.init();

    for (int i = 0; i < 400; i++)
    {
        cpu.cycle();
    }

}