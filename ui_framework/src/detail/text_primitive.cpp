#include "text_primitive.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace ui
{
    namespace
    {
        float getPresentationScale(SDL_Renderer *renderer, float &scaleX, float &scaleY)
        {
            scaleX = 1.0f;
            scaleY = 1.0f;

            int logicalWidth = 0;
            int logicalHeight = 0;
            SDL_RendererLogicalPresentation mode = SDL_LOGICAL_PRESENTATION_DISABLED;
            SDL_FRect presentationRect{};
            float renderScaleX = 1.0f;
            float renderScaleY = 1.0f;

            if (!renderer ||
                !SDL_GetRenderLogicalPresentation(renderer, &logicalWidth, &logicalHeight, &mode) ||
                mode == SDL_LOGICAL_PRESENTATION_DISABLED ||
                logicalWidth <= 0 || logicalHeight <= 0 ||
                !SDL_GetRenderLogicalPresentationRect(renderer, &presentationRect) ||
                !SDL_GetRenderScale(renderer, &renderScaleX, &renderScaleY))
            {
                return 1.0f;
            }

            // A custom viewport or render scale changes the mapping between the
            // framework's logical coordinates and the target. Keep the original
            // path in that case instead of guessing the client's presentation.
            if (std::abs(renderScaleX - 1.0f) > 0.0001f ||
                std::abs(renderScaleY - 1.0f) > 0.0001f ||
                SDL_RenderViewportSet(renderer))
            {
                return 1.0f;
            }

            scaleX = presentationRect.w / static_cast<float>(logicalWidth);
            scaleY = presentationRect.h / static_cast<float>(logicalHeight);

            if (scaleX <= 1.0f || scaleY <= 1.0f)
                return 1.0f;

            return std::min(scaleX, scaleY);
        }
    }

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

        if (renderFontScale_ <= 1.0f)
        {
            TTF_DrawRendererText(textObject_, x, y);
            return;
        }

        int logicalWidth = 0;
        int logicalHeight = 0;
        SDL_RendererLogicalPresentation mode = SDL_LOGICAL_PRESENTATION_DISABLED;
        SDL_FRect presentationRect{};
        if (!SDL_GetRenderLogicalPresentation(renderer, &logicalWidth, &logicalHeight, &mode) ||
            mode == SDL_LOGICAL_PRESENTATION_DISABLED ||
            logicalWidth <= 0 || logicalHeight <= 0 ||
            !SDL_GetRenderLogicalPresentationRect(renderer, &presentationRect))
        {
            TTF_DrawRendererText(textObject_, x, y);
            return;
        }

        const float scaleX = presentationRect.w / static_cast<float>(logicalWidth);
        const float scaleY = presentationRect.h / static_cast<float>(logicalHeight);

        if (scaleX <= 1.0f || scaleY <= 1.0f)
        {
            TTF_DrawRendererText(textObject_, x, y);
            return;
        }

        const float physicalX = presentationRect.x + x * scaleX;
        const float physicalY = presentationRect.y + y * scaleY;

        if (!SDL_SetRenderLogicalPresentation(renderer, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED))
        {
            TTF_DrawRendererText(textObject_, x, y);
            return;
        }

        TTF_DrawRendererText(textObject_, physicalX, physicalY);
        SDL_SetRenderLogicalPresentation(renderer, logicalWidth, logicalHeight, mode);
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

        if (renderFont_)
        {
            TTF_CloseFont(renderFont_);
            renderFont_ = nullptr;
        }

        cachedRenderer_ = nullptr;
        renderFontScale_ = 1.0f;
    }

    bool TextPrimitive::ensureTextObject(SDL_Renderer *renderer)
    {
        if (cachedRenderer_ != renderer)
        {
            releaseTextObject();
            cachedRenderer_ = renderer;
        }

        float scaleX = 1.0f;
        float scaleY = 1.0f;
        const float rasterScale = getPresentationScale(renderer, scaleX, scaleY);

        if (!renderFont_ || std::abs(renderFontScale_ - rasterScale) > 0.0001f)
        {
            if (textObject_)
            {
                TTF_DestroyText(textObject_);
                textObject_ = nullptr;
            }

            if (renderFont_)
            {
                TTF_CloseFont(renderFont_);
                renderFont_ = nullptr;
            }

            if (rasterScale > 1.0f)
            {
                renderFont_ = TTF_CopyFont(font_);
                if (!renderFont_ || !TTF_SetFontSize(renderFont_, TTF_GetFontSize(font_) * rasterScale))
                {
                    if (renderFont_)
                        TTF_CloseFont(renderFont_);
                    renderFont_ = nullptr;
                    renderFontScale_ = 1.0f;
                }
                else
                {
                    renderFontScale_ = rasterScale;
                }
            }
            else
            {
                renderFontScale_ = 1.0f;
            }
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
            TTF_Font *fontForRendering = renderFont_ ? renderFont_ : font_;
            textObject_ = TTF_CreateText(textEngine_, fontForRendering, text_.c_str(), 0);
            if (!textObject_)
                return false;
        }

        return true;
    }
}
