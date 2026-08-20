#pragma once

#include <functional>
#include <cmath>
#include "core/sdl_wrapper.hpp"
#include "game/colors.hpp"

struct LayoutSize
{
    int width;
    int height;

    bool operator==(const LayoutSize &other) const
    {
        return width == other.width && height == other.height;
    }

    bool operator!=(const LayoutSize &other) const
    {
        return !(*this == other);
    }

    bool operator>=(const LayoutSize &other) const
    {
        return width >= other.width && height >= other.height;
    }

    bool operator<=(const LayoutSize &other) const
    {
        return width <= other.width && height <= other.height;
    }

    bool operator>(const LayoutSize &other) const
    {
        return width > other.width && height > other.height;
    }

    bool operator<(const LayoutSize &other) const
    {
        return width < other.width && height < other.height;
    }

    LayoutSize operator+(const LayoutSize &other) const
    {
        return {width + other.width, height + other.height};
    }

    LayoutSize operator-(const LayoutSize &other) const
    {
        int w = width - other.width;
        int h = height - other.height;
        return {w < 0 ? 0 : w, h < 0 ? 0 : h};
    }

    LayoutSize operator*(const LayoutSize &other) const
    {
        return {width * other.width, height * other.height};
    }

    LayoutSize operator/(const LayoutSize &other) const
    {
        if (other.width == 0 || other.height == 0)
            return {0, 0};
        return {width / other.width, height / other.height};
    }

    LayoutSize operator+(int scalar) const
    {
        return {width + scalar, height + scalar};
    }

    LayoutSize operator-(int scalar) const
    {
        int w = width - scalar;
        int h = height - scalar;
        return {w < 0 ? 0 : w, h < 0 ? 0 : h};
    }

    LayoutSize operator*(int scalar) const
    {
        return {width * scalar, height * scalar};
    }

    LayoutSize operator/(int scalar) const
    {
        if (scalar == 0)
            return {0, 0};
        return {width / scalar, height / scalar};
    }

    LayoutSize operator+(float scalar) const
    {
        return {static_cast<int>(width + scalar),
                static_cast<int>(height + scalar)};
    }

    LayoutSize operator-(float scalar) const
    {
        int w = static_cast<int>(width - scalar);
        int h = static_cast<int>(height - scalar);
        return {w < 0 ? 0 : w, h < 0 ? 0 : h};
    }

    LayoutSize operator*(float scalar) const
    {
        return {static_cast<int>(std::round(width * scalar)),
                static_cast<int>(std::round(height * scalar))};
    }

    LayoutSize operator/(float scalar) const
    {
        if (scalar == 0.0f)
            return {0, 0};
        return {static_cast<int>(std::round(width / scalar)),
                static_cast<int>(std::round(height / scalar))};
    }

    LayoutSize &operator+=(int scalar)
    {
        width += scalar;
        height += scalar;
        return *this;
    }

    LayoutSize &operator-=(int scalar)
    {
        width = std::max(0, width - scalar);
        height = std::max(0, height - scalar);
        return *this;
    }

    LayoutSize &operator*=(int scalar)
    {
        width *= scalar;
        height *= scalar;
        return *this;
    }

    LayoutSize &operator/=(int scalar)
    {
        if (scalar != 0)
        {
            width /= scalar;
            height /= scalar;
        }
        return *this;
    }

    LayoutSize &operator+=(float scalar)
    {
        width = static_cast<int>(width + scalar);
        height = static_cast<int>(height + scalar);
        return *this;
    }

    LayoutSize &operator-=(float scalar)
    {
        width = std::max(0, static_cast<int>(width - scalar));
        height = std::max(0, static_cast<int>(height - scalar));
        return *this;
    }

    LayoutSize &operator*=(float scalar)
    {
        width = static_cast<int>(std::round(width * scalar));
        height = static_cast<int>(std::round(height * scalar));
        return *this;
    }

    LayoutSize &operator/=(float scalar)
    {
        if (scalar != 0.0f)
        {
            width = static_cast<int>(std::round(width / scalar));
            height = static_cast<int>(std::round(height / scalar));
        }
        return *this;
    }
};

