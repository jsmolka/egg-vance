#include "mmu.h"

#include "common/utility.h"
#include "ppu/ppu.h"

MMU mmu;

void MMU::reset()
{
    bios.reset();

    ewram.fill(0);
    iwram.fill(0);
}

u8 MMU::readByte(u32 addr)
{
    switch (addr >> 24)
    {
    case PAGE_BIOS:
        if (addr < 0x4000)
            return bios.readByte(addr);
        else
            return 0;

    case PAGE_BIOS+1:
        return 0;

    case PAGE_EWRAM:
        return ewram.readByte(addr);

    case PAGE_IWRAM:
        return iwram.readByte(addr);

    case PAGE_IO:
        if (addr < (MAP_IO + 0x400))
        {
            addr &= 0x3FF;
            //return mmio.readByte(addr);
        }
        return 0;

    case PAGE_PALETTE:
        return ppu.palette.readByte(addr);

    case PAGE_VRAM:
        return ppu.vram.readByte(addr);

    case PAGE_OAM:
        return ppu.oam.readByte(addr);

    case PAGE_GAMEPAK_0:
    case PAGE_GAMEPAK_0+1:
    case PAGE_GAMEPAK_1:
    case PAGE_GAMEPAK_1+1:
    case PAGE_GAMEPAK_2:
        addr &= 0x1FF'FFFF;
        return gamepak.readByte(addr);

    case PAGE_GAMEPAK_2+1:
        if (gamepak.backup->type == Backup::Type::EEPROM)
        {
            // Todo: DMA running?
            if (gamepak.size() <= 0x100'0000 || (addr >= 0xDFF'FF00 && addr < 0xE00'0000))
                return 1;
        }
        addr &= 0x1FF'FFFF;
        return gamepak.readByte(addr);
    
    case PAGE_GAMEPAK_SRAM:
        switch (gamepak.backup->type)
        {
        case Backup::Type::SRAM:
            addr &= 0x7FFF;
            return gamepak.backup->readByte(addr);

        case Backup::Type::FLASH64:
        case Backup::Type::FLASH128:
            addr &= 0xFFFF;
            return gamepak.backup->readByte(addr);
        }
        return 0;

    default:
        if (gamepak.backup->type == Backup::Type::SRAM)
        {
            addr &= 0x7FFF;
            return gamepak.backup->readByte(addr);
        }
        return 0;
    }
    return 0;
}

u16 MMU::readHalf(u32 addr)
{
    switch (addr >> 24)
    {
    case PAGE_BIOS:
        if (addr < 0x4000)
            return bios.readHalf(addr);
        else
            return 0;

    case PAGE_BIOS+1:
        return 0;

    case PAGE_EWRAM:
        return ewram.readHalf(addr);

    case PAGE_IWRAM:
        return iwram.readHalf(addr);

    case PAGE_IO:
        if (addr < (MAP_IO + 0x400))
        {
            addr &= 0x3FE;
            //return mmio.readHalf(addr);
        }
        return 0;

    case PAGE_PALETTE:
        return ppu.palette.readHalf(addr);

    case PAGE_VRAM:
        return ppu.vram.readHalf(addr);

    case PAGE_OAM:
        return ppu.oam.readHalf(addr);

    case PAGE_GAMEPAK_0:
    case PAGE_GAMEPAK_0+1:
    case PAGE_GAMEPAK_1:
    case PAGE_GAMEPAK_1+1:
    case PAGE_GAMEPAK_2:
        addr &= 0x1FF'FFFE;
        return gamepak.readHalf(addr);

    case PAGE_GAMEPAK_2+1:
        if (gamepak.backup->type == Backup::Type::EEPROM)
        {
            if (gamepak.size() <= 0x100'0000 || (addr >= 0xDFF'FF00 && addr < 0xE00'0000))
                return 1;
        }
        addr &= 0x1FF'FFFE;
        return gamepak.readHalf(addr);

    case PAGE_GAMEPAK_SRAM:
    case PAGE_UNUSED:
        return 0;
    }
    return 0;
}

u32 MMU::readWord(u32 addr)
{
    switch (addr >> 24)
    {
    case PAGE_BIOS:
        if (addr < 0x4000)
            return bios.readWord(addr);
        else
            return 0;

    case PAGE_BIOS+1:
        return 0;

    case PAGE_EWRAM:
        addr &= 0x3'FFFC;
        return ewram.readWord(addr);

    case PAGE_IWRAM:
        addr &= 0x7FFC;
        return iwram.readWord(addr);

    case PAGE_IO:
        if (addr < (MAP_IO + 0x400))
        {
            addr &= 0x3FC;
            //return mmio.readWord(addr);
        }
        return 0;

    case PAGE_PALETTE:
        return ppu.palette.readWord(addr);

    case PAGE_VRAM:
        return ppu.vram.readWord(addr);

    case PAGE_OAM:
        return ppu.oam.readWord(addr);

    case PAGE_GAMEPAK_0:
    case PAGE_GAMEPAK_0+1:
    case PAGE_GAMEPAK_1:
    case PAGE_GAMEPAK_1+1:
    case PAGE_GAMEPAK_2:
        addr &= 0x1FF'FFFC;
        return gamepak.readWord(addr);

    case PAGE_GAMEPAK_2+1:
        if (gamepak.backup->type == Backup::Type::EEPROM)
        {
            if (gamepak.size() <= 0x100'0000 || (addr >= 0xDFF'FF00 && addr < 0xE00'0000))
                return 1;
        }
        addr &= 0x1FF'FFFC;
        return gamepak.readWord(addr);

    case PAGE_GAMEPAK_SRAM:
    case PAGE_UNUSED:
        return 0;
    }
    return 0;
}

void MMU::writeByte(u32 addr, u8 byte)
{
    switch (addr >> 24)
    {
    case PAGE_BIOS:
    case PAGE_BIOS+1:
        break;

    case PAGE_EWRAM:
        ewram.writeByte(addr, byte);
        break;

    case PAGE_IWRAM:
        iwram.writeByte(addr, byte);
        break;

    case PAGE_IO:
        if (addr < (MAP_IO + 0x400))
        {
            addr &= 0x3FF;
            //mmio.writeByte(addr, byte);

            //switch (addr)
            //{
            //case REG_DMA0CNT_H+1:
            //case REG_DMA1CNT_H+1:
            //case REG_DMA2CNT_H+1:
            //case REG_DMA3CNT_H+1:
            //    signalDMA(DMA::Timing::IMMEDIATE);
            //    break;
            //}
        }
        break;

    case PAGE_PALETTE:
        ppu.palette.writeByte(addr, byte);
        break;

    case PAGE_VRAM:
        ppu.vram.writeByte(addr, byte);
        break;

    case PAGE_OAM:
        ppu.oam.writeByte(addr, byte);
        break;

    case PAGE_GAMEPAK_0:
    case PAGE_GAMEPAK_0+1:
    case PAGE_GAMEPAK_1:
    case PAGE_GAMEPAK_1+1:
    case PAGE_GAMEPAK_2:
    case PAGE_GAMEPAK_2+1:
        break;

    case PAGE_GAMEPAK_SRAM:
        switch (gamepak.backup->type)
        {
        case Backup::Type::SRAM:
            addr &= 0x7FFF;
            gamepak.backup->writeByte(addr, byte);
            break;

        case Backup::Type::FLASH64:
        case Backup::Type::FLASH128:
            addr &= 0xFFFF;
            gamepak.backup->writeByte(addr, byte);
            break;
        }
        break;

    default:
        if (gamepak.backup->type == Backup::Type::SRAM)
        {
            addr &= 0x7FFF;
            gamepak.backup->writeByte(addr, byte);
        }
        break;
    }
}

void MMU::writeHalf(u32 addr, u16 half)
{
    switch (addr >> 24)
    {
    case PAGE_BIOS:
    case PAGE_BIOS+1:
        break;

    case PAGE_EWRAM:
        ewram.writeHalf(addr, half);
        break;

    case PAGE_IWRAM:
        iwram.writeHalf(addr, half);
        break;

    case PAGE_IO:
        if (addr < (MAP_IO + 0x400))
        {
            addr &= 0x3FE;
            //mmio.writeHalf(addr, half);

            //switch (addr)
            //{
            //case REG_DMA0CNT_H:
            //case REG_DMA1CNT_H:
            //case REG_DMA2CNT_H:
            //case REG_DMA3CNT_H:
            //    signalDMA(DMA::Timing::IMMEDIATE);
            //    break;
            //}
        }
        break;

    case PAGE_PALETTE:
        ppu.palette.writeHalf(addr, half);
        break;

    case PAGE_VRAM:
        ppu.vram.writeHalf(addr, half);
        break;

    case PAGE_OAM:
        ppu.oam.writeHalf(addr, half);
        break;

    case PAGE_GAMEPAK_0:
    case PAGE_GAMEPAK_0+1:
    case PAGE_GAMEPAK_1:
    case PAGE_GAMEPAK_1+1:
    case PAGE_GAMEPAK_2:
    case PAGE_GAMEPAK_2+1:
        break;

    case PAGE_GAMEPAK_SRAM:
    case PAGE_UNUSED:
        break;
    }
}

void MMU::writeWord(u32 addr, u32 word)
{
    switch (addr >> 24)
    {
    case PAGE_BIOS:
    case PAGE_BIOS+1:
        break;

    case PAGE_EWRAM:
        ewram.writeWord(addr, word);
        break;

    case PAGE_IWRAM:
        iwram.writeWord(addr, word);
        break;

    case PAGE_IO:
        if (addr < (MAP_IO + 0x400))
        {
            addr &= 0x3FC;
            //mmio.writeWord(addr, word);

            //switch (addr)
            //{
            //case REG_DMA0CNT_L:
            //case REG_DMA1CNT_L:
            //case REG_DMA2CNT_L:
            //case REG_DMA3CNT_L:
            //    signalDMA(DMA::Timing::IMMEDIATE);
            //    break;
            //}
        }
        break;

    case PAGE_PALETTE:
        ppu.palette.writeWord(addr, word);
        break;

    case PAGE_VRAM:
        ppu.vram.writeWord(addr, word);
        break;

    case PAGE_OAM:
        ppu.oam.writeWord(addr, word);
        break;

    case PAGE_GAMEPAK_0:
    case PAGE_GAMEPAK_0+1:
    case PAGE_GAMEPAK_1:
    case PAGE_GAMEPAK_1+1:
    case PAGE_GAMEPAK_2:
    case PAGE_GAMEPAK_2+1:
        break;

    case PAGE_GAMEPAK_SRAM:
    case PAGE_UNUSED:
        break;
    }
}
