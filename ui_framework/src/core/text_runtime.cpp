#include "text_runtime.hpp"

namespace ui
{
    LayoutSize TextRuntime::measure(const std::string &text, TTF_Font *font, float availableWidth) const noexcept
    {
        return TextPrimitive::measure(font, text, availableWidth);
    }

    void TextRuntime::draw(
        SDL_Renderer *renderer,
        const std::string &text,
        TTF_Font *font,
        TextAlignment horizontalAlignment,
        TextAlignment verticalAlignment,
        Color color,
        const LayoutPosition &position,
        const LayoutSize &size)
    {
        primitive_.draw(renderer, text, font, horizontalAlignment, verticalAlignment, color, position, size);
    }
}
