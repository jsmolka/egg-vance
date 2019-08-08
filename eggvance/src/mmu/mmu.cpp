#include "mmu.h"

#include <fstream>

#include "common/format.h"
#include "common/utility.h"

MMU::MMU()
    : vcount(io.ref<u8>(REG_VCOUNT))
    , intr_request(io.ref<u16>(REG_IF))
    , intr_enabled(io.ref<u16>(REG_IE))
    , keyinput(io.ref<u16>(REG_KEYINPUT))
    , timer{ Timer(0), Timer(1), Timer(2), Timer(3) }
    , dma{ DMA(0, *this), DMA(1, *this), DMA(2, *this), DMA(3, *this) }
{
    timer[0].next = &timer[1];
    timer[1].next = &timer[2];
    timer[2].next = &timer[3];

    reset();
}

void MMU::reset()
{
    bios.fill(0);
    wram.fill(0);
    iwram.fill(0);
    io.fill(0);
    palette.fill(0);
    vram.fill(0);
    oam.fill(0);
    sram.fill(0);

    timer[0].reset();
    timer[1].reset();
    timer[2].reset();
    timer[3].reset();

    for (int i = 0; i < oam.size(); ++i)
    {
        writeByte(MAP_IO + i, 0);
    }
    dispstat.hblank = false;
    dispstat.vblank = false;

    keyinput = 0x3FF;
    halt = false;

    oam_entries.fill(OAMEntry());
}

bool MMU::readFile(const std::string& file)
{
    std::ifstream stream(file, std::ios::binary);
    if (!stream.is_open())
    {
        fmt::printf("Cannot open file %s\n", file);
        return false;
    }

    stream.seekg(0, std::ios::end);
    std::streampos size = stream.tellg();
    stream.seekg(0, std::ios::beg);

    gamepak.resize(size);

    u8* memory_ptr = &gamepak[0];
    stream.read(reinterpret_cast<char*>(memory_ptr), size);

    return true;
}

bool MMU::readBios(const std::string& file)
{
    std::ifstream stream(file, std::ios::binary);
    if (!stream.is_open())
    {
        fmt::printf("Cannot open file %s\n", file);
        return false;
    }

    stream.seekg(0, std::ios::end);
    std::streampos size = stream.tellg();
    stream.seekg(0, std::ios::beg);

    u8* memory_ptr = &bios[0];
    stream.read(reinterpret_cast<char*>(memory_ptr), size);

    return true;
}