inline LayoutSize operator+(int scalar, const LayoutSize &size)
{
    return size + scalar;
}

inline LayoutSize operator-(int scalar, const LayoutSize &size)
{
    int w = scalar - size.width;
    int h = scalar - size.height;
    return {w < 0 ? 0 : w, h < 0 ? 0 : h};
}

inline LayoutSize operator*(int scalar, const LayoutSize &size)
{
    return size * scalar;
}

inline LayoutSize operator/(int scalar, const LayoutSize &size)
{
    if (size.width == 0 || size.height == 0)
        return {0, 0};
    return {scalar / size.width, scalar / size.height};
}

inline LayoutSize operator+(float scalar, const LayoutSize &size)
{
    return size + scalar;
}

inline LayoutSize operator-(float scalar, const LayoutSize &size)
{
    int w = static_cast<int>(scalar - size.width);
    int h = static_cast<int>(scalar - size.height);
    return {w < 0 ? 0 : w, h < 0 ? 0 : h};
}

inline LayoutSize operator*(float scalar, const LayoutSize &size)
{
    return size * scalar;
}

inline LayoutSize operator/(float scalar, const LayoutSize &size)
{
    if (size.width == 0 || size.height == 0)
        return {0, 0};
    return {static_cast<int>(std::round(scalar / size.width)),
            static_cast<int>(std::round(scalar / size.height))};
}

struct LayoutPosition
{
    float x;
    float y;

    bool operator==(const LayoutPosition &other) const
    {
        return x == other.x && y == other.y;
    }

    bool operator!=(const LayoutPosition &other) const
    {
        return !(*this == other);
    }

    bool operator>=(const LayoutPosition &other) const
    {
        return x >= other.x && y >= other.y;
    }

    bool operator<=(const LayoutPosition &other) const
    {
        return x <= other.x && y <= other.y;
    }

    bool operator>(const LayoutPosition &other) const
    {
        return x > other.x && y > other.y;
    }

    bool operator<(const LayoutPosition &other) const
    {
        return x < other.x && y < other.y;
    }

    LayoutPosition operator+(const LayoutPosition &other) const
    {
        return {x + other.x, y + other.y};
    }

    LayoutPosition operator-(const LayoutPosition &other) const
    {
        float newX = x - other.x;
        float newY = y - other.y;
        return {newX < 0 ? 0.0f : newX, newY < 0 ? 0.0f : newY};
    }

    LayoutPosition operator*(const LayoutPosition &other) const
    {
        return {x * other.x, y * other.y};
    }

    LayoutPosition operator/(const LayoutPosition &other) const
    {
        if (other.x == 0.0f || other.y == 0.0f)
            return {0.0f, 0.0f};
        return {x / other.x, y / other.y};
    }

    LayoutPosition operator+(int scalar) const
    {
        return {x + static_cast<float>(scalar),
                y + static_cast<float>(scalar)};
    }

    LayoutPosition operator-(int scalar) const
    {
        float newX = x - static_cast<float>(scalar);
        float newY = y - static_cast<float>(scalar);
        return {newX < 0 ? 0.0f : newX, newY < 0 ? 0.0f : newY};
    }

    LayoutPosition operator*(int scalar) const
    {
        return {x * static_cast<float>(scalar),
                y * static_cast<float>(scalar)};
    }

    LayoutPosition operator/(int scalar) const
    {
        if (scalar == 0)
            return {0.0f, 0.0f};
        return {x / static_cast<float>(scalar),
                y / static_cast<float>(scalar)};
    }

    LayoutPosition operator+(float scalar) const
    {
        return {x + scalar, y + scalar};
    }

    LayoutPosition operator-(float scalar) const
    {
        float newX = x - scalar;
        float newY = y - scalar;
        return {newX < 0 ? 0.0f : newX, newY < 0 ? 0.0f : newY};
    }

    LayoutPosition operator*(float scalar) const
    {
        return {x * scalar, y * scalar};
    }

