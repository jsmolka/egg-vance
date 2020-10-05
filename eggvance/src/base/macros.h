#pragma once

#include <shell/macros.h>

#include "base/int.h"

#define INDEXED_CASE(label, index, ...)         \
    case label + index:                         \
    {                                           \
        constexpr uint kIndex = index;          \
        constexpr uint kLabel = label + index;  \
        __VA_ARGS__;                            \
        break;                                  \
    }

#define INDEXED_CASE1(label, ...)               \
    INDEXED_CASE(label, 0, __VA_ARGS__)

#define INDEXED_CASE2(label, ...)               \
    INDEXED_CASE(label, 0, __VA_ARGS__);        \
    INDEXED_CASE(label, 1, __VA_ARGS__)

#define INDEXED_CASE4(label, ...)               \
    INDEXED_CASE(label, 0, __VA_ARGS__);        \
    INDEXED_CASE(label, 1, __VA_ARGS__);        \
    INDEXED_CASE(label, 2, __VA_ARGS__);        \
    INDEXED_CASE(label, 3, __VA_ARGS__)

#define INDEXED_CASE8(label, ...)               \
    INDEXED_CASE(label, 0, __VA_ARGS__);        \
    INDEXED_CASE(label, 1, __VA_ARGS__);        \
    INDEXED_CASE(label, 2, __VA_ARGS__);        \
    INDEXED_CASE(label, 3, __VA_ARGS__);        \
    INDEXED_CASE(label, 4, __VA_ARGS__);        \
    INDEXED_CASE(label, 5, __VA_ARGS__);        \
    INDEXED_CASE(label, 6, __VA_ARGS__);        \
    INDEXED_CASE(label, 7, __VA_ARGS__)
