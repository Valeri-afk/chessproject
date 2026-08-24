#pragma once

#include <string>
#include <utility>

#include <SDL3_ttf/SDL_ttf.h>

#include "ui_framework/types.hpp"

namespace ui
{
    enum class WrapMode
    {
        WRAP,
        NO_WRAP
    };

    // Shared logical text layout state used by standalone text components and
    // text-bearing controls. It measures in framework logical coordinates;
    // rendering/rasterization belongs to the backend layer.
    class TextLayout
    {
    public:
        TextLayout() = default;
        TextLayout(const TextLayout &) = default;
        TextLayout &operator=(const TextLayout &) = default;

        void setText(std::string text) { text_ = std::move(text); }
        const std::string &getText() const noexcept { return text_; }

        // Non-owning. The caller is responsible for keeping the source font
        // alive for as long as this layout may be measured or rendered.
        void setFont(TTF_Font *font) noexcept { font_ = font; }
        TTF_Font *getFont() const noexcept { return font_; }

        void setFontSize(float logicalSize) noexcept { fontSize_ = logicalSize; }
        float getFontSize() const noexcept { return fontSize_; }

        void setWrapMode(WrapMode mode) noexcept { wrapMode_ = mode; }
        WrapMode getWrapMode() const noexcept { return wrapMode_; }

        LayoutSize measure(float availableWidth) const noexcept;

    private:
        std::string text_;
        TTF_Font *font_ = nullptr;
        float fontSize_ = 0.0f;
        WrapMode wrapMode_ = WrapMode::WRAP;
    };
}
