#pragma once

#include <memory>
#include <string>

#include <SDL3_ttf/SDL_ttf.h>

#include "ui_framework/node.hpp"
#include "ui_framework/text_layout.hpp"

namespace ui
{
    class TextPrimitive;

    class Typography : public Node
    {
    public:
        enum class Variant
        {
            INHERIT,
            H1,
            H2,
            H3,
            H4,
            H5,
            H6,
            SUBTITLE1,
            SUBTITLE2,
            BODY1,
            BODY2,
            BUTTON,
            CAPTION,
            OVERLINE
        };

        Typography();
        ~Typography() override;
        Typography(const Typography &) = delete;
        Typography &operator=(const Typography &) = delete;

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
        void setVariant(Variant variant) noexcept;
        Variant getVariant() const noexcept;

    protected:
        LayoutSize measureContent(const LayoutSize &availableContent) const override;
        void draw(SDL_Renderer *renderer) override;

    private:
        TextLayout textLayout_;
        std::unique_ptr<TextPrimitive> textPrimitive_;
        TextAlignment horizontalAlignment_ = TextAlignment::START;
        TextAlignment verticalAlignment_ = TextAlignment::START;
        Color color_ = Colors::white;
        Variant variant_ = Variant::BODY1;
    };
}
