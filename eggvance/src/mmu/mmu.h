#pragma once

#include <array>
#include <memory>
#include <vector>

#include "common/integer.h"
#include "gamepak/gamepak.h"
#include "registers/include.h"
#include "ppu/oamentry.h"
#include "bios.h"
#include "dma.h"
#include "memory.h"
#include "ram.h"
#include "timer.h"

class MMU
{
public:
    MMU(std::unique_ptr<BIOS> bios);

    void reset();

    void setGamePak(std::unique_ptr<GamePak> gamepak);

    u8  readByte(u32 addr) const;
    u16 readHalf(u32 addr) const;
    u32 readWord(u32 addr) const;

    void writeByte(u32 addr, u8  byte);
    void writeHalf(u32 addr, u16 half);
    void writeWord(u32 addr, u32 word);

    void commitStatus();

    void signalDMA(DMA::Timing timing);

    DisplayControl dispcnt;
    DisplayStatus dispstat;
    u8& vcount;

    BackgroundControl bgcnt[4];
    BackgroundOffset bghofs[4];
    BackgroundOffset bgvofs[4];
    BackgroundReference bgx[2];
    BackgroundReference bgy[2];
    BackgroundParameter bgpa[2];
    BackgroundParameter bgpb[2];
    BackgroundParameter bgpc[2];
    BackgroundParameter bgpd[2];

    WindowRange winh[2];
    WindowRange winv[2];
    WindowInside winin;
    WindowOutside winout;

    Mosaic mosaic;
    BlendControl bldcnt;
    BlendAlpha bldalpha;
    BlendFade bldy;

    u16& keyinput;
    KeyControl keycnt;
    WaitControl waitcnt;

    bool intr_master;
    u16& intr_enabled;
    u16& intr_request;

    Timer timers[4];
    std::vector<DMA*> dmas_active;

    bool halt;

    std::array<OAMEntry, 128> oam_entries;

    std::unique_ptr<GamePak> gamepak;

    RAM<0x00400> io;
    RAM<0x00400> palette;
    RAM<0x18000> vram;
    RAM<0x00400> oam;

private:
    void writeBackgroundControlLower(BackgroundControl& control, u8 byte);
    void writeBackgroundControlUpper(BackgroundControl& control, u8 byte);
    void writeWindow(Window& window, u8 byte);
    void writeBlendLayer(BlendControl::Layer& layer, u8 byte);
    void writeDMAControlLower(DMA::Control& control, u8 byte);
    void writeDMAControlUpper(DMA::Control& control, u8 byte);
    void writeTimerControl(Timer& timer, u8 byte);

    DMA dmas[4];

    std::unique_ptr<BIOS> bios;

    RAM<0x40000> wram;
    RAM<0x08000> iwram;
};
