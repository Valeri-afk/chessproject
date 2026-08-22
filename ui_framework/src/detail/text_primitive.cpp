#include "text_primitive.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace
{
    class PhysicalTextRenderScope final
    {
    public:
        PhysicalTextRenderScope(SDL_Renderer *renderer,
                                int logicalWidth,
                                int logicalHeight,
                                SDL_RendererLogicalPresentation mode,
                                const SDL_FRect &presentationRect,
                                float logicalScale) noexcept
            : renderer_(renderer),
              logicalWidth_(logicalWidth),
              logicalHeight_(logicalHeight),
              mode_(mode),
              presentationRect_(presentationRect),
              logicalScale_(logicalScale)
        {
            if (!renderer_ || logicalScale_ <= 0.0f)
                return;

            viewportSet_ = SDL_RenderViewportSet(renderer_);
            hasViewport_ = SDL_GetRenderViewport(renderer_, &viewport_);
            clipEnabled_ = SDL_RenderClipEnabled(renderer_);
            hasClip_ = SDL_GetRenderClipRect(renderer_, &clip_);
            hasScale_ = SDL_GetRenderScale(renderer_, &scaleX_, &scaleY_);

            if (!SDL_SetRenderLogicalPresentation(
                    renderer_,
                    0,
                    0,
                    SDL_LOGICAL_PRESENTATION_DISABLED))
            {
                return;
            }

            active_ = true;

            // Logical presentation is independent from the renderer viewport,
            // so rebuild an explicit viewport in physical pixels if the caller
            // had one configured.
            if (viewportSet_ && hasViewport_)
            {
                SDL_Rect physicalViewport = viewport_;
                physicalViewport.x = static_cast<int>(std::floor(
                    presentationRect_.x + viewport_.x * logicalScale_));
                physicalViewport.y = static_cast<int>(std::floor(
                    presentationRect_.y + viewport_.y * logicalScale_));
                physicalViewport.w = static_cast<int>(std::ceil(viewport_.w * logicalScale_));
                physicalViewport.h = static_cast<int>(std::ceil(viewport_.h * logicalScale_));
                SDL_SetRenderViewport(renderer_, &physicalViewport);
            }
            else
            {
                SDL_SetRenderViewport(renderer_, nullptr);
            }

            if (clipEnabled_ && hasClip_)
            {
                SDL_Rect physicalClip = clip_;
                physicalClip.x = static_cast<int>(std::floor(clip_.x * logicalScale_));
                physicalClip.y = static_cast<int>(std::floor(clip_.y * logicalScale_));
                physicalClip.w = static_cast<int>(std::ceil(clip_.w * logicalScale_));
                physicalClip.h = static_cast<int>(std::ceil(clip_.h * logicalScale_));
                SDL_SetRenderClipRect(renderer_, &physicalClip);
            }
            else
            {
                SDL_SetRenderClipRect(renderer_, nullptr);
            }

            // Text coordinates are converted to physical pixels explicitly,
            // therefore renderer scaling must not apply a second scale.
            SDL_SetRenderScale(renderer_, 1.0f, 1.0f);
        }

        ~PhysicalTextRenderScope()
        {
            if (!renderer_ || !active_)
                return;

            SDL_SetRenderLogicalPresentation(
                renderer_,
                logicalWidth_,
                logicalHeight_,
                mode_);

            if (viewportSet_ && hasViewport_)
                SDL_SetRenderViewport(renderer_, &viewport_);
            else
                SDL_SetRenderViewport(renderer_, nullptr);

            if (clipEnabled_ && hasClip_)
                SDL_SetRenderClipRect(renderer_, &clip_);
            else
                SDL_SetRenderClipRect(renderer_, nullptr);

            if (hasScale_)
                SDL_SetRenderScale(renderer_, scaleX_, scaleY_);
        }

        PhysicalTextRenderScope(const PhysicalTextRenderScope &) = delete;
        PhysicalTextRenderScope &operator=(const PhysicalTextRenderScope &) = delete;

        bool isActive() const noexcept { return active_; }

    private:
        SDL_Renderer *renderer_ = nullptr;
        int logicalWidth_ = 0;
        int logicalHeight_ = 0;
        SDL_RendererLogicalPresentation mode_ = SDL_LOGICAL_PRESENTATION_DISABLED;
        SDL_FRect presentationRect_{};
        float logicalScale_ = 1.0f;
        SDL_Rect viewport_{};
        SDL_Rect clip_{};
        float scaleX_ = 1.0f;
        float scaleY_ = 1.0f;
        bool viewportSet_ = false;
        bool hasViewport_ = false;
        bool clipEnabled_ = false;
        bool hasClip_ = false;
        bool hasScale_ = false;
        bool active_ = false;
    };
}

