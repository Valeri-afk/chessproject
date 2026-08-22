#pragma once

#include <string>

#include <SDL3_ttf/SDL_ttf.h>

#include "node.hpp"

namespace ui
{
    class TextNode : public Node
    {
    public:
        TextNode() = default;
        ~TextNode() override = default;
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
        std::string text_;
        TTF_Font *font_ = nullptr;
        TextAlignment horizontalAlignment_ = TextAlignment::START;
        TextAlignment verticalAlignment_ = TextAlignment::START;
        Color color_ = Colors::white;
    };
}