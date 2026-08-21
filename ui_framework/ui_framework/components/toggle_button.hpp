#pragma once

#include <functional>

#include "ui_framework/components/button.hpp"

namespace ui
{
    class ToggleButton : public Button
    {
    public:
        ToggleButton();
        ~ToggleButton() override = default;

        void setSelected(bool selected) noexcept;
        bool isSelected() const noexcept;

        void setOnToggle(std::function<void(ToggleButton &, bool)> callback);

        void toggle();
        void activate() override;

    protected:
        Color presentationBackgroundColor() const noexcept override;
        Color presentationBorderColor() const noexcept override;
        Color presentationTextColor() const noexcept override;

    private:
        bool selected_ = false;
        std::function<void(ToggleButton &, bool)> onToggle_;
    };
}
