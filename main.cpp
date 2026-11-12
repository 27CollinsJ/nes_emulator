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
    if (!loaded_cart.has_value())
    {
        std::println(stderr, "Failed to load cart");
    }
    auto smb = loaded_cart.value();

    auto table = OpcodeTable::getInstance();


}