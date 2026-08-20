#pragma once

#include <functional>

#include "ui_framework/core/node.hpp"

namespace ui
{
    class RadioButton : public Node
    {
    public:
        using SelectCallback = std::function<void(RadioButton &, bool)>;

        RadioButton();
        ~RadioButton() override = default;

        void setSelected(bool selected) noexcept;
        bool isSelected() const noexcept;

        void setRadius(float radius) noexcept;
        float getRadius() const noexcept;

        void setOnSelect(SelectCallback callback);
        void select();
        void activate();

    protected:
        LayoutSize measureContent(const LayoutSize &availableContent) const override;
        void draw(SDL_Renderer *renderer) override;

    private:
        bool selected_ = false;
        float radius_ = 10.0f;
        SelectCallback onSelect_;
    };
}
