#include "ui_framework/text_node.hpp"

#include <algorithm>
#include <utility>

#include "text_primitive.hpp"

namespace ui
{
    TextNode::TextNode() : textPrimitive_(std::make_unique<TextPrimitive>()) {}
    TextNode::~TextNode() = default;

    const std::string &TextNode::getText() const noexcept { return textLayout_.getText(); }
    void TextNode::setText(std::string text)
    {
        if (textLayout_.getText() == text) return;
        textLayout_.setText(std::move(text));
    }

    TTF_Font *TextNode::getFont() const noexcept { return textLayout_.getFont(); }
    void TextNode::setFont(TTF_Font *font)
    {
        if (textLayout_.getFont() == font) return;
        textLayout_.setFont(font);
    }

    TextAlignment TextNode::getHorizontalAlignment() const noexcept { return horizontalAlignment_; }
    void TextNode::setHorizontalAlignment(TextAlignment alignment)
    {
        if (horizontalAlignment_ == alignment) return;
        horizontalAlignment_ = alignment;
    }

    TextAlignment TextNode::getVerticalAlignment() const noexcept { return verticalAlignment_; }
    void TextNode::setVerticalAlignment(TextAlignment alignment)
    {
        if (verticalAlignment_ == alignment) return;
        verticalAlignment_ = alignment;
    }

    Color TextNode::getColor() const noexcept { return color_; }
    void TextNode::setColor(const Color &color) { color_ = color; }

    LayoutSize TextNode::measureContent(const LayoutSize &availableContent) const
    {
        return textLayout_.measure(availableContent.width);
    }

    void TextNode::draw(SDL_Renderer *renderer)
    {
        if (!textPrimitive_) return;
        const LayoutPosition position = getActualPosition();
        const LayoutSize size = getActualSize();
        const Padding padding = getPadding();
        const Border border = getBorder();
        const LayoutPosition contentPosition{position.x + border.left + padding.left, position.y + border.top + padding.top};
        const LayoutSize contentSize{
            std::max(0.0f, size.width - border.left - border.right - padding.left - padding.right),
            std::max(0.0f, size.height - border.top - border.bottom - padding.top - padding.bottom)};
        textPrimitive_->draw(renderer, textLayout_.getText(), textLayout_.getFont(), horizontalAlignment_, verticalAlignment_, color_, contentPosition, contentSize);
    }
}