    LayoutPosition operator/(float scalar) const
    {
        if (scalar == 0.0f)
            return {0.0f, 0.0f};
        return {x / scalar, y / scalar};
    }

    LayoutPosition &operator+=(int scalar)
    {
        x += static_cast<float>(scalar);
        y += static_cast<float>(scalar);
        return *this;
    }

    LayoutPosition &operator-=(int scalar)
    {
        x = std::max(0.0f, x - static_cast<float>(scalar));
        y = std::max(0.0f, y - static_cast<float>(scalar));
        return *this;
    }

    LayoutPosition &operator*=(int scalar)
    {
        x *= static_cast<float>(scalar);
        y *= static_cast<float>(scalar);
        return *this;
    }

    LayoutPosition &operator/=(int scalar)
    {
        if (scalar != 0)
        {
            x /= static_cast<float>(scalar);
            y /= static_cast<float>(scalar);
        }
        return *this;
    }

    LayoutPosition &operator+=(float scalar)
    {
        x += scalar;
        y += scalar;
        return *this;
    }

    LayoutPosition &operator-=(float scalar)
    {
        x = std::max(0.0f, x - scalar);
        y = std::max(0.0f, y - scalar);
        return *this;
    }

    LayoutPosition &operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    LayoutPosition &operator/=(float scalar)
    {
        if (scalar != 0.0f)
        {
            x /= scalar;
            y /= scalar;
        }
        return *this;
    }
};

inline LayoutPosition operator+(int scalar, const LayoutPosition &pos)
{
    return pos + scalar;
}

inline LayoutPosition operator-(int scalar, const LayoutPosition &pos)
{
    float newX = static_cast<float>(scalar) - pos.x;
    float newY = static_cast<float>(scalar) - pos.y;
    return {newX < 0 ? 0.0f : newX, newY < 0 ? 0.0f : newY};
}

inline LayoutPosition operator*(int scalar, const LayoutPosition &pos)
{
    return pos * scalar;
}

inline LayoutPosition operator/(int scalar, const LayoutPosition &pos)
{
    if (pos.x == 0.0f || pos.y == 0.0f)
        return {0.0f, 0.0f};
    return {static_cast<float>(scalar) / pos.x,
            static_cast<float>(scalar) / pos.y};
}

inline LayoutPosition operator+(float scalar, const LayoutPosition &pos)
{
    return pos + scalar;
}

inline LayoutPosition operator-(float scalar, const LayoutPosition &pos)
{
    float newX = scalar - pos.x;
    float newY = scalar - pos.y;
    return {newX < 0 ? 0.0f : newX, newY < 0 ? 0.0f : newY};
}

inline LayoutPosition operator*(float scalar, const LayoutPosition &pos)
{
    return pos * scalar;
}

inline LayoutPosition operator/(float scalar, const LayoutPosition &pos)
{
    if (pos.x == 0.0f || pos.y == 0.0f)
        return {0.0f, 0.0f};
    return {scalar / pos.x, scalar / pos.y};
}

inline LayoutPosition operator+(const LayoutPosition &pos, const LayoutSize &size)
{
    return {pos.x + static_cast<float>(size.width),
            pos.y + static_cast<float>(size.height)};
}

inline LayoutPosition operator+(const LayoutSize &size, const LayoutPosition &pos)
{
    return pos + size;
}

inline LayoutPosition operator-(const LayoutPosition &pos, const LayoutSize &size)
{
    float newX = pos.x - static_cast<float>(size.width);
    float newY = pos.y - static_cast<float>(size.height);
    return {newX < 0 ? 0.0f : newX, newY < 0 ? 0.0f : newY};
}

inline LayoutPosition operator-(const LayoutSize &size, const LayoutPosition &pos)
{
    float newX = static_cast<float>(size.width) - pos.x;
    float newY = static_cast<float>(size.height) - pos.y;
    return {newX < 0 ? 0.0f : newX, newY < 0 ? 0.0f : newY};
}

inline LayoutPosition operator*(const LayoutPosition &pos, const LayoutSize &size)
{
    return {pos.x * static_cast<float>(size.width),
            pos.y * static_cast<float>(size.height)};
}

