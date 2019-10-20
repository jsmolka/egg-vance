#pragma once

#include "common/macros.h"
#include "common/utility.h"

class BgParam
{
public:
    void reset();

    template<int index>
    inline u8 readByte();
    template<int index>
    inline void writeByte(u8 byte);

    s16 param;
};

#include "bgparam.inl"
