#pragma once

#include <functional>

#include "ui_framework/core/node.hpp"

namespace ui
{
    class Checkbox : public Node
    {
    public:
        using ToggleCallback = std::function<void(Checkbox &, bool)>;

        Checkbox();
        ~Checkbox() override = default;

        void setChecked(bool checked) noexcept;
        bool isChecked() const noexcept;

        void setBoxSize(float size) noexcept;
        float getBoxSize() const noexcept;

        void setOnToggle(ToggleCallback callback);
        void toggle();
        void activate();

    protected:
        LayoutSize measureContent(const LayoutSize &availableContent) const override;
        void draw(SDL_Renderer *renderer) override;

    private:
        bool checked_ = false;
        float boxSize_ = 20.0f;
        ToggleCallback onToggle_;
    };
}