inline LayoutPosition operator*(const LayoutSize &size, const LayoutPosition &pos)
{
    return pos * size;
}

inline LayoutPosition operator/(const LayoutPosition &pos, const LayoutSize &size)
{
    if (size.width == 0 || size.height == 0)
        return {0.0f, 0.0f};
    return {pos.x / static_cast<float>(size.width),
            pos.y / static_cast<float>(size.height)};
}

inline LayoutPosition operator/(const LayoutSize &size, const LayoutPosition &pos)
{
    if (pos.x == 0.0f || pos.y == 0.0f)
        return {0.0f, 0.0f};
    return {static_cast<float>(size.width) / pos.x,
            static_cast<float>(size.height) / pos.y};
}

inline LayoutPosition &operator+=(LayoutPosition &pos, const LayoutSize &size)
{
    pos.x += static_cast<float>(size.width);
    pos.y += static_cast<float>(size.height);
    return pos;
}

inline LayoutPosition &operator-=(LayoutPosition &pos, const LayoutSize &size)
{
    pos.x = std::max(0.0f, pos.x - static_cast<float>(size.width));
    pos.y = std::max(0.0f, pos.y - static_cast<float>(size.height));
    return pos;
}

inline LayoutPosition &operator*=(LayoutPosition &pos, const LayoutSize &size)
{
    pos.x *= static_cast<float>(size.width);
    pos.y *= static_cast<float>(size.height);
    return pos;
}

inline LayoutPosition &operator/=(LayoutPosition &pos, const LayoutSize &size)
{
    if (size.width != 0 && size.height != 0)
    {
        pos.x /= static_cast<float>(size.width);
        pos.y /= static_cast<float>(size.height);
    }
    return pos;
}

struct Box
{
    LayoutPosition position;
    LayoutSize size;
    DefaultColors backgroundColor;
    int borderWidth;
    DefaultColors borderColor;
};

inline void renderUIBox(SDLWrapper &sdl, LayoutSize size, LayoutPosition pos, DefaultColors backgroundColor)
{
    if (size.width <= 0 || size.height <= 0)
        return;

    Color bgColor = getDefaultColor(backgroundColor);
    sdl.drawRect(pos.x, pos.y, size.width, size.height, bgColor.r, bgColor.g, bgColor.b, bgColor.a, true);
}

inline void renderUIBox(SDLWrapper &sdl, LayoutSize size, LayoutPosition pos, DefaultColors backgroundColor, int borderWidth, DefaultColors borderColor)
{
    if (size.width <= 0 || size.height <= 0)
        return;

    Color bgColor = getDefaultColor(backgroundColor);
    sdl.drawRect(pos.x, pos.y, size.width, size.height, bgColor.r, bgColor.g, bgColor.b, bgColor.a, true);

    if (borderWidth > 0)
    {
        Color brdColor = getDefaultColor(borderColor);

        sdl.drawRect(pos.x, pos.y, size.width, borderWidth,
                     brdColor.r, brdColor.g, brdColor.b, brdColor.a, true);

        sdl.drawRect(pos.x, pos.y + size.height - borderWidth, size.width, borderWidth,
                     brdColor.r, brdColor.g, brdColor.b, brdColor.a, true);

        if (size.height > 2 * borderWidth)
        {
            sdl.drawRect(pos.x, pos.y + borderWidth, borderWidth, size.height - 2 * borderWidth,
                         brdColor.r, brdColor.g, brdColor.b, brdColor.a, true);
        }

        if (size.height > 2 * borderWidth)
        {
            sdl.drawRect(pos.x + size.width - borderWidth, pos.y + borderWidth, borderWidth, size.height - 2 * borderWidth,
                         brdColor.r, brdColor.g, brdColor.b, brdColor.a, true);
        }
    }
}

inline bool isInsideRect(
    LayoutPosition mouse,
    LayoutPosition pos,
    LayoutSize size)
{
    return mouse >= pos && mouse <= pos + size;
};