#pragma once

#include "base/bits.h"
#include "base/register.h"

struct DmaIo
{
    XRegisterW<4> sad;
    XRegisterW<4> dad;

    class Count : public XRegisterW<2>
    {
    public:
        uint count(uint id) const
        {
            if (value == 0)
                return id < 3 ? 0x4000 : 0x1'0000;
            else
                return value;
        }
    } count;

    class Control : public XRegister<2>
    {
    public:
        static constexpr uint kEnable = 1 << 7;

        template<uint Index, uint Mask>
        void write(u8 byte)
        {
            data[Index] = byte & (Mask >> (CHAR_BIT * Index));

            if (Index == 0)
            {
                dadcnt = bits::seq<5, 2>(byte);
                sadcnt = bits::seq<7, 1>(byte) << 0 | (sadcnt & ~0x1);
            }
            else
            {
                sadcnt = bits::seq<0, 1>(byte) << 1 | (sadcnt & ~0x2);
                repeat = bits::seq<1, 1>(byte);
                word   = bits::seq<2, 1>(byte);
                drq    = bits::seq<3, 1>(byte);
                timing = bits::seq<4, 2>(byte);
                irq    = bits::seq<6, 1>(byte);
                reload = bits::seq<7, 1>(byte) ^ enable;
                enable = bits::seq<7, 1>(byte);
            }
        }

        uint dadcnt{};
        uint sadcnt{};
        uint repeat{};
        uint word{};
        uint drq{};
        uint timing{};
        uint irq{};
        uint enable{};
        bool reload{};
    } control;
};
