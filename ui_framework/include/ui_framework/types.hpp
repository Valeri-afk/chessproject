#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <vector>

namespace ui
{

    //==========================================================================
    // Layout
    //==========================================================================

    enum class Overflow
    {
        VISIBLE,
        HIDDEN
    };

    enum class MainAxisAlignment
    {
        START,
        CENTER,
        END,
        SPACE_BETWEEN
    };

    enum class CrossAxisAlignment
    {
        START,
        CENTER,
        END,
        STRETCH
    };

    enum class TextAlignment
    {
        START,
        CENTER,
        END
    };

    enum class PositionMode
    {
        Layout,
        Absolute
    };

    struct LayoutPosition
    {
        float x = 0.0f;
        float y = 0.0f;

        bool operator==(const LayoutPosition &) const = default;
        LayoutPosition operator+(const LayoutPosition &rhs) const { return {x + rhs.x, y + rhs.y}; }
        LayoutPosition operator-(const LayoutPosition &rhs) const { return {x - rhs.x, y - rhs.y}; }
        LayoutPosition &operator+=(const LayoutPosition &rhs) { x += rhs.x; y += rhs.y; return *this; }
        LayoutPosition &operator-=(const LayoutPosition &rhs) { x -= rhs.x; y -= rhs.y; return *this; }
    };

    enum class LayoutValueType
    {
        Auto,
        Value
    };

    struct LayoutValue
    {
        LayoutValueType type = LayoutValueType::Auto;
        float value = 0.0f;
        static LayoutValue autoValue() noexcept { return {LayoutValueType::Auto, 0.0f}; }
        static LayoutValue fixed(float value) noexcept { return {LayoutValueType::Value, value}; }
        bool isAuto() const noexcept { return type == LayoutValueType::Auto; }
        bool isValue() const noexcept { return type == LayoutValueType::Value; }
    };

    struct LayoutSizeValue
    {
        LayoutValue width;
        LayoutValue height;
        static LayoutSizeValue autoSize() noexcept { return {LayoutValue::autoValue(), LayoutValue::autoValue()}; }
        static LayoutSizeValue fixed(float width, float height) noexcept { return {LayoutValue::fixed(width), LayoutValue::fixed(height)}; }
    };

    struct LayoutSize
    {
        float width = 0.0f;
        float height = 0.0f;
        bool operator==(const LayoutSize &rhs) const { return width == rhs.width && height == rhs.height; }
        bool operator!=(const LayoutSize &rhs) const { return !(*this == rhs); }
        bool operator<(const LayoutSize &rhs) const { return width != rhs.width ? width < rhs.width : height < rhs.height; }
        bool operator>(const LayoutSize &rhs) const { return rhs < *this; }
        bool operator<=(const LayoutSize &rhs) const { return !(*this > rhs); }
        bool operator>=(const LayoutSize &rhs) const { return !(*this < rhs); }
        LayoutSize operator+(const LayoutSize &rhs) const { return {width + rhs.width, height + rhs.height}; }
        LayoutSize operator-(const LayoutSize &rhs) const { return {width - rhs.width, height - rhs.height}; }
        LayoutSize operator*(float scalar) const { return {width * scalar, height * scalar}; }
        LayoutSize operator/(float scalar) const { return {width / scalar, height / scalar}; }
        LayoutSize &operator+=(const LayoutSize &rhs) { width += rhs.width; height += rhs.height; return *this; }
        LayoutSize &operator-=(const LayoutSize &rhs) { width -= rhs.width; height -= rhs.height; return *this; }
    };

    struct Padding
    {
        float left = 0.0f;
        float right = 0.0f;
        float top = 0.0f;
        float bottom = 0.0f;
        bool operator==(const Padding &) const = default;
    };

    struct Border
    {
        float left = 0.0f;
        float right = 0.0f;
        float top = 0.0f;
        float bottom = 0.0f;
        bool operator==(const Border &) const = default;
    };

    struct LayoutConstraints
    {
        float minWidth = 0.0f;
        float maxWidth = std::numeric_limits<float>::max();
        float minHeight = 0.0f;
        float maxHeight = std::numeric_limits<float>::max();
        LayoutSize clamp(LayoutSize size) const noexcept
        {
            return {std::clamp(size.width, minWidth, maxWidth), std::clamp(size.height, minHeight, maxHeight)};
        }
    };

    //==========================================================================
    // Colors
    //==========================================================================

    struct Color
    {
        uint8_t r = 255;
        uint8_t g = 255;
        uint8_t b = 255;
        uint8_t a = 255;
        bool operator==(const Color &) const = default;
    };

    namespace Colors
    {
        inline constexpr Color green{0, 255, 0, 255};
        inline constexpr Color red{255, 100, 47, 255};
        inline constexpr Color yellow{255, 255, 0, 255};
        inline constexpr Color white{255, 255, 255, 255};
        inline constexpr Color blue{0, 0, 255, 255};
        inline constexpr Color black{0, 0, 0, 255};
        inline constexpr Color gray{128, 128, 128, 255};
        inline constexpr Color camel{181, 136, 99, 255};
        inline constexpr Color desertSand{240, 217, 181, 255};
        inline constexpr Color transparent{0, 0, 0, 0};
    }

    struct StyleProps
    {
        Color backgroundColor = Colors::transparent;
        Color borderColor = Colors::transparent;
        float borderWidth = 0.0f;
        float borderRadius = 0.0f;
    };

}