#pragma once

#include <memory>
#include <string>

#include <SDL3_ttf/SDL_ttf.h>

#include "node.hpp"
#include "text_layout.hpp"

namespace ui
{
    class TextPrimitive;

    class TextNode : public Node
    {
    public:
        TextNode();
        ~TextNode() override;
        TextNode(const TextNode &) = delete;
        TextNode &operator=(const TextNode &) = delete;

        const std::string &getText() const noexcept;
        void setText(std::string text);
        TTF_Font *getFont() const noexcept;
        void setFont(TTF_Font *font);
        TextAlignment getHorizontalAlignment() const noexcept;
        void setHorizontalAlignment(TextAlignment alignment);
        TextAlignment getVerticalAlignment() const noexcept;
        void setVerticalAlignment(TextAlignment alignment);
        Color getColor() const noexcept;
        void setColor(const Color &color);
    protected:
        LayoutSize measureContent(const LayoutSize &availableContent) const override;
        void draw(SDL_Renderer *renderer) override;
    private:
        TextLayout textLayout_;
        std::unique_ptr<TextPrimitive> textPrimitive_;
        TextAlignment horizontalAlignment_ = TextAlignment::START;
        TextAlignment verticalAlignment_ = TextAlignment::START;
        Color color_ = Colors::white;
    };
}