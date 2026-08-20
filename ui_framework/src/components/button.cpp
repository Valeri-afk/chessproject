#include "ui_framework/components/button.hpp"

#include "ui_framework/core/primitives.hpp"
#include "ui_framework/event_types.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace ui
{
    Button::Button()
    {
        setDefaultGeometry();
        setFocusable(true);
        setCapturable(true);

        addHandler<MouseDownEvent>([this](MouseDownEvent &event, Node &) { handleMouseDown(event); });
        addHandler<MouseUpEvent>([this](MouseUpEvent &event, Node &) { handleMouseUp(event); });
        addHandler<MouseClickEvent>([this](MouseClickEvent &event, Node &) { handleMouseClick(event); });
        addHandler<MouseEnterEvent>([this](MouseEnterEvent &event, Node &) { handleMouseEnter(event); });
        addHandler<MouseLeaveEvent>([this](MouseLeaveEvent &event, Node &) { handleMouseLeave(event); });
    }

    Button::Button(float borderRadius)
        : Button()
    {
        setBorderRadius(borderRadius);
    }

    void Button::setDefaultGeometry()
    {
        setPadding({12.0f, 12.0f, 8.0f, 8.0f});
        setBorder({1.0f, 1.0f, 1.0f, 1.0f});
    }

    void Button::setText(std::string text)
    {
        if (text_.getText() == text)
            return;

        deferLayoutMutation([text = std::move(text)](Node &node)
        {
            static_cast<Button &>(node).text_.setText(text);
        });
    }

    const std::string &Button::getText() const noexcept { return text_.getText(); }

    void Button::setFont(TTF_Font *font)
    {
        if (text_.getFont() == font)
            return;

        deferLayoutMutation([font](Node &node)
        {
            static_cast<Button &>(node).text_.setFont(font);
        });
    }

    TTF_Font *Button::getFont() const noexcept { return text_.getFont(); }

    void Button::setTextColor(Color color) noexcept { text_.setColor(color); }
    Color Button::getTextColor() const noexcept { return text_.getColor(); }

    void Button::setVariant(Variant variant) noexcept { variant_ = variant; }
    Button::Variant Button::getVariant() const noexcept { return variant_; }

    void Button::setBackgroundColor(Color color) noexcept { backgroundColor_ = color; }
    Color Button::getBackgroundColor() const noexcept { return backgroundColor_; }

    void Button::setBorderColor(Color color) noexcept { borderColor_ = color; }
    Color Button::getBorderColor() const noexcept { return borderColor_; }

    void Button::setBorderRadius(float radius) noexcept { borderRadius_ = std::max(0.0f, radius); }
    float Button::getBorderRadius() const noexcept { return borderRadius_; }

    void Button::setPressScale(float scale) noexcept { pressScale_ = std::clamp(scale, 0.0f, 1.0f); }
    float Button::getPressScale() const noexcept { return pressScale_; }

    void Button::setPressAnimationEnabled(bool enabled) noexcept
    {
        pressAnimationEnabled_ = enabled;
        if (!enabled)
        {
            currentScale_ = 1.0f;
            targetScale_ = 1.0f;
        }
    }

    bool Button::isPressAnimationEnabled() const noexcept { return pressAnimationEnabled_; }

    void Button::setPressAnimationSpeed(float speed) noexcept { pressAnimationSpeed_ = std::max(0.0f, speed); }
    float Button::getPressAnimationSpeed() const noexcept { return pressAnimationSpeed_; }

    bool Button::isPressed() const noexcept { return pressed_; }
    bool Button::isHovered() const noexcept { return hovered_; }

    void Button::setOnActivate(ActivateCallback callback) { onActivate_ = std::move(callback); }

    void Button::activate()
    {
        if (!isVisible() || !isEnabled())
            return;

        onActivate();
    }

    void Button::onActivate()
    {
        if (onActivate_)
            onActivate_(*this);
    }

    Color Button::presentationBackgroundColor() const noexcept
    {
        Color background = backgroundColor_;

        if (hovered_ && !pressed_)
            background = lighten(background, 0.08f);

        if (!isEnabled())
            background = multiplyAlpha(background, 0.5f);

        return background;
    }

    Color Button::presentationBorderColor() const noexcept
    {
        Color border = borderColor_;
        if (!isEnabled())
            border = multiplyAlpha(border, 0.5f);
        return border;
    }

    Color Button::presentationTextColor() const noexcept
    {
        Color textColor = text_.getColor();
        if (!isEnabled())
            textColor = multiplyAlpha(textColor, 0.5f);
        return textColor;
    }

    void Button::update(float dt)
    {
        if (!isEnabled())
            pressed_ = false;

        targetScale_ = pressAnimationEnabled_ && pressed_ ? pressScale_ : 1.0f;

        if (!pressAnimationEnabled_ || pressAnimationSpeed_ <= 0.0f)
        {
            currentScale_ = targetScale_;
            return;
        }

        const float t = std::clamp(dt * pressAnimationSpeed_, 0.0f, 1.0f);
        currentScale_ += (targetScale_ - currentScale_) * t;

        if (std::fabs(currentScale_ - targetScale_) < 0.0001f)
            currentScale_ = targetScale_;
    }

    LayoutSize Button::measureContent(const LayoutSize &availableContent) const
    {
        return text_.measure(availableContent.width);
    }

    void Button::draw(SDL_Renderer *renderer)
    {
        if (!renderer)
            return;

        const LayoutPosition position = getActualPosition();
        const LayoutSize size = getActualSize();
        const float scale = std::max(0.0f, currentScale_);
        const float scaledWidth = size.width * scale;
        const float scaledHeight = size.height * scale;
        const float x = position.x + (size.width - scaledWidth) * 0.5f;
        const float y = position.y + (size.height - scaledHeight) * 0.5f;
        const float right = x + scaledWidth;
        const float bottom = y + scaledHeight;

        const Color background = presentationBackgroundColor();
        const Color border = presentationBorderColor();
        const Color textColor = presentationTextColor();

        if (variant_ == Variant::FILLED)
        {
            primitives::roundedBoxRGBA(renderer, x, y, right, bottom, borderRadius_,
                                        background.r, background.g, background.b, background.a);
        }
        else if (variant_ == Variant::OUTLINED)
        {
            primitives::roundedRectangleRGBA(renderer, x, y, right, bottom, borderRadius_,
                                              border.r, border.g, border.b, border.a);
        }

        text_.setColor(textColor);
        text_.setHorizontalAlignment(TextAlignment::CENTER);
        text_.setVerticalAlignment(TextAlignment::CENTER);

        const Padding padding = getPadding();
        const Border nodeBorder = getBorder();
        const float insetX = (nodeBorder.left + padding.left) * scale;
        const float insetY = (nodeBorder.top + padding.top) * scale;
        const float insetRight = (nodeBorder.right + padding.right) * scale;
        const float insetBottom = (nodeBorder.bottom + padding.bottom) * scale;

        text_.draw(
            renderer,
            {x + insetX, y + insetY},
            {std::max(0.0f, scaledWidth - insetX - insetRight),
             std::max(0.0f, scaledHeight - insetY - insetBottom)});
    }

    void Button::handleMouseDown(MouseDownEvent &event)
    {
        if (event.button == MouseButton::Left && isEnabled())
            pressed_ = true;
    }

    void Button::handleMouseUp(MouseUpEvent &event)
    {
        if (event.button == MouseButton::Left)
            pressed_ = false;
    }

    void Button::handleMouseClick(MouseClickEvent &event)
    {
        if (event.button == MouseButton::Left)
            activate();
    }

    void Button::handleMouseEnter(MouseEnterEvent &) { hovered_ = true; }
    void Button::handleMouseLeave(MouseLeaveEvent &) { hovered_ = false; }

    Color Button::multiplyAlpha(Color color, float factor) noexcept
    {
        color.a = static_cast<uint8_t>(std::clamp(static_cast<float>(color.a) * factor, 0.0f, 255.0f));
        return color;
    }

    Color Button::lighten(Color color, float amount) noexcept
    {
        const float factor = std::clamp(amount, 0.0f, 1.0f);
        color.r = static_cast<uint8_t>(color.r + (255 - color.r) * factor);
        color.g = static_cast<uint8_t>(color.g + (255 - color.g) * factor);
        color.b = static_cast<uint8_t>(color.b + (255 - color.b) * factor);
        return color;
    }
}
