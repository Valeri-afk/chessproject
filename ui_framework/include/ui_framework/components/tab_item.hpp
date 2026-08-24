#pragma once

#include <functional>
#include <memory>
#include <string>

#include <SDL3_ttf/SDL_ttf.h>

#include "ui_framework/node.hpp"
#include "ui_framework/event_types.hpp"
#include "ui_framework/text_layout.hpp"

namespace ui
{
    class TextPrimitive;

    class TabItem : public Node
    {
    public:
        using ActivateCallback = std::function<void(TabItem &)>;
        TabItem();
        ~TabItem() override;
        TabItem(const TabItem &) = delete;
        TabItem &operator=(const TabItem &) = delete;
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
        TextLayout textLayout_;
        std::unique_ptr<TextPrimitive> textPrimitive_;
        Color textColor_ = Colors::white;
        Color backgroundColor_ = Colors::transparent;
        bool active_ = false;
        ActivateCallback onActivate_;
    };
}