#include "text_primitive.hpp"

#include <algorithm>
#include <cmath>

namespace
{
    class PhysicalTextRenderScope final
    {
    public:
        PhysicalTextRenderScope(SDL_Renderer *renderer, int logicalWidth, int logicalHeight,
                                SDL_RendererLogicalPresentation mode, const SDL_FRect &presentationRect,
                                float logicalScale) noexcept
            : renderer_(renderer), logicalWidth_(logicalWidth), logicalHeight_(logicalHeight), mode_(mode),
              presentationRect_(presentationRect), logicalScale_(logicalScale)
        {
            if (!renderer_ || logicalScale_ <= 0.0f) return;
            viewportSet_ = SDL_RenderViewportSet(renderer_); hasViewport_ = SDL_GetRenderViewport(renderer_, &viewport_);
            clipEnabled_ = SDL_RenderClipEnabled(renderer_); hasClip_ = SDL_GetRenderClipRect(renderer_, &clip_);
            hasScale_ = SDL_GetRenderScale(renderer_, &scaleX_, &scaleY_);
            if (!SDL_SetRenderLogicalPresentation(renderer_, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED)) return;
            active_ = true;
            if (viewportSet_ && hasViewport_)
            {
                SDL_Rect physical{};
                physical.x = static_cast<int>(std::floor(presentationRect_.x + viewport_.x * logicalScale_));
                physical.y = static_cast<int>(std::floor(presentationRect_.y + viewport_.y * logicalScale_));
                physical.w = static_cast<int>(std::ceil(viewport_.w * logicalScale_));
                physical.h = static_cast<int>(std::ceil(viewport_.h * logicalScale_));
                SDL_SetRenderViewport(renderer_, &physical);
            }
            else SDL_SetRenderViewport(renderer_, nullptr);
            if (clipEnabled_ && hasClip_)
            {
                SDL_Rect physical{};
                physical.x = static_cast<int>(std::floor(presentationRect_.x + clip_.x * logicalScale_));
                physical.y = static_cast<int>(std::floor(presentationRect_.y + clip_.y * logicalScale_));
                physical.w = static_cast<int>(std::ceil(clip_.w * logicalScale_));
                physical.h = static_cast<int>(std::ceil(clip_.h * logicalScale_));
                SDL_SetRenderClipRect(renderer_, &physical);
            }
            else SDL_SetRenderClipRect(renderer_, nullptr);
            SDL_SetRenderScale(renderer_, 1.0f, 1.0f);
        }
        ~PhysicalTextRenderScope()
        {
            if (!renderer_ || !active_) return;
            SDL_SetRenderLogicalPresentation(renderer_, logicalWidth_, logicalHeight_, mode_);
            if (viewportSet_ && hasViewport_) SDL_SetRenderViewport(renderer_, &viewport_); else SDL_SetRenderViewport(renderer_, nullptr);
            if (clipEnabled_ && hasClip_) SDL_SetRenderClipRect(renderer_, &clip_); else SDL_SetRenderClipRect(renderer_, nullptr);
            if (hasScale_) SDL_SetRenderScale(renderer_, scaleX_, scaleY_);
        }
        bool isActive() const noexcept { return active_; }
        PhysicalTextRenderScope(const PhysicalTextRenderScope &) = delete;
        PhysicalTextRenderScope &operator=(const PhysicalTextRenderScope &) = delete;
    private:
        SDL_Renderer *renderer_ = nullptr;
        int logicalWidth_ = 0, logicalHeight_ = 0;
        SDL_RendererLogicalPresentation mode_ = SDL_LOGICAL_PRESENTATION_DISABLED;
        SDL_FRect presentationRect_{};
        float logicalScale_ = 1.0f;
        SDL_Rect viewport_{}, clip_{};
        float scaleX_ = 1.0f, scaleY_ = 1.0f;
        bool viewportSet_ = false, hasViewport_ = false, clipEnabled_ = false, hasClip_ = false, hasScale_ = false, active_ = false;
    };
}

namespace ui
{
    TextPrimitive::~TextPrimitive()
    {
        releaseTextObject();
        releaseRasterFont();
    }

    LayoutSize TextPrimitive::measure(TTF_Font *font, const std::string &text, float availableWidth) noexcept
    {
        if (!font || text.empty()) return {};
        int width = 0, height = 0;
        if (availableWidth > 0.0f)
        {
            const int wrapWidth = static_cast<int>(availableWidth);
            if (!TTF_GetStringSizeWrapped(font, text.c_str(), 0, wrapWidth, &width, &height)) return {};
        }
        else if (!TTF_GetStringSize(font, text.c_str(), 0, &width, &height)) return {};
        return {static_cast<float>(std::max(width, 0)), static_cast<float>(std::max(height, 0))};
    }

