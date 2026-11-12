//
// Created by jonah on 02/10/2026.
//

module;

#include <cstdint>
#include <fstream>
#include <iosfwd>
#include <memory>
#include <vector>
#include <print>
#include <optional>
#include <string>

export module CARTRIDGE_M;

enum ConsoleType
{
    NES_FAMICOM = 0,
    NINTENDO_VS_SYSTEM,
    NINTENDO_PLAYCHOICE_10,
    EXTENDED_CONSOLE_TYPE
};

export enum MapperName
{
    NROM = 0,
    UNROM = 2,
    MMC1 = 1,
    MMC3 = 4,
    AXROM = 7
};

enum NametableArrangement
{
    VERTICALLY_MIRRORED,
    HORIZONTALLY_MIRRORED,
    FOUR_SCREEN_LAYOUT,
    MAPPER_CONTROLLED
};


struct CartridgeHeader
{
    uint16_t prg_rom_banks;
    uint16_t chr_rom_banks;
    ConsoleType console_type;
    bool has_battery_save;
    NametableArrangement nametable_arrangement;
};

export class Cartridge
{
public:
    virtual ~Cartridge() = default;
    virtual uint8_t read_prg(uint16_t address) const = 0;
    virtual void write_prg(uint16_t address, uint8_t value) = 0;
};

export class Cartridge_NROM : public Cartridge
{
private:
    std::vector<uint8_t> prg_rom;
    std::vector<uint8_t> prg_ram;
    uint16_t prg_rom_bank_1_start = 0x0;
    uint16_t prg_rom_bank_2_start;
    uint16_t prg_ram_mask;
public:
    Cartridge_NROM(uint16_t prg_rom_banks, uint16_t prg_ram_banks, std::vector<uint8_t> prg_bytes)
    {
        if (prg_rom_banks == 1)
        {
            prg_rom_bank_2_start = prg_rom_bank_1_start;
            prg_rom = std::vector<uint8_t>(0x4000);
        } else if (prg_rom_banks == 2)
        {
            prg_rom_bank_2_start = prg_rom_bank_1_start + 0x4000;
            prg_rom = std::vector<uint8_t>(0x8000);
        } else { std::println(stderr, "NROM can only have 2 PRG_ROM banks"); }

        if (prg_ram_banks == 0)
        {
            // dummy vector
            prg_ram = std::vector<uint8_t>(0x0001);
            prg_ram_mask = 0x0;
        } else if (prg_ram_banks == 1)
        {
            prg_ram = std::vector<uint8_t>(0x0800);
            prg_ram_mask = 0x07FF;
            // This line breaks family basic, but who plays that?
        } else { std::println(stderr, "NROM can only have 1 PRG_RAM banks"); }

        prg_rom = std::move(prg_bytes);
    }
    uint8_t read_prg(uint16_t address) const override
    {
        if (address <= 0x6000)
        {
            // it shouldn't be here ignore it
            return -1;
        }
        if (address <= 0x7FFF)
        {
            // this is PRG-RAM
            // this puts the address into the range for prg_ram then accounts for the mirroring
            return prg_ram[(address - 0x6000) & prg_ram_mask];
        } else if (address <= 0xBFFF)
        {
            // bank 1 of PRG-ROM
            return prg_rom[prg_rom_bank_1_start + address - 0x8000];
        } else if (address <= 0xFFFF)
        {
            return prg_rom[prg_rom_bank_2_start + address - 0xC000];
        }
        std::println(stderr, "Tried to get data from the cartridge above 0xFFFF");
        return -1;
    }
    void write_prg(uint16_t address, uint8_t value) override
    {
        if (address <= 0x7FFF)
        {
            // this is PRG-RAM
            // this puts the address into the range for prg_ram then accounts for the mirroring
            prg_ram[(address - 0x6000) & prg_ram_mask] = value;
        } else
        {
            std::println(stderr, "Tried to write data to the cartridge above 0x7FFF");
        }
    }
};

