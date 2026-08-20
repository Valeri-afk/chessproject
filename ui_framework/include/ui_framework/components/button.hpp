#pragma once

#include <functional>
#include <string>

#include "ui_framework/core/node.hpp"
#include "ui_framework/core/text_primitive.hpp"
#include "ui_framework/event_types.hpp"

namespace ui
{
    class Button : public Node
    {
    public:
        enum class Variant
        {
            FILLED,
            OUTLINED,
            TEXT
        };

        using ActivateCallback = std::function<void(Button &)>;

        Button();
        explicit Button(float borderRadius);
        ~Button() override = default;

        void setText(std::string text);
        const std::string &getText() const noexcept;

        void setFont(TTF_Font *font);
        TTF_Font *getFont() const noexcept;

        void setTextColor(Color color) noexcept;
        Color getTextColor() const noexcept;

        void setVariant(Variant variant) noexcept;
        Variant getVariant() const noexcept;

        void setBackgroundColor(Color color) noexcept;
        Color getBackgroundColor() const noexcept;

        void setBorderColor(Color color) noexcept;
        Color getBorderColor() const noexcept;

        void setBorderRadius(float radius) noexcept;
        float getBorderRadius() const noexcept;

        void setPressScale(float scale) noexcept;
        float getPressScale() const noexcept;

        void setPressAnimationEnabled(bool enabled) noexcept;
        bool isPressAnimationEnabled() const noexcept;

        void setPressAnimationSpeed(float speed) noexcept;
        float getPressAnimationSpeed() const noexcept;

        bool isPressed() const noexcept;
        bool isHovered() const noexcept;

        void setOnActivate(ActivateCallback callback);
        virtual void activate();

    protected:
        virtual void onActivate();
        virtual Color presentationBackgroundColor() const noexcept;
        virtual Color presentationBorderColor() const noexcept;
        virtual Color presentationTextColor() const noexcept;

        void update(float dt) override;
        LayoutSize measureContent(const LayoutSize &availableContent) const override;
        void draw(SDL_Renderer *renderer) override;

    private:
        void setDefaultGeometry();
        void handleMouseDown(MouseDownEvent &event);
        void handleMouseUp(MouseUpEvent &event);
        void handleMouseClick(MouseClickEvent &event);
        void handleMouseEnter(MouseEnterEvent &event);
        void handleMouseLeave(MouseLeaveEvent &event);

        static Color multiplyAlpha(Color color, float factor) noexcept;
        static Color lighten(Color color, float amount) noexcept;

        TextPrimitive text_;

        Variant variant_ = Variant::FILLED;
        Color backgroundColor_ = Colors::gray;
        Color borderColor_ = Colors::black;
        float borderRadius_ = 4.0f;

        float pressScale_ = 0.96f;
        bool pressAnimationEnabled_ = true;
        float pressAnimationSpeed_ = 14.0f;
        float currentScale_ = 1.0f;
        float targetScale_ = 1.0f;

        bool pressed_ = false;
        bool hovered_ = false;

        ActivateCallback onActivate_;
    };
}
