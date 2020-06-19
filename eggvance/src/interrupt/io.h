#pragma once

#include "base/register.h"

struct IrqIo
{
    XRegister<4, 0x0001> master;
    XRegister<2, 0x3FFF> enable;

    class IrqRequest : public XRegister<2, 0x3FFF>
    {
    public:
        template<uint Index>
        void write(u8 byte)
        {
            data[Index] &= ~(byte & mask<Index, kMask>());
        }
    } request;
};
