#include "ui_framework/core/text_primitive.hpp"

#include <algorithm>
#include <utility>

namespace ui
{
    TextPrimitive::~TextPrimitive()
    {
        releaseTextObject();
    }

    const std::string &TextPrimitive::getText() const noexcept { return text_; }

    void TextPrimitive::setText(std::string text)
    {
        if (text_ == text)
            return;
        text_ = std::move(text);
        releaseTextObject();
    }

    TTF_Font *TextPrimitive::getFont() const noexcept { return font_; }

    void TextPrimitive::setFont(TTF_Font *font) noexcept
    {
        if (font_ == font)
            return;
        font_ = font;
        releaseTextObject();
    }

    TextAlignment TextPrimitive::getHorizontalAlignment() const noexcept { return horizontalAlignment_; }
    void TextPrimitive::setHorizontalAlignment(TextAlignment alignment) noexcept { horizontalAlignment_ = alignment; }
    TextAlignment TextPrimitive::getVerticalAlignment() const noexcept { return verticalAlignment_; }
    void TextPrimitive::setVerticalAlignment(TextAlignment alignment) noexcept { verticalAlignment_ = alignment; }
    Color TextPrimitive::getColor() const noexcept { return color_; }
    void TextPrimitive::setColor(Color color) noexcept { color_ = color; }

    LayoutSize TextPrimitive::measure(float availableWidth) const noexcept
    {
        if (!font_ || text_.empty())
            return {};

        int width = 0;
        int height = 0;

        if (availableWidth > 0.0f)
        {
            const int wrapWidth = static_cast<int>(availableWidth);
            if (!TTF_GetStringSizeWrapped(font_, text_.c_str(), 0, wrapWidth, &width, &height))
                return {};
        }
        else if (!TTF_GetStringSize(font_, text_.c_str(), 0, &width, &height))
        {
            return {};
        }

        return {static_cast<float>(std::max(width, 0)), static_cast<float>(std::max(height, 0))};
    }

    void TextPrimitive::draw(SDL_Renderer *renderer, const LayoutPosition &position, const LayoutSize &size)
    {
        if (!renderer || !font_ || text_.empty() || !ensureTextObject(renderer))
            return;

        int textWidth = 0;
        int textHeight = 0;
        if (!TTF_GetStringSize(font_, text_.c_str(), 0, &textWidth, &textHeight))
            return;

        float x = position.x;
        float y = position.y;

        if (horizontalAlignment_ == TextAlignment::CENTER)
            x += (size.width - static_cast<float>(textWidth)) * 0.5f;
        else if (horizontalAlignment_ == TextAlignment::END)
            x += size.width - static_cast<float>(textWidth);

        if (verticalAlignment_ == TextAlignment::CENTER)
            y += (size.height - static_cast<float>(textHeight)) * 0.5f;
        else if (verticalAlignment_ == TextAlignment::END)
            y += size.height - static_cast<float>(textHeight);

        TTF_SetTextColor(textObject_, color_.r, color_.g, color_.b, color_.a);
        TTF_DrawRendererText(textObject_, x, y);
    }

    void TextPrimitive::releaseTextObject() noexcept
    {
        if (textObject_)
        {
            TTF_DestroyText(textObject_);
            textObject_ = nullptr;
        }

        if (textEngine_)
        {
            TTF_DestroyRendererTextEngine(textEngine_);
            textEngine_ = nullptr;
        }

        cachedRenderer_ = nullptr;
    }

    bool TextPrimitive::ensureTextObject(SDL_Renderer *renderer)
    {
        if (cachedRenderer_ != renderer)
        {
            releaseTextObject();
            cachedRenderer_ = renderer;
        }

        if (!textEngine_)
        {
            textEngine_ = TTF_CreateRendererTextEngine(renderer);
            if (!textEngine_)
            {
                cachedRenderer_ = nullptr;
                return false;
            }
        }

        if (!textObject_)
        {
            textObject_ = TTF_CreateText(textEngine_, font_, text_.c_str(), 0);
            if (!textObject_)
                return false;
        }

        return true;
    }
}
