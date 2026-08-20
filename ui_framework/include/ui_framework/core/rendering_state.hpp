#pragma once

#include <algorithm>
#include <cmath>
#include <limits>

#include <SDL3/SDL.h>

#include "ui_framework/core/node.hpp"

namespace ui
{
    inline SDL_Rect toSDLRect(const Node &node) noexcept
    {
        const LayoutPosition position = node.getActualPosition();
        const LayoutSize size = node.getActualSize();

        const float left = std::isfinite(position.x) ? position.x : 0.0f;
        const float top = std::isfinite(position.y) ? position.y : 0.0f;
        const float width = std::isfinite(size.width) ? std::max(0.0f, size.width) : 0.0f;
        const float height = std::isfinite(size.height) ? std::max(0.0f, size.height) : 0.0f;
        const float right = left + width;
        const float bottom = top + height;

        const auto clampInt = [](float value) noexcept -> int
        {
            const float safe = std::clamp(
                value,
                static_cast<float>(std::numeric_limits<int>::min()),
                static_cast<float>(std::numeric_limits<int>::max()));
            return static_cast<int>(safe);
        };

        const int x = clampInt(std::floor(left));
        const int y = clampInt(std::floor(top));
        const int rightInt = clampInt(std::ceil(right));
        const int bottomInt = clampInt(std::ceil(bottom));

        return {
            x,
            y,
            std::max(0, rightInt - x),
            std::max(0, bottomInt - y)};
    }

    inline SDL_Rect intersectRects(
        const SDL_Rect &a,
        const SDL_Rect &b) noexcept
    {
        const int left = std::max(a.x, b.x);
        const int top = std::max(a.y, b.y);
        const int right = std::min(a.x + a.w, b.x + b.w);
        const int bottom = std::min(a.y + a.h, b.y + b.h);

        if (right <= left || bottom <= top)
            return {left, top, 0, 0};

        return {left, top, right - left, bottom - top};
    }

    class RendererStateScope final
    {
    public:
        explicit RendererStateScope(SDL_Renderer *renderer) noexcept
            : renderer_(renderer)
        {
            if (!renderer_)
                return;

            target_ = SDL_GetRenderTarget(renderer_);
            hasViewport_ = SDL_GetRenderViewport(renderer_, &viewport_);
            viewportSet_ = SDL_RenderViewportSet(renderer_);
            clipEnabled_ = SDL_RenderClipEnabled(renderer_);
            hasClip_ = SDL_GetRenderClipRect(renderer_, &clip_);
            hasScale_ = SDL_GetRenderScale(renderer_, &scaleX_, &scaleY_);
            hasColor_ = SDL_GetRenderDrawColor(renderer_, &r_, &g_, &b_, &a_);
            hasBlendMode_ = SDL_GetRenderDrawBlendMode(renderer_, &blendMode_);
            hasColorScale_ = SDL_GetRenderColorScale(renderer_, &colorScale_);
        }

        ~RendererStateScope()
        {
            if (!renderer_)
                return;

            if (SDL_GetRenderTarget(renderer_) != target_)
                SDL_SetRenderTarget(renderer_, target_);

            if (hasViewport_)
            {
                if (viewportSet_)
                    SDL_SetRenderViewport(renderer_, &viewport_);
                else
                    SDL_SetRenderViewport(renderer_, nullptr);
            }

            if (hasScale_)
                SDL_SetRenderScale(renderer_, scaleX_, scaleY_);

            if (clipEnabled_ && hasClip_)
                SDL_SetRenderClipRect(renderer_, &clip_);
            else
                SDL_SetRenderClipRect(renderer_, nullptr);

            if (hasColor_)
                SDL_SetRenderDrawColor(renderer_, r_, g_, b_, a_);

            if (hasBlendMode_)
                SDL_SetRenderDrawBlendMode(renderer_, blendMode_);

            if (hasColorScale_)
                SDL_SetRenderColorScale(renderer_, colorScale_);
        }

        RendererStateScope(const RendererStateScope &) = delete;
        RendererStateScope &operator=(const RendererStateScope &) = delete;

    private:
        SDL_Renderer *renderer_ = nullptr;
        SDL_Texture *target_ = nullptr;
        SDL_Rect viewport_{};
        SDL_Rect clip_{};
        float scaleX_ = 1.0f;
        float scaleY_ = 1.0f;
        float colorScale_ = 1.0f;
        Uint8 r_ = 0;
        Uint8 g_ = 0;
        Uint8 b_ = 0;
        Uint8 a_ = 255;
        SDL_BlendMode blendMode_ = SDL_BLENDMODE_NONE;
        bool hasViewport_ = false;
        bool viewportSet_ = false;
        bool clipEnabled_ = false;
        bool hasClip_ = false;
        bool hasScale_ = false;
        bool hasColor_ = false;
        bool hasBlendMode_ = false;
        bool hasColorScale_ = false;
    };
}
