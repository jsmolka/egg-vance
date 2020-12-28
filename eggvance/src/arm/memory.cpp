#include "arm.h"

#include "gamepak/gamepak.h"
#include "ppu/ppu.h"

enum Region
{
    kRegionBios,
    kRegionUnused,
    kRegionExternalWorkRam,
    kRegionInternalWorkRam,
    kRegionIo,
    kRegionPaletteRam,
    kRegionVideoRam,
    kRegionOam,
    kRegionGamePak0L,
    kRegionGamePak0H,
    kRegionGamePak1L,
    kRegionGamePak1H,
    kRegionGamePak2L,
    kRegionGamePak2H,
    kRegionSaveL,
    kRegionSaveH
};

u8 Arm::readByte(u32 addr, Access access)
{
    pipe.access = Access::NonSequential;

    u8  byte;
    int wait;

    switch (addr >> 24)
    {
    case kRegionBios:
        if (addr < Bios::kSize)
        {
            prefetchRam(1);
            return bios.readByte(addr);
        }
        [[fallthrough]];

    case kRegionUnused:
        prefetchRam(1);
        return readUnused() >> (8 * (addr & 0x3));

    case kRegionExternalWorkRam:
        prefetchRam(3);
        return ewram.readByte(addr);

    case kRegionInternalWorkRam:
        prefetchRam(1);
        return iwram.readByte(addr);

    case kRegionIo:
        prefetchRam(1);
        return readIo(addr);

    case kRegionPaletteRam:
        prefetchRam(1);
        return ppu.pram.readByte(addr);

    case kRegionVideoRam:
        prefetchRam(1);
        return ppu.vram.readByte(addr);

    case kRegionOam:
        prefetchRam(1);
        return ppu.oam.readByte(addr);

    case kRegionGamePak0L:
    case kRegionGamePak0H:
    case kRegionGamePak1L:
    case kRegionGamePak1H:
    case kRegionGamePak2L:
    case kRegionGamePak2H:
        prefetchRom(addr, waitcnt.cyclesHalf(addr, access));
        return gamepak.read<u8>(addr);

    case kRegionSaveL:
    case kRegionSaveH:
        prefetchRom(addr, waitcnt.cyclesHalf(addr, access));
        return gamepak.readSave(addr);

    default:
        prefetchRam(1);
        return readUnused() >> (8 * (addr & 0x3));
    }
}

u16 Arm::readHalf(u32 addr, Access access)
{
    pipe.access = Access::NonSequential;

    switch (addr >> 24)
    {
    case kRegionBios:
        if (addr < Bios::kSize)
        {
            prefetchRam(1);
            return bios.readHalf(addr);
        }
        [[fallthrough]];

    case kRegionUnused:
        prefetchRam(1);
        return readUnused() >> (8 * (addr & 0x2));

    case kRegionExternalWorkRam:
        prefetchRam(3);
        return ewram.readHalf(addr);

    case kRegionInternalWorkRam:
        prefetchRam(1);
        return iwram.readHalf(addr);

    case kRegionIo:
        prefetchRam(1);
        addr &= ~0x1;
        return readIo(addr + 0) << 0
             | readIo(addr + 1) << 8;

    case kRegionPaletteRam:
        prefetchRam(1);
        return ppu.pram.readHalf(addr);

    case kRegionVideoRam:
        prefetchRam(1);
        return ppu.vram.readHalf(addr);

    case kRegionOam:
        prefetchRam(1);
        return ppu.oam.readHalf(addr);

    case kRegionGamePak2H:
        if (gamepak.isEepromAccess(addr))
        {
            prefetchRom(addr, waitcnt.cyclesHalf(addr, access));
            return 1;
        }
        [[fallthrough]];

    case kRegionGamePak0L:
    case kRegionGamePak0H:
    case kRegionGamePak1L:
    case kRegionGamePak1H:
    case kRegionGamePak2L:
        prefetchRom(addr, waitcnt.cyclesHalf(addr, access));
        return gamepak.read<u16>(addr);

    case kRegionSaveL:
    case kRegionSaveH:
        prefetchRom(addr, waitcnt.cyclesHalf(addr, access));
        return gamepak.readSave(addr) * 0x0101;

    default:
        prefetchRam(1);
        return readUnused() >> (8 * (addr & 0x2));
    }
}

