#pragma once

#include "common/integer.h"

class Window
{
public:
    void write(u8 byte);

    int bg0;  // BG0 enabled
    int bg1;  // BG1 enabled
    int bg2;  // BG2 enabled
    int bg3;  // BG3 enabled
    int obj;  // OBJ enabled
    int sfx;  // SFX enabled
};
