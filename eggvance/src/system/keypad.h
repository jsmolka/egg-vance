#pragma once

#include "registers/keycontrol.h"
#include "registers/keyinput.h"

class Keypad
{
public:
    void reset();

    void poll();

    struct IO
    {
        KeyControl keycnt;
        KeyInput keyinput;
    } io;
};

extern Keypad keypad;