u32 Arm::readWord(u32 addr, Access access)
{
    pipe.access = Access::NonSequential;

    switch (addr >> 24)
    {
    case kRegionBios:
        if (addr < Bios::kSize)
        {
            prefetchRam(1);
            return bios.readWord(addr);
        }
        [[fallthrough]];

    case kRegionUnused:
        prefetchRam(1);
        return readUnused();

    case kRegionExternalWorkRam:
        prefetchRam(6);
        return ewram.readWord(addr);

    case kRegionInternalWorkRam:
        prefetchRam(1);
        return iwram.readWord(addr);

    case kRegionIo:
        prefetchRam(1);
        addr &= ~0x3;
        return readIo(addr + 0) <<  0
             | readIo(addr + 1) <<  8
             | readIo(addr + 2) << 16
             | readIo(addr + 3) << 24;

    case kRegionPaletteRam:
        prefetchRam(2);
        return ppu.pram.readWord(addr);

    case kRegionVideoRam:
        prefetchRam(2);
        return ppu.vram.readWord(addr);

    case kRegionOam:
        prefetchRam(1);
        return ppu.oam.readWord(addr);

    case kRegionGamePak2H:
        if (gamepak.isEepromAccess(addr))
        {
            prefetchRom(addr, waitcnt.cyclesWord(addr, access));
            return 1;
        }
        [[fallthrough]];

    case kRegionGamePak0L:
    case kRegionGamePak0H:
    case kRegionGamePak1L:
    case kRegionGamePak1H:
    case kRegionGamePak2L:
        prefetchRom(addr, waitcnt.cyclesWord(addr, access));
        return gamepak.read<u32>(addr);

    case kRegionSaveL:
    case kRegionSaveH:
        prefetchRom(addr, waitcnt.cyclesWord(addr, access));
        return gamepak.readSave(addr) * 0x0101'0101;

    default:
        prefetchRam(1);
        return readUnused();
    }
}

void Arm::writeByte(u32 addr, u8 byte, Access access)
{
    pipe.access = Access::NonSequential;

    switch (addr >> 24)
    {
    case kRegionBios:
    case kRegionUnused:
        prefetchRam(1);
        break;

    case kRegionExternalWorkRam:
        prefetchRam(3);
        ewram.writeByte(addr, byte);
        break;

    case kRegionInternalWorkRam:
        prefetchRam(1);
        iwram.writeByte(addr, byte);
        break;

    case kRegionIo:
        prefetchRam(1);
        writeIo(addr, byte);
        break;

    case kRegionPaletteRam:
        prefetchRam(1);
        ppu.pram.writeByte(addr, byte);
        break;

    case kRegionVideoRam:
        prefetchRam(1);
        ppu.vram.writeByte(addr, byte);
        break;

    case kRegionOam:
        prefetchRam(1);
        break;

    case kRegionGamePak0L:
    case kRegionGamePak0H:
    case kRegionGamePak1L:
    case kRegionGamePak1H:
    case kRegionGamePak2L:
    case kRegionGamePak2H:
        prefetchRom(addr, waitcnt.cyclesHalf(addr, access));
        gamepak.write<u8>(addr, byte);
        break;

    case kRegionSaveL:
    case kRegionSaveH:
        prefetchRom(addr, waitcnt.cyclesHalf(addr, access));
        gamepak.writeSave(addr, byte);
        break;

    default:
        prefetchRam(1);
        break;
    }
}

