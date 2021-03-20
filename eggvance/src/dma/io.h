#pragma once

#include <functional>

#include "base/register.h"

class DmaSource : public RegisterW<u32>
{
public:
    DmaSource(uint id);

    operator u32() const;
};

class DmaDestination : public RegisterW<u32>
{
public:
    DmaDestination(uint id);

    operator u32() const;

    bool isFifo() const;
};

class DmaCount : public RegisterW<u16>
{
public:
    DmaCount(uint id);

    operator uint() const;
};

class DmaControl : public Register<u16>
{
public:
    enum Control
    {
        kIncrement,
        kDecrement,
        kFixed,
        kReload
    };

    enum Timing
    {
        kImmediate,
        kVBlank,
        kHBlank,
        kSpecial
    };

    DmaControl(uint id);

    void write(uint index, u8 byte);
    void setEnabled(bool enabled);

    uint dadcnt = 0;
    uint sadcnt = 0;
    uint repeat = 0;
    uint word   = 0;
    uint timing = 0;
    uint irq    = 0;
    uint enable = 0;

    std::function<void(bool)> on_write;
};
