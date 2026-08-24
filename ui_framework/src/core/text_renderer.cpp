#include "text_renderer.hpp"

#include <algorithm>
#include <cmath>

namespace ui
{
    TextRenderer::~TextRenderer()
    {
        if (textObject_)
            TTF_DestroyText(textObject_);
        if (textEngine_)
            TTF_DestroyRendererTextEngine(textEngine_);
        if (rasterFont_)
            TTF_CloseFont(rasterFont_);
    }

    void TextRenderer::draw(SDL_Renderer *renderer, const std::string &text, TTF_Font *font,
                            const LayoutPosition &position, float wrapWidth, Color color)
    {
        if (!renderer || !font || text.empty())
            return;

        float scale = 1.0f;
        SDL_FRect presentationRect{};
        if (getIntegerPresentationScale(renderer, scale, presentationRect))
            ensureRasterFont(font, scale);

        TTF_Font *renderFont = rasterFont_ && rasterScale_ == scale ? rasterFont_ : font;
        if (!ensureTextObject(renderer, renderFont, text))
            return;

        const int physicalWrapWidth = wrapWidth > 0.0f
            ? std::max(1, static_cast<int>(std::floor(wrapWidth * scale)))
            : 0;
        TTF_SetTextWrapWidth(textObject_, physicalWrapWidth);
        TTF_SetTextColor(textObject_, color.r, color.g, color.b, color.a);
        TTF_DrawRendererText(textObject_, position.x * scale, position.y * scale);
    }

    bool TextRenderer::getIntegerPresentationScale(SDL_Renderer *renderer, float &scale,
                                                     SDL_FRect &presentationRect) const noexcept
    {
        scale = 1.0f;
        presentationRect = {};
        if (!renderer)
            return false;

        int logicalWidth = 0, logicalHeight = 0;
        SDL_RendererLogicalPresentation mode = SDL_LOGICAL_PRESENTATION_DISABLED;
        if (!SDL_GetRenderLogicalPresentation(renderer, &logicalWidth, &logicalHeight, &mode) ||
            mode != SDL_LOGICAL_PRESENTATION_INTEGER_SCALE || logicalWidth <= 0 || logicalHeight <= 0 ||
            !SDL_GetRenderLogicalPresentationRect(renderer, &presentationRect))
            return false;

        const float scaleX = presentationRect.w / static_cast<float>(logicalWidth);
        const float scaleY = presentationRect.h / static_cast<float>(logicalHeight);
        if (scaleX <= 0.0f || std::abs(scaleX - scaleY) > 0.001f)
            return false;
        scale = scaleX;
        return scale > 1.0f;
    }

    bool TextRenderer::ensureRasterFont(TTF_Font *font, float scale)
    {
        if (!font || scale <= 1.0f)
            return false;
        const Uint32 generation = TTF_GetFontGeneration(font);
        if (rasterFont_ && rasterSourceFont_ == font && rasterScale_ == scale && rasterFontGeneration_ == generation)
            return true;
        releaseRasterFont();
        rasterFont_ = TTF_CopyFont(font);
        if (!rasterFont_)
            return false;
        if (!TTF_SetFontSize(rasterFont_, TTF_GetFontSize(font) * scale))
        {
            releaseRasterFont();
            return false;
        }
        rasterSourceFont_ = font;
        rasterScale_ = scale;
        rasterFontGeneration_ = generation;
        return true;
    }

    void TextRenderer::releaseTextObject() noexcept
    {
        if (textObject_)
            TTF_DestroyText(textObject_);
        if (textEngine_)
            TTF_DestroyRendererTextEngine(textEngine_);
        textObject_ = nullptr;
        textEngine_ = nullptr;
        cachedRenderer_ = nullptr;
        cachedTextFont_ = nullptr;
        cachedText_.clear();
    }

    void TextRenderer::releaseRasterFont() noexcept
    {
        if (rasterFont_)
            TTF_CloseFont(rasterFont_);
        rasterFont_ = nullptr;
        rasterSourceFont_ = nullptr;
        rasterScale_ = 1.0f;
        rasterFontGeneration_ = 0;
    }

    bool TextRenderer::ensureTextObject(SDL_Renderer *renderer, TTF_Font *font, const std::string &text)
    {
        if (!renderer || !font)
            return false;
        if (cachedRenderer_ != renderer || cachedTextFont_ != font || cachedText_ != text)
            releaseTextObject();
        if (!textEngine_)
            textEngine_ = TTF_CreateRendererTextEngine(renderer);
        if (!textEngine_)
            return false;
        if (!textObject_)
            textObject_ = TTF_CreateText(textEngine_, font, text.c_str(), 0);
        if (!textObject_)
            return false;
        cachedRenderer_ = renderer;
        cachedTextFont_ = font;
        cachedText_ = text;
        return true;
    }
}
