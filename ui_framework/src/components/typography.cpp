#include "ui_framework/components/typography.hpp"

#include <algorithm>
#include <utility>

#include "text_primitive.hpp"

namespace ui
{
    Typography::Typography() : textPrimitive_(std::make_unique<TextPrimitive>()) {}
    Typography::~Typography() = default;

    const std::string &Typography::getText() const noexcept { return textLayout_.getText(); }

    void Typography::setText(std::string text)
    {
        if (textLayout_.getText() == text) return;
        textLayout_.setText(std::move(text));
    }

    TTF_Font *Typography::getFont() const noexcept { return textLayout_.getFont(); }

    void Typography::setFont(TTF_Font *font)
    {
        if (textLayout_.getFont() == font) return;
        textLayout_.setFont(font);
    }

    TextAlignment Typography::getHorizontalAlignment() const noexcept { return horizontalAlignment_; }

    void Typography::setHorizontalAlignment(TextAlignment alignment)
    {
        if (horizontalAlignment_ == alignment) return;
        horizontalAlignment_ = alignment;
    }

    TextAlignment Typography::getVerticalAlignment() const noexcept { return verticalAlignment_; }

    void Typography::setVerticalAlignment(TextAlignment alignment)
    {
        if (verticalAlignment_ == alignment) return;
        verticalAlignment_ = alignment;
    }

    Color Typography::getColor() const noexcept { return color_; }

    void Typography::setColor(const Color &color) { color_ = color; }

    void Typography::setVariant(Variant variant) noexcept { variant_ = variant; }

    Typography::Variant Typography::getVariant() const noexcept { return variant_; }

    LayoutSize Typography::measureContent(const LayoutSize &availableContent) const
    {
        return textLayout_.measure(availableContent.width);
    }

    void Typography::draw(SDL_Renderer *renderer)
    {
        if (!textPrimitive_) return;

        const LayoutPosition position = getActualPosition();
        const LayoutSize size = getActualSize();
        const Padding padding = getPadding();
        const Border border = getBorder();
        const LayoutPosition contentPosition{
            position.x + border.left + padding.left,
            position.y + border.top + padding.top};
        const LayoutSize contentSize{
            std::max(0.0f, size.width - border.left - border.right - padding.left - padding.right),
            std::max(0.0f, size.height - border.top - border.bottom - padding.top - padding.bottom)};

        textPrimitive_->draw(
            renderer,
            textLayout_.getText(),
            textLayout_.getFont(),
            horizontalAlignment_,
            verticalAlignment_,
            color_,
            contentPosition,
            contentSize);
    }
}