export class CartridgeFactory
{
public:
    static std::optional<std::shared_ptr<Cartridge>> from_file(const std::string& filename)
    {
        std::ifstream romData(filename, std::ios::binary);
        if (!romData.is_open())
        {
            std::println(stderr, "Error opening file {}", filename);
        }
        std::vector<char> rawHeader(16);
        romData.read(rawHeader.data(), 16);

        uint32_t idString = *reinterpret_cast<const uint32_t*>(rawHeader.data());
        // nintendo drm magic number
        if (idString != 0x1A53454E)
        {
            std::println(stderr, "Rom provided does not have Nintendo ID string at the start");
        }
        // lower 8 bits of prg rom size in kb
        uint8_t prg_rom_size_LSB = rawHeader.data()[4];
        // lower 8 bits of chr rom size in kb
        uint8_t chr_rom_size_LSB = rawHeader.data()[5];
        // bunch of flags
        uint8_t flags_6 = rawHeader.data()[6];
        // bunch of flags 2 electric boogaloo
        uint8_t flags_7 = rawHeader.data()[7];
        // weird mapper shit, includes MSB of mapper no and submapper
        uint8_t mapper_MSB_submapper = rawHeader.data()[8];
        // top 4 bits of prg and chr rom size
        uint8_t prg_chr_size_MSB = rawHeader.data()[9];
        // size of extra ram and battery backed mem on cart
        uint8_t prg_ram_nvram_size = rawHeader.data()[10];
        // size of extra vram and battery backed GRAPHICS??? used by like two games
        uint8_t chr_ram_size = rawHeader.data()[11];
        // basically NTSC vs PAL
        uint8_t cpu_ppu_timing = rawHeader.data()[12];
        // eg arcade NES DRM
        uint8_t weird_hardware_info_shit = rawHeader.data()[13];
        // literally used on like 5 games total ignore for own sanity
        uint8_t misc_chip_info = rawHeader.data()[14];
        // physical peripheral to plug in
        uint8_t default_expansion_device = rawHeader.data()[15];

        // suck it im only implementing nes2 who even uses ines
        bool is_nes2 = (flags_7 & 0x0C) == 0x08;

        if (!is_nes2) { std::println(stderr, "bum this isn't NES2 kill yourself"); }

        // figure out mapper id
        uint8_t mapper_lower_nibble = (flags_6 & 0xF0) >> 4;
        uint8_t mapper_upper_nibble = (flags_7 & 0xF0) >> 4;
        uint8_t mapper_highest_nibble = mapper_MSB_submapper & 0x0F;

        uint16_t mapper_id = mapper_highest_nibble << 8 | mapper_upper_nibble << 4 | mapper_lower_nibble;
        MapperName mapper_name = static_cast<MapperName>(mapper_id);

        // figure out nametable arrangement
        NametableArrangement arrangement_type = VERTICALLY_MIRRORED;
        if (flags_6 & 0x08)
        {
            arrangement_type = FOUR_SCREEN_LAYOUT;
        }
        else if (mapper_id == 1 || mapper_id == 4 || mapper_id == 7)
        {
            arrangement_type = MAPPER_CONTROLLED;
        }
        else if (flags_6 & 0x01)
        {
            arrangement_type = HORIZONTALLY_MIRRORED;
        }

        uint8_t prg_rom_size_MSB = prg_chr_size_MSB & 0x0F;
        uint8_t chr_rom_size_MSB = (prg_chr_size_MSB & 0xF0) >> 4;

        // IN BANKS NOT KB
        uint16_t prg_rom_bank_count = prg_rom_size_MSB << 8 | prg_rom_size_LSB;
        uint16_t chr_rom_bank_count = chr_rom_size_MSB << 8 | chr_rom_size_LSB;
        bool has_battery_backed_mem = flags_6 & 0x02;
        bool has_trainer = flags_6 & 0x04;
        ConsoleType console_type = static_cast<ConsoleType>(flags_7 & 0x03);

        romData.seekg(16, std::ios::beg);
        if (has_trainer)
        {
            romData.seekg(512, std::ios::cur);
        }

        // Grab the rom data
        int prg_rom_size_kb = prg_rom_bank_count * 16384;
        int chr_rom_size_kb = chr_rom_bank_count * 8192;

        std::vector<uint8_t> prg_bytes(prg_rom_size_kb);
        std::vector<uint8_t> chr_bytes(chr_rom_size_kb);
        romData.read(reinterpret_cast<char*>(prg_bytes.data()), prg_rom_size_kb);
        // some roms use chr-ram instead of chr-rom, provide it with one page worth of blank data in this case
        if (chr_rom_bank_count > 0)
        {
            romData.read(reinterpret_cast<char*>(chr_bytes.data()), chr_rom_size_kb);
        } else { chr_bytes.resize(8192); }

        CartridgeHeader header = CartridgeHeader {
            .prg_rom_banks = prg_rom_bank_count,
            .chr_rom_banks = chr_rom_bank_count,
            .console_type = console_type,
            .has_battery_save = has_battery_backed_mem,
            .nametable_arrangement = arrangement_type
        };


        switch (mapper_name)
        {
        case NROM:
            {
                return std::make_shared<Cartridge_NROM>(Cartridge_NROM(prg_rom_bank_count, prg_ram_nvram_size, prg_bytes));
            }
        }
    }
};