#include "arm.h"

#include "gamepak/gamepak.h"
#include "gpu/gpu.h"
#include "timer/timer.h"

enum Region
{
    kRegionBios,
    kRegionUnused,
    kRegionExternalWorkRam,
    kRegionInternalWorkRam,
    kRegionMmio,
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

u32 Arm::readUnused() const
{
    if (cpsr.t == 0)
        return pipe[1];

    switch (pc >> 24)
    {
    case kRegionBios:
    case kRegionOam:
        return (pipe[1] << 16) | pipe[0];

    case kRegionInternalWorkRam:
        return (pipe[1] << 16) | pipe[(pc >> 1) ^ 0x1];

    default:
        return (pipe[1] << 16) | pipe[1];
    }
}

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
            byte = bios.read<u8>(addr);
            wait = 1;
            break;
        }
        [[fallthrough]];

    case kRegionUnused:
        byte = bit::ror(readUnused(), 8 * (addr & 0x3));
        wait = 1;
        break;

    case kRegionExternalWorkRam:
        byte = ewram.readByte(addr);
        wait = 3;
        break;

    case kRegionInternalWorkRam:
        byte = iwram.readByte(addr);
        wait = 1;
        break;

    case kRegionMmio:
        byte = mmio.readByte(addr);
        wait = 1;
        break;

    case kRegionPaletteRam:
        byte = gpu.pram.readByte(addr);
        wait = 1;
        break;

    case kRegionVideoRam:
        byte = gpu.vram.readByte(addr);
        wait = 1;
        break;

    case kRegionOam:
        byte = gpu.oam.readByte(addr);
        wait = 1;
        break;

    case kRegionGamePak0L:
    case kRegionGamePak0H:
    case kRegionGamePak1L:
    case kRegionGamePak1H:
    case kRegionGamePak2L:
    case kRegionGamePak2H:
        byte = gamepak.read<u8>(addr);
        wait = waitcnt.cyclesHalf(addr, access);
        break;

    case kRegionSaveL:
    case kRegionSaveH:
        byte = gamepak.readSave(addr);
        wait = waitcnt.cyclesHalf(addr, access);
        break;

    default:
        byte = bit::ror(readUnused(), 8 * (addr & 0x3));
        wait = 1;
        break;
    }

    clock(wait);

    return byte;
}

u16 Arm::readHalf(u32 addr, Access access)
{
    pipe.access = Access::NonSequential;

    u16 half;
    int wait;
    
    switch (addr >> 24)
    {
    case kRegionBios:
        if (addr < Bios::kSize)
        {
            half = bios.read<u16>(addr);
            wait = 1;
            break;
        }
        [[fallthrough]];

    case kRegionUnused:
        half = bit::ror(readUnused(), 8 * (addr & 0x2));
        wait = 1;
        break;

    case kRegionExternalWorkRam:
        half = ewram.readHalf(addr);
        wait = 3;
        break;

    case kRegionInternalWorkRam:
        half = iwram.readHalf(addr);
        wait = 1;
        break;

    case kRegionMmio:
        half = mmio.readHalf(addr);
        wait = 1;
        break;

    case kRegionPaletteRam:
        half = gpu.pram.readHalf(addr);
        wait = 1;
        break;

    case kRegionVideoRam:
        half = gpu.vram.readHalf(addr);
        wait = 1;
        break;

    case kRegionOam:
        half = gpu.oam.readHalf(addr);
        wait = 1;
        break;

    case kRegionGamePak2H:
        if (gamepak.isEepromAccess(addr))
        {
            half = 1;
            wait = waitcnt.cyclesHalf(addr, access);
            break;
        }
        [[fallthrough]];

    case kRegionGamePak0L:
    case kRegionGamePak0H:
    case kRegionGamePak1L:
    case kRegionGamePak1H:
    case kRegionGamePak2L:
        half = gamepak.read<u16>(addr);
        wait = waitcnt.cyclesHalf(addr, access);
        break;

    case kRegionSaveL:
    case kRegionSaveH:
        half = gamepak.readSave(addr) * 0x0101;
        wait = waitcnt.cyclesHalf(addr, access);
        break;

    default:
        half = bit::ror(readUnused(), 8 * (addr & 0x2));
        wait = 1;
    }

    clock(wait);

    return half;
}

u32 Arm::readWord(u32 addr, Access access)
{
    pipe.access = Access::NonSequential;

    u32 word;
    int wait;

    switch (addr >> 24)
    {
    case kRegionBios:
        if (addr < Bios::kSize)
        {
            word = bios.read<u32>(addr);
            wait = 1;
            break;
        }
        [[fallthrough]];

    case kRegionUnused:
        word = readUnused();
        wait = 1;
        break;

    case kRegionExternalWorkRam:
        word = ewram.readWord(addr);
        wait = 6;
        break;

    case kRegionInternalWorkRam:
        word = iwram.readWord(addr);
        wait = 1;
        break;

    case kRegionMmio:
        word = mmio.readWord(addr);
        wait = 1;
        break;

    case kRegionPaletteRam:
        word = gpu.pram.readWord(addr);
        wait = 2;
        break;

    case kRegionVideoRam:
        word = gpu.vram.readWord(addr);
        wait = 2;
        break;

    case kRegionOam:
        word = gpu.oam.readWord(addr);
        wait = 1;
        break;

    case kRegionGamePak2H:
        if (gamepak.isEepromAccess(addr))
        {
            word = 1;
            wait = waitcnt.cyclesWord(addr, access);
            break;
        }
        [[fallthrough]];

    case kRegionGamePak0L:
    case kRegionGamePak0H:
    case kRegionGamePak1L:
    case kRegionGamePak1H:
    case kRegionGamePak2L:
        word = gamepak.read<u32>(addr);
        wait = waitcnt.cyclesWord(addr, access);
        break;

    case kRegionSaveL:
    case kRegionSaveH:
        word = gamepak.readSave(addr) * 0x0101'0101;
        wait = waitcnt.cyclesWord(addr, access);
        break;

    default:
        word = readUnused();
        wait = 1;
        break;
    }

    clock(wait);

    return word;
}

