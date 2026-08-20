#include "ui_framework/core/textnode.hpp"

namespace ui
{
    const std::string &TextNode::getText() const noexcept
    {
        return text_.getText();
    }

    void TextNode::setText(std::string text)
    {
        if (text_.getText() == text)
            return;

        deferLayoutMutation(
            [text = std::move(text)](Node &node)
            {
                auto &textNode = static_cast<TextNode &>(node);
                textNode.text_.setText(text);
            });
    }

    TTF_Font *TextNode::getFont() const noexcept
    {
        return text_.getFont();
    }

    void TextNode::setFont(TTF_Font *font)
    {
        if (text_.getFont() == font)
            return;

        deferLayoutMutation(
            [font](Node &node)
            {
                static_cast<TextNode &>(node).text_.setFont(font);
            });
    }

    TextAlignment TextNode::getHorizontalAlignment() const noexcept
    {
        return text_.getHorizontalAlignment();
    }

    void TextNode::setHorizontalAlignment(TextAlignment alignment)
    {
        if (getHorizontalAlignment() == alignment)
            return;

        deferLayoutMutation(
            [alignment](Node &node)
            {
                static_cast<TextNode &>(node).text_.setHorizontalAlignment(alignment);
            });
    }

    TextAlignment TextNode::getVerticalAlignment() const noexcept
    {
        return text_.getVerticalAlignment();
    }

    void TextNode::setVerticalAlignment(TextAlignment alignment)
    {
        if (getVerticalAlignment() == alignment)
            return;

        deferLayoutMutation(
            [alignment](Node &node)
            {
                static_cast<TextNode &>(node).text_.setVerticalAlignment(alignment);
            });
    }

    Color TextNode::getColor() const noexcept
    {
        return text_.getColor();
    }

    void TextNode::setColor(const Color &color)
    {
        text_.setColor(color);
    }

    LayoutSize TextNode::measureContent(const LayoutSize &availableContent) const
    {
        return text_.measure(availableContent.width);
    }

    void TextNode::draw(SDL_Renderer *renderer)
    {
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

        text_.draw(renderer, contentPosition, contentSize);
    }
}
