#include "ui_framework/components/menu_item.hpp"
#include "ui_framework/primitives.hpp"
#include "ui_framework/event_types.hpp"
#include <algorithm>
#include <utility>

namespace ui
{
    namespace
    {
        Color multiplyAlpha(Color c, float f) noexcept
        {
            c.a = static_cast<uint8_t>(std::clamp(static_cast<float>(c.a) * f, 0.0f, 255.0f));
            return c;
        }

        Color lighten(Color c, float amount) noexcept
        {
            amount = std::clamp(amount, 0.0f, 1.0f);
            c.r = static_cast<uint8_t>(c.r + (255 - c.r) * amount);
            c.g = static_cast<uint8_t>(c.g + (255 - c.g) * amount);
            c.b = static_cast<uint8_t>(c.b + (255 - c.b) * amount);
            return c;
        }
    }

    MenuItem::MenuItem()
    {
        setPadding({12.0f, 12.0f, 8.0f, 8.0f});
        setFocusable(true);
        setCapturable(true);
        addHandler<MouseEnterEvent>([this](MouseEnterEvent &e, Node &) { handleMouseEnter(e); });
        addHandler<MouseLeaveEvent>([this](MouseLeaveEvent &e, Node &) { handleMouseLeave(e); });
        addHandler<MouseClickEvent>([this](MouseClickEvent &e, Node &) { handleMouseClick(e); });
    }

    void MenuItem::setText(std::string text)
    {
        if (text_.getText() == text)
            return;
        deferLayoutMutation([text = std::move(text)](Node &n) {
            static_cast<MenuItem &>(n).text_.setText(text);
        });
    }

    const std::string &MenuItem::getText() const noexcept { return text_.getText(); }

    void MenuItem::setFont(TTF_Font *font)
    {
        if (text_.getFont() == font)
            return;
        deferLayoutMutation([font](Node &n) {
            static_cast<MenuItem &>(n).text_.setFont(font);
        });
    }

    TTF_Font *MenuItem::getFont() const noexcept { return text_.getFont(); }
    void MenuItem::setTextColor(Color c) noexcept { text_.setColor(c); }
    Color MenuItem::getTextColor() const noexcept { return text_.getColor(); }
    void MenuItem::setBackgroundColor(Color c) noexcept { backgroundColor_ = c; }
    Color MenuItem::getBackgroundColor() const noexcept { return backgroundColor_; }
    void MenuItem::setHighlighted(bool v) noexcept { highlighted_ = v; }
    bool MenuItem::isHighlighted() const noexcept { return highlighted_; }
    void MenuItem::setSelected(bool v) noexcept { selected_ = v; }
    bool MenuItem::isSelected() const noexcept { return selected_; }
    void MenuItem::setOnActivate(ActivateCallback cb) { onActivate_ = std::move(cb); }

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

    LayoutSize MenuItem::measureContent(const LayoutSize &a) const { return text_.measure(a.width); }

    void MenuItem::draw(SDL_Renderer *r)
    {
        if (!r)
            return;

        const auto p = getActualPosition();
        const auto s = getActualSize();

        Color bg = backgroundColor_;
        if (highlighted_)
            bg = lighten(bg, 0.12f);
        if (selected_)
            bg = lighten(bg, 0.18f);
        if (!isEnabled())
            bg = multiplyAlpha(bg, 0.5f);

        if (bg.a)
            primitives::boxRGBA(r, p.x, p.y, p.x + s.width, p.y + s.height,
                                bg.r, bg.g, bg.b, bg.a);

        text_.setHorizontalAlignment(TextAlignment::START);
        text_.setVerticalAlignment(TextAlignment::CENTER);

        const auto pad = getPadding();
        const auto border = getBorder();
        text_.draw(r,
                   {p.x + border.left + pad.left, p.y + border.top + pad.top},
                   {std::max(0.0f, s.width - border.left - border.right - pad.left - pad.right),
                    std::max(0.0f, s.height - border.top - border.bottom - pad.top - pad.bottom)});
    }

    void MenuItem::handleMouseEnter(MouseEnterEvent &)
    {
        if (isEnabled())
            highlighted_ = true;
    }

    void MenuItem::handleMouseLeave(MouseLeaveEvent &) { highlighted_ = false; }

    void MenuItem::handleMouseClick(MouseClickEvent &e)
    {
        if (e.button == MouseButton::Left)
            activate();
    }
}
