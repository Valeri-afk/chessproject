#pragma once

#include <functional>
#include <string>

#include "ui_framework/core/node.hpp"
#include "ui_framework/core/text_primitive.hpp"
#include "ui_framework/event_types.hpp"

namespace ui
{
    class TabItem : public Node
    {
    public:
        using ActivateCallback = std::function<void(TabItem &)>;

        TabItem();
        ~TabItem() override = default;

        void setText(std::string text);
        const std::string &getText() const noexcept;

        void setFont(TTF_Font *font);
        TTF_Font *getFont() const noexcept;

        void setTextColor(Color color) noexcept;
        Color getTextColor() const noexcept;

        void setBackgroundColor(Color color) noexcept;
        Color getBackgroundColor() const noexcept;

        void setActive(bool active) noexcept;
        bool isActive() const noexcept;

        void setOnActivate(ActivateCallback callback);
        void activate();

    protected:
        LayoutSize measureContent(const LayoutSize &availableContent) const override;
        void draw(SDL_Renderer *renderer) override;

    private:
        void handleMouseClick(MouseClickEvent &event);

        TextPrimitive text_;
        Color textColor_ = Colors::white;
        Color backgroundColor_ = Colors::transparent;
        bool active_ = false;
        ActivateCallback onActivate_;
    };
}