u8 MMU::readByte(u32 addr) const
{
    switch (addr >> 24)
    {
    case PAGE_BIOS:
    case PAGE_BIOS+1:
        if (addr >= (MAP_BIOS + 0x4000))
            return 0;

        // Todo: Return last fetched value if PC outside BIOS
        addr &= 0x3FFF;
        return bios[addr];

    case PAGE_WRAM:
        addr &= 0x3'FFFF;
        return wram[addr];

    case PAGE_IWRAM:
        addr &= 0x7FFF;
        return iwram[addr];

    case PAGE_IO:
        if (addr >= (MAP_IO + 0x400))
            return 0;

        addr &= 0x3FF;
        switch (addr)
        {
        case REG_VCOUNT: 
            return vcount;

        case REG_VCOUNT+1: 
            return 0;

        case REG_BG0HOFS: case REG_BG0HOFS+1:
        case REG_BG0VOFS: case REG_BG0VOFS+1:
        case REG_BG1HOFS: case REG_BG1HOFS+1:
        case REG_BG1VOFS: case REG_BG1VOFS+1:
        case REG_BG2HOFS: case REG_BG2HOFS+1:
        case REG_BG2VOFS: case REG_BG2VOFS+1:
        case REG_BG3HOFS: case REG_BG3HOFS+1:
        case REG_BG3VOFS: case REG_BG3VOFS+1:
            return 0;

        case REG_BG2X: case REG_BG2X+1: case REG_BG2X+2: case REG_BG2X+3:
        case REG_BG2Y: case REG_BG2Y+1: case REG_BG2Y+2: case REG_BG2Y+3:
        case REG_BG3X: case REG_BG3X+1: case REG_BG3X+2: case REG_BG3X+3:
        case REG_BG3Y: case REG_BG3Y+1: case REG_BG3Y+2: case REG_BG3Y+3:
            return 0;

        case REG_BG2PA: case REG_BG2PA+1: 
        case REG_BG2PB: case REG_BG2PB+1: 
        case REG_BG2PC: case REG_BG2PC+1: 
        case REG_BG2PD: case REG_BG2PD+1:
        case REG_BG3PA: case REG_BG3PA+1: 
        case REG_BG3PB: case REG_BG3PB+1:
        case REG_BG3PC: case REG_BG3PC+1: 
        case REG_BG3PD: case REG_BG3PD+1:
            return 0;

        case REG_WIN0H: case REG_WIN0H+1: 
        case REG_WIN1H: case REG_WIN1H+1:
        case REG_WIN0V: case REG_WIN0V+1:
        case REG_WIN1V: case REG_WIN1V+1:
            return 0;

        case REG_MOSAIC: 
        case REG_MOSAIC+1: 
        case REG_MOSAIC+2: 
        case REG_MOSAIC+3:
            return 0;

        case REG_BLDALPHA: 
        case REG_BLDALPHA+1:
            return 0;

        case REG_BLDY:
        case REG_BLDY+1:
        case REG_BLDY+2:
        case REG_BLDY+3:
            return 0;

        case REG_DMA0SAD: case REG_DMA0SAD+1: case REG_DMA0SAD+2: case REG_DMA0SAD+3:
        case REG_DMA1SAD: case REG_DMA1SAD+1: case REG_DMA1SAD+2: case REG_DMA1SAD+3:
        case REG_DMA2SAD: case REG_DMA2SAD+1: case REG_DMA2SAD+2: case REG_DMA2SAD+3:
        case REG_DMA3SAD: case REG_DMA3SAD+1: case REG_DMA3SAD+2: case REG_DMA3SAD+3:
            return 0;

        case REG_DMA0DAD: case REG_DMA0DAD+1: case REG_DMA0DAD+2: case REG_DMA0DAD+3:
        case REG_DMA1DAD: case REG_DMA1DAD+1: case REG_DMA1DAD+2: case REG_DMA1DAD+3:
        case REG_DMA2DAD: case REG_DMA2DAD+1: case REG_DMA2DAD+2: case REG_DMA2DAD+3:
        case REG_DMA3DAD: case REG_DMA3DAD+1: case REG_DMA3DAD+2: case REG_DMA3DAD+3:
            return 0;

        case REG_DMA0CNT_L: case REG_DMA0CNT_L+1:
        case REG_DMA1CNT_L: case REG_DMA1CNT_L+1:
        case REG_DMA2CNT_L: case REG_DMA2CNT_L+1:
        case REG_DMA3CNT_L: case REG_DMA3CNT_L+1:
            return 0;

        case REG_TM0CNT_L+0: return timer[0].data_b[0];
        case REG_TM0CNT_L+1: return timer[0].data_b[1];
        case REG_TM1CNT_L+0: return timer[1].data_b[0];
        case REG_TM1CNT_L+1: return timer[1].data_b[1];
        case REG_TM2CNT_L+0: return timer[2].data_b[0];
        case REG_TM2CNT_L+1: return timer[2].data_b[1];
        case REG_TM3CNT_L+0: return timer[3].data_b[0];
        case REG_TM3CNT_L+1: return timer[3].data_b[1];
        }
        return io[addr];

    case PAGE_PALETTE:
        addr &= 0x3FF;
        return palette[addr];

    case PAGE_VRAM:
        addr &= 0x1'FFFF;
        if (addr > 0x1'7FFF)
            addr -= 0x8000;

        return vram[addr];

    case PAGE_OAM:
        addr &= 0x3FF;
        return oam[addr];

    case PAGE_GAMEPAK_0: case PAGE_GAMEPAK_0+1:
    case PAGE_GAMEPAK_1: case PAGE_GAMEPAK_1+1:
    case PAGE_GAMEPAK_2: case PAGE_GAMEPAK_2+1:
        addr &= 0x1FF'FFFF;
        if (addr >= gamepak.size())
            return 0;
        return gamepak[addr];

    default:
        addr &= 0xFFFF;
        return sram[addr];
    }
}

u16 MMU::readHalf(u32 addr) const
{
    addr = alignHalf(addr);
    return (readByte(addr + 1) << 8) | readByte(addr);
}

u32 MMU::readWord(u32 addr) const
{
    addr = alignWord(addr);
    return (readHalf(addr + 2) << 16) | readHalf(addr);
}

void MMU::writeByte(u32 addr, u8 byte)
{
    switch (addr >> 24)
    {
    case PAGE_BIOS:
    case PAGE_BIOS+1:
        break;

    case PAGE_WRAM:
        addr &= 0x3'FFFF;
        wram[addr] = byte;
        break;

    case PAGE_IWRAM:
        addr &= 0x7FFF;
        iwram[addr] = byte;
        break;

    case PAGE_IO:
        if (addr >= (MAP_IO + 0x400))
            return;

        addr &= 0x3FF;
        switch (addr)
        {
        case REG_DISPCNT:
            dispcnt.mode        = bits<0, 3>(byte);
            dispcnt.gbc         = bits<3, 1>(byte);
            dispcnt.frame       = bits<4, 1>(byte);
            dispcnt.oam_hblank  = bits<5, 1>(byte);
            dispcnt.mapping_1d  = bits<6, 1>(byte);
            dispcnt.force_blank = bits<7, 1>(byte);
            break;

        case REG_DISPCNT+1:
            dispcnt.bg0    = bits<0, 1>(byte);
            dispcnt.bg1    = bits<1, 1>(byte);
            dispcnt.bg2    = bits<2, 1>(byte);
            dispcnt.bg3    = bits<3, 1>(byte);
            dispcnt.obj    = bits<4, 1>(byte);
            dispcnt.win0   = bits<5, 1>(byte);
            dispcnt.win1   = bits<6, 1>(byte);
            dispcnt.winobj = bits<7, 1>(byte);
            break;

        case REG_DISPSTAT:
            dispstat.vblank_irq = bits<3, 1>(byte);
            dispstat.hblank_irq = bits<4, 1>(byte);
            dispstat.vmatch_irq = bits<5, 1>(byte);
            break;

        case REG_DISPSTAT+1:
            dispstat.vcount_compare = byte;
            break;

        case REG_VCOUNT:
        case REG_VCOUNT+1:
            return;

        case REG_BG0CNT+0: writeBackgroundControlLower(bgcnt[0], byte); break;
        case REG_BG0CNT+1: writeBackgroundControlUpper(bgcnt[0], byte); break;
        case REG_BG1CNT+0: writeBackgroundControlLower(bgcnt[1], byte); break;
        case REG_BG1CNT+1: writeBackgroundControlUpper(bgcnt[1], byte); break;
        case REG_BG2CNT+0: writeBackgroundControlLower(bgcnt[2], byte); break;
        case REG_BG2CNT+1: writeBackgroundControlUpper(bgcnt[2], byte); break;
        case REG_BG3CNT+0: writeBackgroundControlLower(bgcnt[3], byte); break;
        case REG_BG3CNT+1: writeBackgroundControlUpper(bgcnt[3], byte); break;

        case REG_BG0HOFS+0: bghofs[0].offset_b[0] = byte; break;
        case REG_BG0HOFS+1: bghofs[0].offset_b[1] = byte & 0x1; break;
        case REG_BG0VOFS+0: bgvofs[0].offset_b[0] = byte; break;
        case REG_BG0VOFS+1: bgvofs[0].offset_b[1] = byte & 0x1; break;
        case REG_BG1HOFS+0: bghofs[1].offset_b[0] = byte; break;
        case REG_BG1HOFS+1: bghofs[1].offset_b[1] = byte & 0x1; break;
        case REG_BG1VOFS+0: bgvofs[1].offset_b[0] = byte; break;
        case REG_BG1VOFS+1: bgvofs[1].offset_b[1] = byte & 0x1; break;
        case REG_BG2HOFS+0: bghofs[2].offset_b[0] = byte; break;
        case REG_BG2HOFS+1: bghofs[2].offset_b[1] = byte & 0x1; break;
        case REG_BG2VOFS+0: bgvofs[2].offset_b[0] = byte; break;
        case REG_BG2VOFS+1: bgvofs[2].offset_b[1] = byte & 0x1; break;
        case REG_BG3HOFS+0: bghofs[3].offset_b[0] = byte; break;
        case REG_BG3HOFS+1: bghofs[3].offset_b[1] = byte & 0x1; break;
        case REG_BG3VOFS+0: bgvofs[3].offset_b[0] = byte; break;
        case REG_BG3VOFS+1: bgvofs[3].offset_b[1] = byte & 0x1; break;

        case REG_BG2X+0: bgx[0].ref_b[0] = byte; bgx[0].internal = bgx[0].ref; break;
        case REG_BG2X+1: bgx[0].ref_b[1] = byte; bgx[0].internal = bgx[0].ref; break;
        case REG_BG2X+2: bgx[0].ref_b[2] = byte; bgx[0].internal = bgx[0].ref; break;
        case REG_BG2X+3: bgx[0].ref_b[3] = signExtend<4>(byte); bgx[0].internal = bgx[0].ref; break;

        case REG_BG3X+0: bgx[1].ref_b[0] = byte; bgx[1].internal = bgx[1].ref; break;
        case REG_BG3X+1: bgx[1].ref_b[1] = byte; bgx[1].internal = bgx[1].ref; break;
        case REG_BG3X+2: bgx[1].ref_b[2] = byte; bgx[1].internal = bgx[1].ref; break;
        case REG_BG3X+3: bgx[1].ref_b[3] = signExtend<4>(byte); bgx[1].internal = bgx[1].ref; break;

        case REG_BG2Y+0: bgy[0].ref_b[0] = byte; bgy[0].internal = bgy[0].ref; break; 
        case REG_BG2Y+1: bgy[0].ref_b[1] = byte; bgy[0].internal = bgy[0].ref; break;
        case REG_BG2Y+2: bgy[0].ref_b[2] = byte; bgy[0].internal = bgy[0].ref; break;
        case REG_BG2Y+3: bgy[0].ref_b[3] = signExtend<4>(byte); bgy[0].internal = bgy[0].ref; break;

        case REG_BG3Y+0: bgy[1].ref_b[0] = byte; bgy[1].internal = bgy[1].ref; break;
        case REG_BG3Y+1: bgy[1].ref_b[1] = byte; bgy[1].internal = bgy[1].ref; break;
        case REG_BG3Y+2: bgy[1].ref_b[2] = byte; bgy[1].internal = bgy[1].ref; break;
        case REG_BG3Y+3: bgy[1].ref_b[3] = signExtend<4>(byte); bgy[1].internal = bgy[1].ref; break;

        case REG_BG2PA+0: bgpa[0].param_b[0] = byte; break;
        case REG_BG2PA+1: bgpa[0].param_b[1] = byte; break;
        case REG_BG2PB+0: bgpb[0].param_b[0] = byte; break;
        case REG_BG2PB+1: bgpb[0].param_b[1] = byte; break;
        case REG_BG2PC+0: bgpc[0].param_b[0] = byte; break;
        case REG_BG2PC+1: bgpc[0].param_b[1] = byte; break;
        case REG_BG2PD+0: bgpd[0].param_b[0] = byte; break;
        case REG_BG2PD+1: bgpd[0].param_b[1] = byte; break;

        case REG_BG3PA+0: bgpa[1].param_b[0] = byte; break;
        case REG_BG3PA+1: bgpa[1].param_b[1] = byte; break;
        case REG_BG3PB+0: bgpb[1].param_b[0] = byte; break;
        case REG_BG3PB+1: bgpb[1].param_b[1] = byte; break;
        case REG_BG3PC+0: bgpc[1].param_b[0] = byte; break;
        case REG_BG3PC+1: bgpc[1].param_b[1] = byte; break;
        case REG_BG3PD+0: bgpd[1].param_b[0] = byte; break;
        case REG_BG3PD+1: bgpd[1].param_b[1] = byte; break;

        case REG_WIN0H+0: winh[0].max = byte; break;
        case REG_WIN0H+1: winh[0].min = byte; break;
        case REG_WIN1H+0: winh[1].max = byte; break;
        case REG_WIN1H+1: winh[1].min = byte; break;

        case REG_WIN0V+0: winv[0].max = byte; break;
        case REG_WIN0V+1: winv[0].min = byte; break;
        case REG_WIN1V+0: winv[1].max = byte; break;
        case REG_WIN1V+1: winv[1].min = byte; break;

        case REG_WININ+0: writeWindow(winin.win0, byte); break;
        case REG_WININ+1: writeWindow(winin.win1, byte); break;

        case REG_WINOUT+0: writeWindow(winout.winout, byte); break;
        case REG_WINOUT+1: writeWindow(winout.winobj, byte); break;

        case REG_MOSAIC:
            mosaic.bg.x  = bits<0, 4>(byte);
            mosaic.bg.y  = bits<4, 4>(byte);
            break;

        case REG_MOSAIC+1:
            mosaic.obj.x = bits<0, 4>(byte);
            mosaic.obj.y = bits<4, 4>(byte);
            break;

        case REG_BLDCNT:
            writeBlendLayer(bldcnt.upper, byte);
            bldcnt.mode = bits<6, 2>(byte);
            break;

        case REG_BLDCNT+1:
            writeBlendLayer(bldcnt.lower, byte);
            break;

        case REG_BLDALPHA:
            bldalpha.eva = bits<0, 5>(byte);
            break;

        case REG_BLDALPHA+1:
            bldalpha.evb = bits<0, 5>(byte);
            break;

        case REG_BLDY:
            bldy.evy = bits<0, 5>(byte);
            break;

        case REG_WAITCNT:
            waitcnt.sram  = bits<0, 2>(byte);
            waitcnt.ws0.n = bits<2, 2>(byte);
            waitcnt.ws0.s = bits<4, 1>(byte);
            waitcnt.ws1.n = bits<5, 2>(byte);
            waitcnt.ws1.s = bits<7, 1>(byte);
            break;

        case REG_WAITCNT+1:
            waitcnt.ws2.n    = bits<0, 2>(byte);
            waitcnt.ws2.s    = bits<2, 1>(byte);
            waitcnt.phi      = bits<3, 2>(byte);
            waitcnt.prefetch = bits<6, 1>(byte);
            waitcnt.type     = bits<7, 1>(byte);
            break;

        case REG_KEYINPUT:
        case REG_KEYINPUT+1:
            return;

        case REG_DMA0SAD+0: dma[0].src.addr_b[0] = byte; break;
        case REG_DMA0SAD+1: dma[0].src.addr_b[1] = byte; break;
        case REG_DMA0SAD+2: dma[0].src.addr_b[2] = byte; break;
        case REG_DMA0SAD+3: dma[0].src.addr_b[3] = byte & 0x7; break;
        case REG_DMA1SAD+0: dma[1].src.addr_b[0] = byte; break;
        case REG_DMA1SAD+1: dma[1].src.addr_b[1] = byte; break;
        case REG_DMA1SAD+2: dma[1].src.addr_b[2] = byte; break;
        case REG_DMA1SAD+3: dma[1].src.addr_b[3] = byte & 0xF; break;
        case REG_DMA2SAD+0: dma[2].src.addr_b[0] = byte; break;
        case REG_DMA2SAD+1: dma[2].src.addr_b[1] = byte; break;
        case REG_DMA2SAD+2: dma[2].src.addr_b[2] = byte; break;
        case REG_DMA2SAD+3: dma[2].src.addr_b[3] = byte & 0xF; break;
        case REG_DMA3SAD+0: dma[3].src.addr_b[0] = byte; break;
        case REG_DMA3SAD+1: dma[3].src.addr_b[1] = byte; break;
        case REG_DMA3SAD+2: dma[3].src.addr_b[2] = byte; break;
        case REG_DMA3SAD+3: dma[3].src.addr_b[3] = byte & 0xF; break;

        case REG_DMA0DAD+0: dma[0].dst.addr_b[0] = byte; break;
        case REG_DMA0DAD+1: dma[0].dst.addr_b[1] = byte; break;
        case REG_DMA0DAD+2: dma[0].dst.addr_b[2] = byte; break;
        case REG_DMA0DAD+3: dma[0].dst.addr_b[3] = byte & 0x7; break;
        case REG_DMA1DAD+0: dma[1].dst.addr_b[0] = byte; break;
        case REG_DMA1DAD+1: dma[1].dst.addr_b[1] = byte; break;
        case REG_DMA1DAD+2: dma[1].dst.addr_b[2] = byte; break;
        case REG_DMA1DAD+3: dma[1].dst.addr_b[3] = byte & 0x7; break;
        case REG_DMA2DAD+0: dma[2].dst.addr_b[0] = byte; break;
        case REG_DMA2DAD+1: dma[2].dst.addr_b[1] = byte; break;
        case REG_DMA2DAD+2: dma[2].dst.addr_b[2] = byte; break;
        case REG_DMA2DAD+3: dma[2].dst.addr_b[3] = byte & 0x7; break;
        case REG_DMA3DAD+0: dma[3].dst.addr_b[0] = byte; break;
        case REG_DMA3DAD+1: dma[3].dst.addr_b[1] = byte; break;
        case REG_DMA3DAD+2: dma[3].dst.addr_b[2] = byte; break;
        case REG_DMA3DAD+3: dma[3].dst.addr_b[3] = byte & 0xF; break;

        case REG_DMA0CNT_L+0: dma[0].units_b[0] = byte; break;
        case REG_DMA0CNT_L+1: dma[0].units_b[1] = byte & 0x3F; break;
        case REG_DMA1CNT_L+0: dma[1].units_b[0] = byte; break;
        case REG_DMA1CNT_L+1: dma[1].units_b[1] = byte & 0x3F; break;
        case REG_DMA2CNT_L+0: dma[2].units_b[0] = byte; break;
        case REG_DMA2CNT_L+1: dma[2].units_b[1] = byte & 0x3F; break;
        case REG_DMA3CNT_L+0: dma[3].units_b[0] = byte; break;
        case REG_DMA3CNT_L+1: dma[3].units_b[1] = byte; break;

        case REG_DMA0CNT_H+0: writeDMAControlLower(dma[0].control, byte); break;
        case REG_DMA0CNT_H+1: writeDMAControlUpper(dma[0].control, byte); break;
        case REG_DMA1CNT_H+0: writeDMAControlLower(dma[1].control, byte); break;
        case REG_DMA1CNT_H+1: writeDMAControlUpper(dma[1].control, byte); break;
        case REG_DMA2CNT_H+0: writeDMAControlLower(dma[2].control, byte); break;
        case REG_DMA2CNT_H+1: writeDMAControlUpper(dma[2].control, byte); break;
        case REG_DMA3CNT_H+0: writeDMAControlLower(dma[3].control, byte); break;
        case REG_DMA3CNT_H+1: writeDMAControlUpper(dma[3].control, byte); break;

        case REG_TM0CNT_L+0: timer[0].initial_b[0] = byte; break;
        case REG_TM0CNT_L+1: timer[0].initial_b[1] = byte; break;
        case REG_TM1CNT_L+0: timer[1].initial_b[0] = byte; break;
        case REG_TM1CNT_L+1: timer[1].initial_b[1] = byte; break;
        case REG_TM2CNT_L+0: timer[2].initial_b[0] = byte; break;
        case REG_TM2CNT_L+1: timer[2].initial_b[1] = byte; break;
        case REG_TM3CNT_L+0: timer[3].initial_b[0] = byte; break;
        case REG_TM3CNT_L+1: timer[3].initial_b[1] = byte; break;

        case REG_TM0CNT_H: writeTimerControl(timer[0], byte); break;
        case REG_TM1CNT_H: writeTimerControl(timer[1], byte); break;
        case REG_TM2CNT_H: writeTimerControl(timer[2], byte); break;
        case REG_TM3CNT_H: writeTimerControl(timer[3], byte); break;

        case REG_KEYCNT:
            keycnt.keys_b[0] = byte;
            break;

        case REG_KEYCNT+1:
            keycnt.keys_b[1] = byte & 0x3;
            keycnt.irq       = bits<6, 1>(byte);
            keycnt.logic     = bits<7, 1>(byte);
            break;

        case REG_IME:
            intr_master = bits<0, 1>(byte);
            break;

        case REG_IF:
        case REG_IF+1:
            io[addr] &= ~byte;
            return;

        case REG_HALTCNT:
            halt = true;
            break;
        }
        io[addr] = byte;
        break;

    case PAGE_PALETTE:
        addr &= 0x3FF;
        palette[addr] = byte;
        break;

    case PAGE_VRAM:
        addr &= 0x1'FFFF;
        if (addr > 0x1'7FFF)
            addr -= 0x8000;

        vram[addr] = byte;
        break;

    case PAGE_OAM:
        addr &= 0x3FF;
        if ((addr & 0b110) != 0b110)
        {
            OAMEntry& entry = oam_entries[addr >> 3];
            switch (addr & 0x7)
            {
            case 0:
                entry.y = byte;
                break;

            case 1:
                entry.affine      = bits<0, 1>(byte);
                entry.double_size = bits<1, 1>(byte);
                entry.disabled    = bits<1, 1>(byte);
                entry.gfx_mode    = bits<2, 2>(byte);
                entry.mosaic      = bits<4, 1>(byte);
                entry.color_mode  = bits<5, 1>(byte);
                entry.shape       = bits<6, 2>(byte);
                break;

            case 2:
                entry.x_b[0] = byte;
                break;
                
            case 3:
                entry.x_b[1]   = byte & 0x1;
                entry.paramter = bits<1, 5>(byte);
                entry.flip_x   = bits<4, 1>(byte);
                entry.flip_y   = bits<5, 1>(byte);
                entry.size     = bits<6, 2>(byte);
                break;

            case 4:
                entry.tile_b[0] = byte;
                break;

            case 5:
                entry.tile_b[1] = byte & 0x3;
                entry.priority  = bits<2, 2>(byte);
                entry.palette   = bits<4, 4>(byte);
                break;
            }
        }
        oam[addr] = byte;
        break;

    case PAGE_GAMEPAK_0: case PAGE_GAMEPAK_0+1:
    case PAGE_GAMEPAK_1: case PAGE_GAMEPAK_1+1:
    case PAGE_GAMEPAK_2: case PAGE_GAMEPAK_2+1:
        break;

    default:
        addr &= 0xFFFF;
        sram[addr] = byte;
        break;
    }
}

void MMU::writeHalf(u32 addr, u16 half)
{
    addr = alignHalf(addr);
    writeByte(addr, static_cast<u8>(half));
    writeByte(addr + 1, half >> 8);
}

void MMU::writeWord(u32 addr, u32 word)
{
    addr = alignWord(addr);
    writeHalf(addr, static_cast<u16>(word));
    writeHalf(addr + 2, word >> 16);
}

void MMU::commitStatus()
{   
    io[REG_DISPSTAT] &= ~0x7;
    io[REG_DISPSTAT] |= (dispstat.vblank << 0);
    io[REG_DISPSTAT] |= (dispstat.hblank << 1);
    io[REG_DISPSTAT] |= (dispstat.vmatch << 2);
}

void MMU::writeBackgroundControlLower(BackgroundControl& control, u8 byte)
{
    control.priority     = bits<0, 2>(byte);
    control.tile_block   = bits<2, 2>(byte);
    control.mosaic       = bits<6, 1>(byte);
    control.palette_type = bits<7, 1>(byte);
 } 

void MMU::writeBackgroundControlUpper(BackgroundControl& control, u8 byte)
{
    control.map_block   = bits<0, 5>(byte);
    control.wraparound  = bits<5, 1>(byte);
    control.screen_size = bits<6, 2>(byte);
}

void MMU::writeWindow(Window& window, u8 byte)
{
    window.bg0 = bits<0, 1>(byte);
    window.bg1 = bits<1, 1>(byte);
    window.bg2 = bits<2, 1>(byte);
    window.bg3 = bits<3, 1>(byte);
    window.obj = bits<4, 1>(byte);
    window.sfx = bits<5, 1>(byte);
}

void MMU::writeBlendLayer(BlendControl::Layer& layer, u8 byte)
{
    layer.bg0 = bits<0, 1>(byte);
    layer.bg1 = bits<1, 1>(byte);
    layer.bg2 = bits<2, 1>(byte);
    layer.bg3 = bits<3, 1>(byte);
    layer.obj = bits<4, 1>(byte);
    layer.bdp = bits<5, 1>(byte);
}

void MMU::writeDMAControlLower(DMA::Control& control, u8 byte)
{
    control.dst_control = bits<5, 2>(byte);
    control.src_control = (control.src_control & ~0x1) | bits<7, 1>(byte);
}

void MMU::writeDMAControlUpper(DMA::Control& control, u8 byte)
{
    control.src_control = (control.src_control & ~0x2) | (bits<0, 1>(byte) << 1);
    control.repeat      = bits<1, 1>(byte);
    control.word        = bits<2, 1>(byte);
    control.gamepak_drq = bits<3, 1>(byte);
    control.timing      = bits<4, 2>(byte);
    control.irq         = bits<6, 1>(byte);
    control.enable      = bits<7, 1>(byte);
}

void MMU::writeTimerControl(Timer& timer, u8 byte)
{
    timer.attemptInit(byte & 0x80);
    timer.control.prescaler = bits<0, 2>(byte);
    timer.control.cascade   = bits<2, 1>(byte);
    timer.control.irq       = bits<6, 1>(byte);
    timer.control.enabled   = bits<7, 1>(byte);
}
 