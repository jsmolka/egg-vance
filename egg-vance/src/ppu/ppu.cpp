#include "ppu.h"

#include "mmu/map.h"

PPU::PPU(MMU& mmu)
    : mmu(mmu)
    , buffer()
{
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(
        "egg-vance",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        2 * WIDTH, 2 * HEIGHT,
        SDL_WINDOW_RESIZABLE
    );
    
    renderer = SDL_CreateRenderer(
        window, 
        -1, 
        SDL_RENDERER_ACCELERATED
    );
    SDL_RenderSetIntegerScale(renderer, SDL_TRUE);
    SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_BGR555,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH, HEIGHT
    );
}

PPU::~PPU()
{
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
}

void PPU::reset()
{
    buffer.fill(0);
}

void PPU::scanline()
{
    mmu.dispstat.hblank = false;
    mmu.dispstat.vblank = false;

    // Clear line buffers
    if (mmu.dispcnt.bg0) buffer_bg0.fill(COLOR_TRANSPARENT);
    if (mmu.dispcnt.bg1) buffer_bg1.fill(COLOR_TRANSPARENT);
    if (mmu.dispcnt.bg2) buffer_bg2.fill(COLOR_TRANSPARENT);
    if (mmu.dispcnt.bg3) buffer_bg3.fill(COLOR_TRANSPARENT);

    for (auto& sprites : buffer_sprites)
        sprites.fill(COLOR_TRANSPARENT);

    // Fill line buffers
    switch (mmu.dispcnt.bg_mode)
    {
    case 0: renderMode0(); break;
    case 1: renderMode1(); break;
    case 2: renderMode2(); break;
    case 3: renderMode3(); break;
    case 4: renderMode4(); break;
    case 5: renderMode5(); break;
    }
    renderSprites();

    // Generate scanline
    int backdrop = readBgColor(0, 0);
    u16* scanline = &buffer[mmu.vcount * WIDTH];
    for (int x = 0; x < WIDTH; ++x)
    {
        u16 pixel = COLOR_TRANSPARENT;

        for (int priority = 3; priority > -1; --priority)
        {
            if (mmu.dispcnt.bg3 && mmu.bg3cnt.priority == priority)
            {
                if (buffer_bg3[x] != COLOR_TRANSPARENT)
                    pixel = buffer_bg3[x];
            }
            if (mmu.dispcnt.bg2 && mmu.bg2cnt.priority == priority)
            {
                if (buffer_bg2[x] != COLOR_TRANSPARENT)
                    pixel = buffer_bg2[x];
            }
            if (mmu.dispcnt.bg1 && mmu.bg1cnt.priority == priority)
            {
                if (buffer_bg1[x] != COLOR_TRANSPARENT)
                    pixel = buffer_bg1[x];
            }
            if (mmu.dispcnt.bg0 && mmu.bg0cnt.priority == priority)
            {
                if (buffer_bg0[x] != COLOR_TRANSPARENT)
                    pixel = buffer_bg0[x];
            }
            if (mmu.dispcnt.sprites)
            {
                if (buffer_sprites[priority][x] != COLOR_TRANSPARENT)
                    pixel = buffer_sprites[priority][x];
            }
        }

        scanline[x] = (pixel != COLOR_TRANSPARENT) ? pixel : backdrop;
    }

    // Increment reference points each scanline
    mmu.bg2x.internal += mmu.bg2pb.value();
    mmu.bg3x.internal += mmu.bg3pb.value();
    mmu.bg2y.internal += mmu.bg2pd.value();
    mmu.bg3y.internal += mmu.bg3pd.value();
}

void PPU::hblank()
{
    mmu.dispstat.hblank = true;
    mmu.dispstat.vblank = false;

    if (mmu.dispstat.hblank_irq)
    {
        // Todo: request interrupt
    }
}

void PPU::vblank()
{
    mmu.dispstat.hblank = false;
    mmu.dispstat.vblank = true;

    // Copy reference points
    mmu.bg2x.internal = mmu.bg2x.value();
    mmu.bg3x.internal = mmu.bg3x.value();
    mmu.bg2y.internal = mmu.bg2y.value();
    mmu.bg3y.internal = mmu.bg3y.value();

    if (mmu.dispstat.vblank_irq)
    {
        // Todo: request interrupt
    }
}

void PPU::next()
{
    bool vcount_match = mmu.vcount == mmu.dispstat.vcount_compare;

    mmu.vcount = (mmu.vcount + 1) % 228;
    mmu.dispstat.vcount_match = vcount_match;

    if (vcount_match && mmu.dispstat.vcount_irq)
    {
        // Todo: request interrupt
    }
}

void PPU::render()
{
    SDL_UpdateTexture(
        texture,
        0,
        buffer.data(),
        2 * WIDTH
    );
    SDL_RenderCopy(renderer, texture, 0, 0);
    SDL_RenderPresent(renderer);
}

int PPU::readBgColor(int index, int palette)
{
    if (index == 0)
        return COLOR_TRANSPARENT;

    return mmu.readHalfFast(MAP_PALETTE + 0x20 * palette + 2 * index);
}

int PPU::readFgColor(int index, int palette)
{
    return mmu.readHalfFast(MAP_PALETTE + 0x200 + 0x20 * palette + 2 * index);
}

int PPU::readPixel(u32 addr, int x, int y, PixelFormat format)
{
    if (format == BPP4)
    {
        int byte = mmu.readByteFast(
            addr + 4 * y + x / 2
        );
        return (x & 0x1) ? (byte >> 4) : (byte & 0xF);
    }
    else
    {
        return mmu.readByteFast(
            addr + 8 * y + x
        );
    }
}

int PPU::readTilePixel(u32 addr, int x, int y, bool flip_x, bool flip_y, PixelFormat format)
{
    if (flip_x) x = 7 - x;
    if (flip_y) y = 7 - y;

    return readPixel(addr, x, y, format);
}