void Arm::writeHalf(u32 addr, u16 half, Access access)
{
    pipe.access = Access::NonSequential;
    
    switch (addr >> 24)
    {
    case kRegionBios:
    case kRegionUnused:
        prefetchRam(1);
        break;

    case kRegionExternalWorkRam:
        ewram.writeHalf(addr, half);
        prefetchRam(3);
        break;

    case kRegionInternalWorkRam:
        prefetchRam(1);
        iwram.writeHalf(addr, half);
        break;

    case kRegionIo:
        prefetchRam(1);
        addr &= ~0x1;
        writeIo(addr + 0, bit::seq<0, 8>(half));
        writeIo(addr + 1, bit::seq<8, 8>(half));
        break;

    case kRegionPaletteRam:
        prefetchRam(1);
        ppu.pram.writeHalf(addr, half);
        break;

    case kRegionVideoRam:
        prefetchRam(1);
        ppu.vram.writeHalf(addr, half);
        break;

    case kRegionOam:
        prefetchRam(1);
        ppu.oam.writeHalf(addr, half);
        break;

    case kRegionGamePak0L:
    case kRegionGamePak0H:
    case kRegionGamePak1L:
    case kRegionGamePak1H:
    case kRegionGamePak2L:
    case kRegionGamePak2H:
        prefetchRom(addr, waitcnt.cyclesHalf(addr, access));
        gamepak.write<u16>(addr, half);
        break;

    case kRegionSaveL:
    case kRegionSaveH:
        prefetchRom(addr, waitcnt.cyclesHalf(addr, access));
        gamepak.writeSave(addr, half >> (8 * (addr & 0x1)));
        break;

    default:
        prefetchRam(1);
        break;
    }
}

void Arm::writeWord(u32 addr, u32 word, Access access)
{
    pipe.access = Access::NonSequential;
    
    switch (addr >> 24)
    {
    case kRegionBios:
    case kRegionUnused:
        prefetchRam(1);
        break;

    case kRegionExternalWorkRam:
        prefetchRam(6);
        ewram.writeWord(addr, word);
        break;

    case kRegionInternalWorkRam:
        prefetchRam(1);
        iwram.writeWord(addr, word);
        break;

    case kRegionIo:
        prefetchRam(1);
        addr &= ~0x3;
        writeIo(addr + 0, bit::seq< 0, 8>(word));
        writeIo(addr + 1, bit::seq< 8, 8>(word));
        writeIo(addr + 2, bit::seq<16, 8>(word));
        writeIo(addr + 3, bit::seq<24, 8>(word));
        break;

    case kRegionPaletteRam:
        prefetchRam(2);
        ppu.pram.writeWord(addr, word);
        break;

    case kRegionVideoRam:
        prefetchRam(2);
        ppu.vram.writeWord(addr, word);
        break;

    case kRegionOam:
        prefetchRam(1);
        ppu.oam.writeWord(addr, word);
        break;

    case kRegionGamePak0L:
    case kRegionGamePak0H:
    case kRegionGamePak1L:
    case kRegionGamePak1H:
    case kRegionGamePak2L:
    case kRegionGamePak2H:
        prefetchRom(addr, waitcnt.cyclesWord(addr, access));
        gamepak.write<u32>(addr, word);
        break;

    case kRegionSaveL:
    case kRegionSaveH:
        prefetchRom(addr, waitcnt.cyclesWord(addr, access));
        gamepak.writeSave(addr, word >> (8 * (addr & 0x3)));
        break;

    default:
        prefetchRam(1);
        break;
    }
}

u32 Arm::readUnused() const
{
    if (cpsr.t == 0)
        return pipe[1];

    switch (pc >> 24)
    {
    case kRegionBios:
    case kRegionOam:
        return pipe[1] << 16 | pipe[0];

    case kRegionInternalWorkRam:
        return pipe[1] << 16 | pipe[(pc >> 1) ^ 0x1];

    default:
        return pipe[1] << 16 | pipe[1];
    }
}

u32 Arm::readHalfRotate(u32 addr, Access access)
{
    u32 value = readHalf(addr, access);
    return bit::ror(value, 8 * (addr & 0x1));
}

u32 Arm::readWordRotate(u32 addr, Access access)
{
    u32 value = readWord(addr, access);
    return bit::ror(value, 8 * (addr & 0x3));
}

u32 Arm::readByteSignEx(u32 addr, Access access)
{
    u32 value = readByte(addr, access);
    return bit::signEx<8>(value);
}

u32 Arm::readHalfSignEx(u32 addr, Access access)
{
    u32 value = readHalf(addr, access);
    value = bit::signEx<16>(value);
    return bit::sar(value, 8 * (addr & 0x1));
}