#include "ui_framework/components/typography.hpp"

#include <algorithm>
#include <utility>

#include "text_primitive.hpp"

namespace ui
{
    namespace
    {
        float defaultFontSize(TypographyVariant variant) noexcept
        {
            switch (variant)
            {
            case TypographyVariant::H1: return 32.0f;
            case TypographyVariant::H2: return 28.0f;
            case TypographyVariant::H3: return 24.0f;
            case TypographyVariant::H4: return 20.0f;
            case TypographyVariant::H5: return 18.0f;
            case TypographyVariant::H6: return 16.0f;
            case TypographyVariant::SUBTITLE1: return 16.0f;
            case TypographyVariant::SUBTITLE2: return 14.0f;
            case TypographyVariant::BODY1: return 16.0f;
            case TypographyVariant::BODY2: return 14.0f;
            case TypographyVariant::BUTTON: return 14.0f;
            case TypographyVariant::CAPTION: return 12.0f;
            case TypographyVariant::OVERLINE: return 12.0f;
            case TypographyVariant::INHERIT: default: return 16.0f;
            }
        }
    }

    Typography::Typography() : textPrimitive_(std::make_unique<TextPrimitive>())
    {
        applyVariantDefaults();
    }

    Typography::~Typography() = default;

    const std::string &Typography::getText() const noexcept { return textLayout_.getText(); }

    void Typography::setText(std::string text)
    {
        if (textLayout_.getText() == text)
            return;
        textLayout_.setText(std::move(text));
    }

    TTF_Font *Typography::getFont() const noexcept { return textLayout_.getFont(); }

    void Typography::setFont(TTF_Font *font)
    {
        if (textLayout_.getFont() == font)
            return;
        textLayout_.setFont(font);
    }

    void Typography::setVariant(Variant variant) noexcept
    {
        if (variant_ == variant)
            return;
        variant_ = variant;
        if (!fontSizeExplicit_ || !lineHeightExplicit_)
            applyVariantDefaults();
    }

    Typography::Variant Typography::getVariant() const noexcept { return variant_; }

    void Typography::setFontSize(float logicalSize) noexcept
    {
        explicitFontSize_ = std::max(0.0f, logicalSize);
        fontSizeExplicit_ = true;
        textLayout_.setFontSize(explicitFontSize_);
    }

    float Typography::getFontSize() const noexcept { return textLayout_.getFontSize(); }

    void Typography::setLineHeight(float logicalLineHeight) noexcept
    {
        explicitLineHeight_ = std::max(0.0f, logicalLineHeight);
        lineHeightExplicit_ = true;
        textLayout_.setLineHeight(explicitLineHeight_);
    }

    float Typography::getLineHeight() const noexcept { return textLayout_.getLineHeight(); }

    void Typography::setWrapMode(WrapMode mode) noexcept
    {
        textLayout_.setWrapMode(mode);
    }

    WrapMode Typography::getWrapMode() const noexcept { return textLayout_.getWrapMode(); }

    TextAlignment Typography::getHorizontalAlignment() const noexcept { return horizontalAlignment_; }

    void Typography::setHorizontalAlignment(TextAlignment alignment)
    {
        if (horizontalAlignment_ == alignment)
            return;
        horizontalAlignment_ = alignment;
    }

    TextAlignment Typography::getVerticalAlignment() const noexcept { return verticalAlignment_; }

    void Typography::setVerticalAlignment(TextAlignment alignment)
    {
        if (verticalAlignment_ == alignment)
            return;
        verticalAlignment_ = alignment;
    }

    Color Typography::getColor() const noexcept { return color_; }

    void Typography::setColor(const Color &color) { color_ = color; }

    LayoutSize Typography::measureContent(const LayoutSize &availableContent) const
    {
        layoutResult_ = textLayout_.measureLayout(availableContent.width);
        return layoutResult_.desiredSize;
    }

    void Typography::arrangeContent(const LayoutPosition &contentPosition, const LayoutSize &contentSize)
    {
        arrangedContentPosition_ = contentPosition;
        arrangedContentSize_ = contentSize;
        layoutResult_ = textLayout_.measureLayout(contentSize.width);
    }

    void Typography::draw(SDL_Renderer *renderer)
    {
        if (!renderer || !textPrimitive_ || textLayout_.getText().empty())
            return;

        textPrimitive_->draw(
            renderer,
            textLayout_.getText(),
            textLayout_.getFont(),
            horizontalAlignment_,
            verticalAlignment_,
            color_,
            arrangedContentPosition_,
            arrangedContentSize_);
    }

    void Typography::applyVariantDefaults() noexcept
    {
        if (!fontSizeExplicit_)
        {
            textLayout_.setFontSize(defaultFontSize(variant_));
        }
        else
        {
            textLayout_.setFontSize(explicitFontSize_);
        }

        if (!lineHeightExplicit_)
        {
            textLayout_.setLineHeight(0.0f);
        }
        else
        {
            textLayout_.setLineHeight(explicitLineHeight_);
        }
    }
}
