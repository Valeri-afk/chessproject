#pragma once

#include <functional>
#include <string>

#include <SDL3_ttf/SDL_ttf.h>

#include "ui_framework/node.hpp"
#include "ui_framework/event_types.hpp"

namespace ui
{
    class MenuItem : public Node
    {
    public:
        using ActivateCallback = std::function<void(MenuItem &)>;
        MenuItem();
        ~MenuItem() override = default;
        void setText(std::string text);
        const std::string &getText() const noexcept;
        void setFont(TTF_Font *font);
        TTF_Font *getFont() const noexcept;
        void setTextColor(Color color) noexcept;
        Color getTextColor() const noexcept;
        void setBackgroundColor(Color color) noexcept;
        Color getBackgroundColor() const noexcept;
        void setHighlighted(bool highlighted) noexcept;
        bool isHighlighted() const noexcept;
        void setSelected(bool selected) noexcept;
        bool isSelected() const noexcept;
        void setOnActivate(ActivateCallback callback);
        void activate();
    protected:
        void update(float dt) override;
        LayoutSize measureContent(const LayoutSize &availableContent) const override;
        void draw(SDL_Renderer *renderer) override;
    private:
        void handleMouseEnter(MouseEnterEvent &event);
        void handleMouseLeave(MouseLeaveEvent &event);
        void handleMouseClick(MouseClickEvent &event);
        std::string text_;
        TTF_Font *font_ = nullptr;
        Color textColor_ = Colors::white;
        Color backgroundColor_ = Colors::transparent;
        bool highlighted_ = false;
        bool selected_ = false;
        ActivateCallback onActivate_;
    };
}