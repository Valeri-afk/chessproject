#include "ui_framework/components/radio_button.hpp"

#include "ui_framework/core/primitives.hpp"
#include "ui_framework/event_types.hpp"

#include <algorithm>
#include <utility>

namespace ui
{
    RadioButton::RadioButton()
    {
        setFocusable(true);
        setCapturable(true);
        setSize({radius_ * 2.0f, radius_ * 2.0f});
        addHandler<MouseClickEvent>([this](MouseClickEvent &event, Node &)
        {
            if (event.button == MouseButton::Left)
                activate();
        });
    }

    void RadioButton::setSelected(bool selected) noexcept { selected_ = selected; }
    bool RadioButton::isSelected() const noexcept { return selected_; }

    void RadioButton::setRadius(float radius) noexcept
    {
        radius_ = std::max(1.0f, radius);
        setSize({radius_ * 2.0f, radius_ * 2.0f});
    }

    float RadioButton::getRadius() const noexcept { return radius_; }

    void RadioButton::setOnSelect(SelectCallback callback) { onSelect_ = std::move(callback); }

    void RadioButton::select()
    {
        if (!isVisible() || !isEnabled() || selected_)
            return;
        selected_ = true;
        if (onSelect_)
            onSelect_(*this, true);
    }

    void RadioButton::activate() { select(); }

    LayoutSize RadioButton::measureContent(const LayoutSize &) const
    {
        return {radius_ * 2.0f, radius_ * 2.0f};
    }

    void RadioButton::draw(SDL_Renderer *renderer)
    {
        if (!renderer)
            return;

        const auto p = getActualPosition();
        const auto s = getActualSize();
        const float radius = std::min(s.width, s.height) * 0.5f;
        const float cx = p.x + s.width * 0.5f;
        const float cy = p.y + s.height * 0.5f;

        primitives::arcRGBA(renderer, cx, cy, radius - 1.0f,
                             0, 359, 220, 220, 220, 255);

        if (selected_)
        {
            const float dotRadius = radius * 0.5f;
            primitives::filledCircleRGBA(renderer, cx, cy, dotRadius,
                                         255, 255, 255, 255);
        }
    }
}
