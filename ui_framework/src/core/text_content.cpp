#include "text_content.hpp"

#include <utility>

#include "text_primitive.hpp"

namespace ui
{
    TextContent::TextContent()
        : renderer_(std::make_unique<TextPrimitive>())
    {
    }

    TextContent::~TextContent() = default;

    const std::string &TextContent::getText() const noexcept { return layout_.getText(); }

    void TextContent::setText(std::string text)
    {
        layout_.setText(std::move(text));
    }

    TTF_Font *TextContent::getFont() const noexcept { return layout_.getFont(); }

    void TextContent::setFont(TTF_Font *font) noexcept { layout_.setFont(font); }

    float TextContent::getFontSize() const noexcept { return layout_.getFontSize(); }

    void TextContent::setFontSize(float logicalSize) noexcept { layout_.setFontSize(logicalSize); }

    float TextContent::getLineHeight() const noexcept { return layout_.getLineHeight(); }

    void TextContent::setLineHeight(float logicalLineHeight) noexcept { layout_.setLineHeight(logicalLineHeight); }

    WrapMode TextContent::getWrapMode() const noexcept { return layout_.getWrapMode(); }

    void TextContent::setWrapMode(WrapMode mode) noexcept { layout_.setWrapMode(mode); }

    TextAlignment TextContent::getHorizontalAlignment() const noexcept { return horizontalAlignment_; }

    void TextContent::setHorizontalAlignment(TextAlignment alignment) noexcept { horizontalAlignment_ = alignment; }

    TextAlignment TextContent::getVerticalAlignment() const noexcept { return verticalAlignment_; }

    void TextContent::setVerticalAlignment(TextAlignment alignment) noexcept { verticalAlignment_ = alignment; }

    Color TextContent::getColor() const noexcept { return color_; }

    void TextContent::setColor(const Color &color) noexcept { color_ = color; }

    LayoutSize TextContent::measure(float availableWidth) const
    {
        return layout_.measure(availableWidth);
    }

    void TextContent::arrange(const LayoutPosition &contentPosition, const LayoutSize &contentSize)
    {
        arrangedPosition_ = contentPosition;
        arrangedSize_ = contentSize;
        arrangedLayoutResult_ = layout_.measureLayout(contentSize.width);
    }

    void TextContent::draw(SDL_Renderer *renderer)
    {
        if (!renderer_ || layout_.getText().empty() || !layout_.getFont())
            return;

        renderer_->draw(
            renderer,
            layout_.getText(),
            layout_.getFont(),
            horizontalAlignment_,
            verticalAlignment_,
            color_,
            arrangedPosition_,
            arrangedSize_);
    }
}