    bool TextPrimitive::getIntegerPresentationScale(SDL_Renderer *renderer, float &scale, SDL_FRect &presentationRect) const noexcept
    {
        scale = 1.0f; presentationRect = {};
        if (!renderer) return false;
        int logicalWidth = 0, logicalHeight = 0;
        SDL_RendererLogicalPresentation mode = SDL_LOGICAL_PRESENTATION_DISABLED;
        if (!SDL_GetRenderLogicalPresentation(renderer, &logicalWidth, &logicalHeight, &mode) ||
            mode != SDL_LOGICAL_PRESENTATION_INTEGER_SCALE || logicalWidth <= 0 || logicalHeight <= 0 ||
            !SDL_GetRenderLogicalPresentationRect(renderer, &presentationRect) || presentationRect.w <= 0.0f || presentationRect.h <= 0.0f)
            return false;
        const float scaleX = presentationRect.w / static_cast<float>(logicalWidth);
        const float scaleY = presentationRect.h / static_cast<float>(logicalHeight);
        if (scaleX <= 0.0f || scaleY <= 0.0f || std::abs(scaleX - scaleY) > 0.001f) return false;
        scale = scaleX;
        return scale > 1.0f;
    }

    bool TextPrimitive::ensureRasterFont(TTF_Font *font, float scale)
    {
        if (!font || scale <= 1.0f) return false;
        const Uint32 generation = TTF_GetFontGeneration(font);
        if (rasterFont_ && rasterSourceFont_ == font && rasterScale_ == scale && rasterFontGeneration_ == generation) return true;
        releaseRasterFont();
        rasterFont_ = TTF_CopyFont(font);
        if (!rasterFont_) return false;
        const float rasterSize = TTF_GetFontSize(font) * scale;
        if (rasterSize <= 0.0f || !TTF_SetFontSize(rasterFont_, rasterSize)) { releaseRasterFont(); return false; }
        rasterSourceFont_ = font; rasterScale_ = scale; rasterFontGeneration_ = generation;
        return true;
    }

    void TextPrimitive::draw(SDL_Renderer *renderer, const std::string &text, TTF_Font *font,
                             TextAlignment horizontalAlignment, TextAlignment verticalAlignment, Color color,
                             const LayoutPosition &position, const LayoutSize &size)
    {
        if (!renderer || !font || text.empty()) return;
        const LayoutSize logicalTextSize = measure(font, text);
        float x = position.x, y = position.y;
        if (horizontalAlignment == TextAlignment::CENTER) x += (size.width - logicalTextSize.width) * 0.5f;
        else if (horizontalAlignment == TextAlignment::END) x += size.width - logicalTextSize.width;
        if (verticalAlignment == TextAlignment::CENTER) y += (size.height - logicalTextSize.height) * 0.5f;
        else if (verticalAlignment == TextAlignment::END) y += size.height - logicalTextSize.height;

        float scale = 1.0f; SDL_FRect presentationRect{};
        const bool usePhysicalText = getIntegerPresentationScale(renderer, scale, presentationRect) && ensureRasterFont(font, scale);
        TTF_Font *renderFont = usePhysicalText ? rasterFont_ : font;

        if (!ensureTextObject(renderer, renderFont, text)) return;
        TTF_SetTextColor(textObject_, color.r, color.g, color.b, color.a);

        if (!usePhysicalText)
        {
            TTF_DrawRendererText(textObject_, x, y);
            return;
        }

        int logicalWidth = 0, logicalHeight = 0;
        SDL_RendererLogicalPresentation mode = SDL_LOGICAL_PRESENTATION_DISABLED;
        if (!SDL_GetRenderLogicalPresentation(renderer, &logicalWidth, &logicalHeight, &mode)) return;
        PhysicalTextRenderScope scope(renderer, logicalWidth, logicalHeight, mode, presentationRect, scale);
        if (!scope.isActive()) return;
        TTF_DrawRendererText(textObject_, presentationRect.x + x * scale, presentationRect.y + y * scale);
    }

    void TextPrimitive::releaseTextObject() noexcept
    {
        if (textObject_) { TTF_DestroyText(textObject_); textObject_ = nullptr; }
        if (textEngine_) { TTF_DestroyRendererTextEngine(textEngine_); textEngine_ = nullptr; }
        cachedRenderer_ = nullptr; cachedTextFont_ = nullptr; cachedText_.clear();
    }

    void TextPrimitive::releaseRasterFont() noexcept
    {
        if (rasterFont_) { TTF_CloseFont(rasterFont_); rasterFont_ = nullptr; }
        rasterSourceFont_ = nullptr; rasterScale_ = 1.0f; rasterFontGeneration_ = 0;
    }

    bool TextPrimitive::ensureTextObject(SDL_Renderer *renderer, TTF_Font *font, const std::string &text)
    {
        if (!renderer || !font) return false;
        if (cachedRenderer_ != renderer || cachedTextFont_ != font || cachedText_ != text) releaseTextObject();
        if (!cachedRenderer_) cachedRenderer_ = renderer;
        if (!cachedTextFont_) cachedTextFont_ = font;
        if (!textEngine_)
        {
            textEngine_ = TTF_CreateRendererTextEngine(renderer);
            if (!textEngine_) { cachedRenderer_ = nullptr; cachedTextFont_ = nullptr; return false; }
        }
        if (!textObject_)
        {
            textObject_ = TTF_CreateText(textEngine_, font, text.c_str(), 0);
            if (!textObject_) return false;
            cachedText_ = text;
        }
        return true;
    }
}