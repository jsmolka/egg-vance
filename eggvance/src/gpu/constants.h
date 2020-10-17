#pragma once

#include "point.h"

inline constexpr auto kScreen      = Point(240, 160);
inline constexpr auto kColorMask   = 0x7FFF;
inline constexpr auto kTransparent = 0x8000;
inline constexpr auto kRefreshRate = 59.737;

enum ColorMode
{
    kColorMode16x16,
    kColorMode256x1
};

enum BlendMode
{
    kBlendDisabled,
    kBlendAlpha,
    kBlendWhite,
    kBlendBlack
};

enum ObjectMapping
{
    kObjectMapping2d,
    kObjectMapping1d
};