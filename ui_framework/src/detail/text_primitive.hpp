#pragma once

#include <string>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "ui_framework/types.hpp"

namespace ui
{
    // Internal runtime text renderer shared by text-bearing nodes/components.
    // It does not own client-provided fonts. It may own derived rasterization
    // resources created from those fonts for physical-resolution rendering.
    class TextPrimitive
    {
    public:
        TextPrimitive() = default;
        ~TextPrimitive();

        TextPrimitive(const TextPrimitive &) = delete;
        TextPrimitive &operator=(const TextPrimitive &) = delete;

        static LayoutSize measure(
            TTF_Font *font,
            const std::string &text,
            float availableWidth = -1.0f) noexcept;

        void draw(
            SDL_Renderer *renderer,
            const std::string &text,
            TTF_Font *font,
            TextAlignment horizontalAlignment,
            TextAlignment verticalAlignment,
            Color color,
            const LayoutPosition &position,
            const LayoutSize &size);

        class ScopedCurrent final
        {
        public:
            explicit ScopedCurrent(TextPrimitive &primitive) noexcept;
            ~ScopedCurrent();

            ScopedCurrent(const ScopedCurrent &) = delete;
            ScopedCurrent &operator=(const ScopedCurrent &) = delete;

        private:
            TextPrimitive *previous_ = nullptr;
        };

        static TextPrimitive *current() noexcept;

    private:
        void releaseTextObject() noexcept;
        void releaseRasterFont() noexcept;
        bool ensureTextObject(SDL_Renderer *renderer, TTF_Font *font, const std::string &text);
        bool ensureRasterFont(TTF_Font *font, float scale);
        bool getIntegerPresentationScale(
            SDL_Renderer *renderer,
            float &scale,
            SDL_FRect &presentationRect) const noexcept;

        SDL_Renderer *cachedRenderer_ = nullptr;
        TTF_Font *cachedTextFont_ = nullptr;
        std::string cachedText_;
        TTF_TextEngine *textEngine_ = nullptr;
        TTF_Text *textObject_ = nullptr;

        TTF_Font *rasterFont_ = nullptr;
        TTF_Font *rasterSourceFont_ = nullptr;
        float rasterScale_ = 1.0f;
        Uint32 rasterFontGeneration_ = 0;
    };
}