void Arm::writeByte(u32 addr, u8 byte, Access access)
{
    pipe.access = Access::NonSequential;

    int wait;
    
    switch (addr >> 24)
    {
    case kRegionBios:
    case kRegionUnused:
        wait = 1;
        break;

    case kRegionExternalWorkRam:
        ewram.writeByte(addr, byte);
        wait = 3;
        break;

    case kRegionInternalWorkRam:
        iwram.writeByte(addr, byte);
        wait = 1;
        break;

    case kRegionMmio:
        mmio.writeByte(addr, byte);
        wait = 1;
        break;

    case kRegionPaletteRam:
        gpu.pram.writeByte(addr, byte);
        wait = 1;
        break;

    case kRegionVideoRam:
        gpu.vram.writeByte(addr, byte);
        wait = 1;
        break;

    case kRegionOam:
        wait = 1;
        break;

    case kRegionGamePak0L:
    case kRegionGamePak0H:
    case kRegionGamePak1L:
    case kRegionGamePak1H:
    case kRegionGamePak2L:
    case kRegionGamePak2H:
        gamepak.write<u8>(addr, byte);
        wait = waitcnt.cyclesHalf(addr, access);
        break;

    case kRegionSaveL:
    case kRegionSaveH:
        gamepak.writeSave(addr, byte);
        wait = waitcnt.cyclesHalf(addr, access);
        break;

    default:
        wait = 1;
        break;
    }

    clock(wait);
}

void Arm::writeHalf(u32 addr, u16 half, Access access)
{
    pipe.access = Access::NonSequential;
    
    int wait;

    switch (addr >> 24)
    {
    case kRegionBios:
    case kRegionUnused:
        wait = 1;
        break;

    case kRegionExternalWorkRam:
        ewram.writeHalf(addr, half);
        wait = 3;
        break;

    case kRegionInternalWorkRam:
        iwram.writeHalf(addr, half);
        wait = 1;
        break;

    case kRegionMmio:
        mmio.writeHalf(addr, half);
        wait = 1;
        break;

    case kRegionPaletteRam:
        gpu.pram.writeHalf(addr, half);
        wait = 1;
        break;

    case kRegionVideoRam:
        gpu.vram.writeHalf(addr, half);
        wait = 1;
        break;

    case kRegionOam:
        gpu.oam.writeHalf(addr, half);
        wait = 1;
        break;

    case kRegionGamePak0L:
    case kRegionGamePak0H:
    case kRegionGamePak1L:
    case kRegionGamePak1H:
    case kRegionGamePak2L:
    case kRegionGamePak2H:
        gamepak.write<u16>(addr, half);
        wait = waitcnt.cyclesHalf(addr, access);
        break;

    case kRegionSaveL:
    case kRegionSaveH:
        gamepak.writeSave(addr, half >> (8 * (addr & 0x1)));
        wait = waitcnt.cyclesHalf(addr, access);
        break;

    default:
        wait = 1;
        break;
    }

    clock(wait);
}

void Arm::writeWord(u32 addr, u32 word, Access access)
{
    pipe.access = Access::NonSequential;
    
    int wait;

    switch (addr >> 24)
    {
    case kRegionBios:
    case kRegionUnused:
        wait = 1;
        break;

    case kRegionExternalWorkRam:
        ewram.writeWord(addr, word);
        wait = 6;
        break;

    case kRegionInternalWorkRam:
        iwram.writeWord(addr, word);
        wait = 1;
        break;

    case kRegionMmio:
        mmio.writeWord(addr, word);
        wait = 1;
        break;

    case kRegionPaletteRam:
        gpu.pram.writeWord(addr, word);
        wait = 2;
        break;

    case kRegionVideoRam:
        gpu.vram.writeWord(addr, word);
        wait = 2;
        break;

    case kRegionOam:
        gpu.oam.writeWord(addr, word);
        wait = 1;
        break;

    case kRegionGamePak0L:
    case kRegionGamePak0H:
    case kRegionGamePak1L:
    case kRegionGamePak1H:
    case kRegionGamePak2L:
    case kRegionGamePak2H:
        gamepak.write<u32>(addr, word);
        wait = waitcnt.cyclesWord(addr, access);
        break;

    case kRegionSaveL:
    case kRegionSaveH:
        gamepak.writeSave(addr, word >> (8 * (addr & 0x3)));
        wait = waitcnt.cyclesWord(addr, access);
        break;

    default:
        wait = 1;
        break;
    }

    clock(wait);
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
    if (addr & 0x1)
    {
        u32 value = readByte(addr, access);
        return bit::signEx<8>(value);
    }
    else
    {
        u32 value = readHalf(addr, access);
        return bit::signEx<16>(value);
    }
}
