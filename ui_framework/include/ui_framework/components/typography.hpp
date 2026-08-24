#pragma once

#include "ui_framework/text_node.hpp"

namespace ui
{
    class Typography : public TextNode
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

        Typography() = default;
        explicit Typography(Variant variant) noexcept : variant_(variant) {}
        ~Typography() override = default;

        void setVariant(Variant variant) noexcept { variant_ = variant; }
        Variant getVariant() const noexcept { return variant_; }

    private:
        Variant variant_ = Variant::BODY1;
    };
}
