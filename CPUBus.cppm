//
// Created by jonah on 01/10/2026.
//
module;

#include <cstdint>
#include <memory>

export module CPUBus_M;

import WRAM_M;
import Cartridge_M;


enum class DataUnit
{
    WRAM,
    CARTRIDGE,
    PPU_REGISTERS,
    APU_IO_DMA,
};

struct AddressDestination
{
    DataUnit unit;
    uint16_t address;
};

export class CPUBus
{
    WRAM& wram;
    std::shared_ptr<Cartridge> cart;

    static AddressDestination decode_address(uint16_t const & address)
    {
        if (address < 0x2000)
        {
            // The address is in WRAM somewhere
            // WRAM is 2KB repeated 4 times
            // Modulus it to get the real address
            // some moron made any operation on a uint16_t automatically promote to an int
            // i hope they have a bad day
            return {DataUnit::WRAM, static_cast<uint16_t>(address % 0x800)};
        } else if (address < 0x4000)
        {
            // address is into ppu registers
            // genuinely why is there like fifty billion repeats of these things
            return {DataUnit::PPU_REGISTERS, static_cast<uint16_t>((address - 0x2000) % 0x0008)};
        } else if (address < 0x4020)
        {
            // This is for the APU, I/O, or DMA
            // They just chucked a load of shit in this gap between cartridge and ppu
            // Probably going to have to sort this out into multiple sections later but its
            // complicated, eg $4017 is both controller 2 and manages the APU frame counter
            return {DataUnit::APU_IO_DMA, static_cast<uint16_t>(address - 0x4000)};
        } else
        {
            // its on the cart somewhere. Mappers like to have the real address not the offset one
            return {DataUnit::CARTRIDGE, address};
        }
    }

public:
    // i don't like c++
    explicit CPUBus(WRAM& wram) : wram(wram){}

    uint8_t read(uint16_t address)
    {
        AddressDestination dest = decode_address(address);
        // i'm lazy i'm not implementing this for stuff thats not wram yet
        if (dest.unit == DataUnit::WRAM)
        {
            return wram.read_byte(dest.address);
        } else if (dest.unit == DataUnit::CARTRIDGE)
        {
            return cart->read_prg(address);
        }
        else {return 0;}
    }
    void write(uint16_t address, uint8_t value)
    {
        AddressDestination dest = decode_address(address);
        if (dest.unit == DataUnit::WRAM)
        {
            wram.write_byte(dest.address, value);
        } else if (dest.unit == DataUnit::CARTRIDGE)
        {
            cart->write_prg(address, value);
        }
    }
    void swap_cart(std::shared_ptr<Cartridge> cart)
    {
        this->cart = cart;
    }
};