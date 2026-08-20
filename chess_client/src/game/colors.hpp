#pragma once

#include <SDL3/SDL.h>

enum class DefaultColors
{
    GREEN,
    RED,
    YELLOW,
    BLUE,
    WHITE,
    BLACK,
    DESERT_SAND,
    CAMEL
};

struct Color
{
    Uint8 r = 255;
    Uint8 g = 255;
    Uint8 b = 255;
    Uint8 a = 255;
};

inline Color getDefaultColor(DefaultColors color)
{
    switch (color)
    {
    case DefaultColors::GREEN:
        return {0, 255, 0, 255};
    case DefaultColors::RED:
        return {255, 100, 47, 255};
    case DefaultColors::YELLOW:
        return {255, 255, 0, 255};
    case DefaultColors::WHITE:
        return {255, 255, 255, 255};
    case DefaultColors::BLUE:
        return {0, 0, 255, 255};
    case DefaultColors::BLACK:
        return {0, 0, 0, 255};
    case DefaultColors::DESERT_SAND:
        return {240, 217, 181, 255};
    case DefaultColors::CAMEL:
        return {181, 136, 99, 255};
    }

    return {0, 0, 0, 255};
}
