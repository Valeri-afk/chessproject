#include "ui_framework/components/tab_item.hpp"

#include "ui_framework/core/primitives.hpp"

#include <algorithm>
#include <utility>

namespace ui
{
    TabItem::TabItem()
    {
        setPadding({12.0f, 12.0f, 8.0f, 8.0f});
        setFocusable(true);
        setCapturable(true);

        addHandler<MouseClickEvent>(
            [this](MouseClickEvent &event, Node &)
            {
                handleMouseClick(event);
            });
    }

    void TabItem::setText(std::string text)
    {
        if (text_.getText() == text)
            return;

        deferLayoutMutation(
            [text = std::move(text)](Node &node)
            {
                static_cast<TabItem &>(node).text_.setText(text);
            });
    }

    const std::string &TabItem::getText() const noexcept
    {
        return text_.getText();
    }

    void TabItem::setFont(TTF_Font *font)
    {
        if (text_.getFont() == font)
            return;

        deferLayoutMutation(
            [font](Node &node)
            {
                static_cast<TabItem &>(node).text_.setFont(font);
            });
    }

    TTF_Font *TabItem::getFont() const noexcept
    {
        return text_.getFont();
    }

    void TabItem::setTextColor(Color color) noexcept
    {
        textColor_ = color;
        text_.setColor(color);
    }

    Color TabItem::getTextColor() const noexcept
    {
        return textColor_;
    }

    void TabItem::setBackgroundColor(Color color) noexcept
    {
        backgroundColor_ = color;
    }

    Color TabItem::getBackgroundColor() const noexcept
    {
        return backgroundColor_;
    }

    void TabItem::setActive(bool active) noexcept
    {
        active_ = active;
    }

    bool TabItem::isActive() const noexcept
    {
        return active_;
    }

    void TabItem::setOnActivate(ActivateCallback callback)
    {
        onActivate_ = std::move(callback);
    }

    void TabItem::activate()
    {
        if (!isVisible() || !isEnabled())
            return;

        if (onActivate_)
            onActivate_(*this);
    }

    LayoutSize TabItem::measureContent(const LayoutSize &availableContent) const
    {
        return text_.measure(availableContent.width);
    }

    void TabItem::draw(SDL_Renderer *renderer)
    {
        if (!renderer)
            return;

        const LayoutPosition position = getActualPosition();
        const LayoutSize size = getActualSize();
        Color background = backgroundColor_;

        if (active_)
        {
            background.r = static_cast<uint8_t>(background.r + (255 - background.r) * 0.16f);
            background.g = static_cast<uint8_t>(background.g + (255 - background.g) * 0.16f);
            background.b = static_cast<uint8_t>(background.b + (255 - background.b) * 0.16f);
        }

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

        const Padding padding = getPadding();
        const Border border = getBorder();
        text_.setColor(textColor_);
        text_.setHorizontalAlignment(TextAlignment::CENTER);
        text_.setVerticalAlignment(TextAlignment::CENTER);
        text_.draw(
            renderer,
            {position.x + border.left + padding.left,
             position.y + border.top + padding.top},
            {std::max(0.0f, size.width - border.left - border.right - padding.left - padding.right),
             std::max(0.0f, size.height - border.top - border.bottom - padding.top - padding.bottom)});
    }

    void TabItem::handleMouseClick(MouseClickEvent &event)
    {
        if (event.button == MouseButton::Left)
            activate();
    }
}
