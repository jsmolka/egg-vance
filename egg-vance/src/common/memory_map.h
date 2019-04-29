#pragma once

enum MemoryMap
{
    MAP_BIOS         = 0x00000000,
    MAP_WRAM         = 0x02000000,
    MAP_IWRAM        = 0x03000000,
    MAP_IO           = 0x04000000,
    MAP_PALETTE      = 0x05000000,
    MAP_VRAM         = 0x06000000,
    MAP_OAM          = 0x07000000,
    MAP_GAMEPAK_0    = 0x08000000,
    MAP_GAMEPAK_1    = 0x0A000000,
    MAP_GAMEPAK_2    = 0x0C000000,
    MAP_GAMEPAK_SRAM = 0x0E000000
};

enum MemoryRegister
{
    REG_DISPCNT  = 0x04000000,
    REG_DISPSTAT = 0x04000004,
    REG_VCOUNT   = 0x04000006,
    REG_BG0CNT   = 0x04000008,
    REG_BG1CNT   = 0x0400000A,
    REG_BG2CNT   = 0x0400000C,
    REG_BG3CNT   = 0x0400000E,
    REG_BG0VOFS  = 0x04000010,
    REG_BG0HOFS  = 0x04000012,
    REG_BG1VOFS  = 0x04000014,
    REG_BG1HOFS  = 0x04000016,
    REG_BG2VOFS  = 0x04000018,
    REG_BG2HOFS  = 0x0400001A,
    REG_BG3VOFS  = 0x0400001C,
    REG_BG3HOFS  = 0x0400001E,
    REG_BG2PA    = 0x04000020,
    REG_BG2PB    = 0x04000022,
    REG_BG2PC    = 0x04000024,
    REG_BG2PD    = 0x04000026,
    REG_BG2X     = 0x04000028,
    REG_BG2Y     = 0x0400002C,
    REG_BG3PA    = 0x04000030,
    REG_BG3PB    = 0x04000032,
    REG_BG3PC    = 0x04000034,
    REG_BG3PD    = 0x04000036,
    REG_BG3X     = 0x04000038,
    REG_BG3Y     = 0x0400003C,
    REG_WIN0H    = 0x04000040,
    REG_WIN1H    = 0x04000042,
    REG_WIN0V    = 0x04000044,
    REG_WIN1V    = 0x04000046,
    REG_WININ    = 0x04000048,
    REG_WINOUT   = 0x0400004A,
    REG_MOSAIC   = 0x0400004C,
    REG_BLDCNT   = 0x04000050,
    REG_BLDALPHA = 0x04000052,
    REG_BLDY     = 0x04000054,
    REG_KEYINPUT = 0x04000130,
    REG_KEYCNT   = 0x04000132,
    REG_IE       = 0x04000200,
    REG_IF       = 0x04000202,
    REG_WAITCNT  = 0x04000204,
    REG_IME      = 0x04000208,
};
