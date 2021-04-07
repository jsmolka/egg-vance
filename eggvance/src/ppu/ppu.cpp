#include "ppu.h"

#include <algorithm>
#include <numeric>

#include "constants.h"
#include "arm/arm.h"
#include "base/bit.h"
#include "base/config.h"
#include "dma/dma.h"
#include "frontend/videocontext.h"

Ppu::Ppu()
{
    for (u32 color = 0; color < argb.size(); ++color)
    {
        uint r = bit::seq< 0, 5>(color) << 3 | 0x7;
        uint g = bit::seq< 5, 5>(color) << 3 | 0x7;
        uint b = bit::seq<10, 5>(color) << 3 | 0x7;

        if (config.lcd_color)
        {
            constexpr double kDarken   = 0.5;
            constexpr double kGammaOut = 2.2;
            constexpr double kGammaLcd = 2.5;

            double rs = std::pow(r / 255.0, kGammaOut + kDarken);
            double gs = std::pow(g / 255.0, kGammaOut + kDarken);
            double bs = std::pow(b / 255.0, kGammaOut + kDarken);

            double rd = std::pow(0.84 * rs + 0.18 * gs + 0.00 * bs, 1 / kGammaLcd + kDarken * 0.125);
            double gd = std::pow(0.09 * rs + 0.67 * gs + 0.26 * bs, 1 / kGammaLcd + kDarken * 0.125);
            double bd = std::pow(0.15 * rs + 0.10 * gs + 0.75 * bs, 1 / kGammaLcd + kDarken * 0.125);

            r = static_cast<uint>(std::min(1.0, rd) * 255);
            g = static_cast<uint>(std::min(1.0, gd) * 255);
            b = static_cast<uint>(std::min(1.0, bd) * 255);
        }

        argb[color] = 0xFF00'0000 | (r << 16) | (g << 8) | b;
    }
}

void Ppu::init()
{
    events.hblank = [this](u64 late)
    {
        hblank(late);
    };

    events.hblank_end = [this](u64 late)
    {
        hblankEnd(late);
    };

    scheduler.insert(events.hblank, 1006);
}

void Ppu::scanline()
{
    if (dispcnt.blank)
    {
        auto& scanline = video_ctx.scanline(vcount);
        std::fill(scanline.begin(), scanline.end(), 0xFFFF'FFFF);
        return;
    }

    for (auto& background : backgrounds)
        background.buffer.flip();

    if (objects_exist)
    {
        objects.fill(ObjectLayer());
        objects_exist = false;
        objects_alpha = false;
    }

    if (dispcnt.layers & Layer::Flag::Obj)
    {
        renderObjects();
    }

    switch (dispcnt.mode)
    {
    case 0:
        renderBg(&Ppu::renderBgMode0, backgrounds[0]);
        renderBg(&Ppu::renderBgMode0, backgrounds[1]);
        renderBg(&Ppu::renderBgMode0, backgrounds[2]);
        renderBg(&Ppu::renderBgMode0, backgrounds[3]);
        collapse(uint(Layer::Flag::Bg0 | Layer::Flag::Bg1 | Layer::Flag::Bg2 | Layer::Flag::Bg3));
        break;

    case 1:
        renderBg(&Ppu::renderBgMode0, backgrounds[0]);
        renderBg(&Ppu::renderBgMode0, backgrounds[1]);
        renderBg(&Ppu::renderBgMode2, backgrounds[2]);
        collapse(uint(Layer::Flag::Bg0 | Layer::Flag::Bg1 | Layer::Flag::Bg2));
        break;

    case 2:
        renderBg(&Ppu::renderBgMode2, backgrounds[2]);
        renderBg(&Ppu::renderBgMode2, backgrounds[3]);
        collapse(uint(Layer::Flag::Bg2 | Layer::Flag::Bg3));
        break;

    case 3:
        renderBg(&Ppu::renderBgMode3, backgrounds[2]);
        collapse(uint(Layer::Flag::Bg2));
        break;

    case 4:
        renderBg(&Ppu::renderBgMode4, backgrounds[2]);
        collapse(uint(Layer::Flag::Bg2));
        break;

    case 5:
        renderBg(&Ppu::renderBgMode5, backgrounds[2]);
        collapse(uint(Layer::Flag::Bg2));
        break;
    }
}

void Ppu::hblank(u64 late)
{
    dispstat.hblank = true;

    if (dispstat.hblank_irq)
    {
        arm.raise(Irq::HBlank, late);
    }

    if (vcount < 160)
    {
        scanline();

        backgrounds[2].matrix.hblank();
        backgrounds[3].matrix.hblank();

        dma.broadcast(Dma::Event::HBlank);
    }

    if (vcount > 1 && vcount < 162)
    {
        dma.broadcast(Dma::Event::Hdma);
    }

    scheduler.insert(events.hblank_end, 226 - late);
}

void Ppu::hblankEnd(u64 late)
{
    ++vcount;

    dispstat.hblank = false;
    dispstat.vblank = vcount >= 160 && vcount < 227;

    dispstat.vmatch = vcount == dispstat.vcompare;
    if (dispstat.vmatch && dispstat.vmatch_irq)
    {
        arm.raise(Irq::VMatch, late);
    }

    if (vcount == 160)
    {
        backgrounds[2].matrix.vblank();
        backgrounds[3].matrix.vblank();

        if (dispstat.vblank_irq)
        {
            arm.raise(Irq::VBlank, late);
        }
        dma.broadcast(Dma::Event::VBlank);
    }

    scheduler.insert(events.hblank, 1006 - late);
}

void Ppu::present()
{
    if (dispcnt.isActive())
    {
        video_ctx.renderCopyTexture();
        video_ctx.renderPresent();
    }
}