namespace ui
{
    TextPrimitive::~TextPrimitive()
    {
        releaseTextObject();
        releaseRasterFont();
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
        releaseRasterFont();
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

    bool TextPrimitive::getIntegerPresentationScale(
        SDL_Renderer *renderer,
        float &scale,
        SDL_FRect &presentationRect) const noexcept
    {
        scale = 1.0f;
        presentationRect = {};

        if (!renderer)
            return false;

        int logicalWidth = 0;
        int logicalHeight = 0;
        SDL_RendererLogicalPresentation mode = SDL_LOGICAL_PRESENTATION_DISABLED;
        if (!SDL_GetRenderLogicalPresentation(
                renderer,
                &logicalWidth,
                &logicalHeight,
                &mode) ||
            mode != SDL_LOGICAL_PRESENTATION_INTEGER_SCALE ||
            logicalWidth <= 0 ||
            logicalHeight <= 0 ||
            !SDL_GetRenderLogicalPresentationRect(renderer, &presentationRect) ||
            presentationRect.w <= 0.0f ||
            presentationRect.h <= 0.0f)
        {
            return false;
        }

        const float scaleX = presentationRect.w / static_cast<float>(logicalWidth);
        const float scaleY = presentationRect.h / static_cast<float>(logicalHeight);
        if (scaleX <= 0.0f || scaleY <= 0.0f || std::abs(scaleX - scaleY) > 0.001f)
            return false;

        scale = scaleX;
        return scale > 1.0f;
    }

    bool TextPrimitive::ensureRasterFont(float scale)
    {
        if (!font_ || scale <= 1.0f)
            return false;

        const Uint32 generation = TTF_GetFontGeneration(font_);
        if (rasterFont_ &&
            rasterScale_ == scale &&
            rasterFontGeneration_ == generation)
        {
            return true;
        }

        releaseRasterFont();

        rasterFont_ = TTF_CopyFont(font_);
        if (!rasterFont_)
            return false;

        const float rasterSize = TTF_GetFontSize(font_) * scale;
        if (rasterSize <= 0.0f || !TTF_SetFontSize(rasterFont_, rasterSize))
        {
            releaseRasterFont();
            return false;
        }

        rasterScale_ = scale;
        rasterFontGeneration_ = generation;
        return true;
    }

    void TextPrimitive::draw(SDL_Renderer *renderer, const LayoutPosition &position, const LayoutSize &size)
    {
        if (!renderer || !font_ || text_.empty())
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

        float scale = 1.0f;
        SDL_FRect presentationRect{};
        const bool usePhysicalText = getIntegerPresentationScale(renderer, scale, presentationRect) &&
                                     ensureRasterFont(scale);

        if (!usePhysicalText)
        {
            if (!ensureTextObject(renderer, font_))
                return;

            TTF_SetTextColor(textObject_, color_.r, color_.g, color_.b, color_.a);
            TTF_DrawRendererText(textObject_, x, y);
            return;
        }

        // Keep layout/alignment in logical coordinates, but rasterize the glyphs
        // at the physical resolution and draw them directly into the physical
        // render target. This avoids scaling an 8px glyph bitmap up to 24px.
        const float physicalX = presentationRect.x + x * scale;
        const float physicalY = presentationRect.y + y * scale;

        PhysicalTextRenderScope scope(
            renderer,
            [&]()
            {
                int logicalWidth = 0;
                int logicalHeight = 0;
                SDL_RendererLogicalPresentation mode = SDL_LOGICAL_PRESENTATION_DISABLED;
                SDL_GetRenderLogicalPresentation(
                    renderer,
                    &logicalWidth,
                    &logicalHeight,
                    &mode);
                return std::array<int, 2>{logicalWidth, logicalHeight};
            }()[0],
            [&]()
            {
                int logicalWidth = 0;
                int logicalHeight = 0;
                SDL_RendererLogicalPresentation mode = SDL_LOGICAL_PRESENTATION_DISABLED;
                SDL_GetRenderLogicalPresentation(
                    renderer,
                    &logicalWidth,
                    &logicalHeight,
                    &mode);
                return std::array<int, 2>{logicalWidth, logicalHeight};
            }()[1],
            SDL_LOGICAL_PRESENTATION_INTEGER_SCALE,
            presentationRect,
            scale);

        if (!scope.isActive())
            return;

        if (!ensureTextObject(renderer, rasterFont_))
            return;

        TTF_SetTextColor(textObject_, color_.r, color_.g, color_.b, color_.a);
        TTF_DrawRendererText(textObject_, physicalX, physicalY);
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
        cachedTextFont_ = nullptr;
    }

    void TextPrimitive::releaseRasterFont() noexcept
    {
        if (rasterFont_)
        {
            TTF_CloseFont(rasterFont_);
            rasterFont_ = nullptr;
        }

        rasterScale_ = 1.0f;
        rasterFontGeneration_ = 0;
    }

    bool TextPrimitive::ensureTextObject(SDL_Renderer *renderer, TTF_Font *font)
    {
        if (!renderer || !font)
            return false;

        if (cachedRenderer_ != renderer || cachedTextFont_ != font)
        {
            releaseTextObject();
            cachedRenderer_ = renderer;
            cachedTextFont_ = font;
        }

        if (!textEngine_)
        {
            textEngine_ = TTF_CreateRendererTextEngine(renderer);
            if (!textEngine_)
            {
                cachedRenderer_ = nullptr;
                cachedTextFont_ = nullptr;
                return false;
            }
        }

        if (!textObject_)
        {
            textObject_ = TTF_CreateText(textEngine_, font, text_.c_str(), 0);
            if (!textObject_)
                return false;
        }

        return true;
    }
}
