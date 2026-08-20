#include "ui_framework/components/menu_item.hpp"

#include "ui_framework/core/primitives.hpp"
#include "ui_framework/event_types.hpp"

#include <algorithm>
#include <utility>

namespace ui
{
    MenuItem::MenuItem()
    {
        setPadding({12.0f, 12.0f, 8.0f, 8.0f});
        setFocusable(true);
        setCapturable(true);

        addHandler<MouseEnterEvent>(
            [this](MouseEnterEvent &event, Node &)
            {
                handleMouseEnter(event);
            });

        addHandler<MouseLeaveEvent>(
            [this](MouseLeaveEvent &event, Node &)
            {
                handleMouseLeave(event);
            });

        addHandler<MouseClickEvent>(
            [this](MouseClickEvent &event, Node &)
            {
                handleMouseClick(event);
            });
    }

    void MenuItem::setText(std::string text)
    {
        if (text_.getText() == text)
            return;

        deferLayoutMutation(
            [text = std::move(text)](Node &node)
            {
                static_cast<MenuItem &>(node).text_.setText(text);
            });
    }

    const std::string &MenuItem::getText() const noexcept
    {
        return text_.getText();
    }

    void MenuItem::setFont(TTF_Font *font)
    {
        if (text_.getFont() == font)
            return;

        deferLayoutMutation(
            [font](Node &node)
            {
                static_cast<MenuItem &>(node).text_.setFont(font);
            });
    }

    TTF_Font *MenuItem::getFont() const noexcept
    {
        return text_.getFont();
    }

    void MenuItem::setTextColor(Color color) noexcept
    {
        text_.setColor(color);
    }

    Color MenuItem::getTextColor() const noexcept
    {
        return text_.getColor();
    }

    void MenuItem::setBackgroundColor(Color color) noexcept
    {
        backgroundColor_ = color;
    }

    Color MenuItem::getBackgroundColor() const noexcept
    {
        return backgroundColor_;
    }

    void MenuItem::setHighlighted(bool highlighted) noexcept
    {
        highlighted_ = highlighted;
    }

    bool MenuItem::isHighlighted() const noexcept
    {
        return highlighted_;
    }

    void MenuItem::setSelected(bool selected) noexcept
    {
        selected_ = selected;
    }

    bool MenuItem::isSelected() const noexcept
    {
        return selected_;
    }

    void MenuItem::setOnActivate(ActivateCallback callback)
    {
        onActivate_ = std::move(callback);
    }

    void MenuItem::activate()
    {
        if (!isVisible() || !isEnabled())
            return;

        if (onActivate_)
            onActivate_(*this);
    }

    void MenuItem::update(float)
    {
        if (!isEnabled())
            highlighted_ = false;
    }

    LayoutSize MenuItem::measureContent(const LayoutSize &availableContent) const
    {
        return text_.measure(availableContent.width);
    }

    void MenuItem::draw(SDL_Renderer *renderer)
    {
        if (!renderer)
            return;

        const LayoutPosition position = getActualPosition();
        const LayoutSize size = getActualSize();

        Color background = backgroundColor_;

        if (highlighted_)
            background = lighten(background, 0.12f);

        if (selected_)
            background = lighten(background, 0.18f);

        if (!isEnabled())
            background = multiplyAlpha(background, 0.5f);

        if (background.a > 0)
        {
            primitives::boxRGBA(
                renderer,
                position.x,
                position.y,
                position.x + size.width,
                position.y + size.height,
                background.r,
                background.g,
                background.b,
                background.a);
        }

        text_.setHorizontalAlignment(TextAlignment::START);
        text_.setVerticalAlignment(TextAlignment::CENTER);

        const Padding padding = getPadding();
        const Border border = getBorder();
        text_.draw(
            renderer,
            {position.x + border.left + padding.left,
             position.y + border.top + padding.top},
            {std::max(0.0f, size.width - border.left - border.right - padding.left - padding.right),
             std::max(0.0f, size.height - border.top - border.bottom - padding.top - padding.bottom)});
    }

    void MenuItem::handleMouseEnter(MouseEnterEvent &)
    {
        if (isEnabled())
            highlighted_ = true;
    }

    void MenuItem::handleMouseLeave(MouseLeaveEvent &)
    {
        highlighted_ = false;
    }

    void MenuItem::handleMouseClick(MouseClickEvent &event)
    {
        if (event.button == MouseButton::Left)
            activate();
    }

    Color MenuItem::multiplyAlpha(Color color, float factor) noexcept
    {
        color.a = static_cast<uint8_t>(
            std::clamp(static_cast<float>(color.a) * factor, 0.0f, 255.0f));
        return color;
    }

    Color MenuItem::lighten(Color color, float amount) noexcept
    {
        const float factor = std::clamp(amount, 0.0f, 1.0f);
        color.r = static_cast<uint8_t>(color.r + (255 - color.r) * factor);
        color.g = static_cast<uint8_t>(color.g + (255 - color.g) * factor);
        color.b = static_cast<uint8_t>(color.b + (255 - color.b) * factor);
        return color;
    }
}
