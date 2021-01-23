#pragma once

#include "base/int.h"

class Length
{
public:
    Length(uint base);

    void init();
    void tick();
    bool isEnabled() const;

    uint length = 0;
    uint expire = 0;
    uint timer  = 0;
    uint base   = 0;
};