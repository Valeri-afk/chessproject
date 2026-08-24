#pragma once

#include "ui_framework/text_node.hpp"

namespace ui
{
    class Heading : public TextNode
    {
    public:
        enum class Level
        {
            H1,
            H2,
            H3,
            H4
        };

        Heading() = default;
        explicit Heading(Level level) noexcept : level_(level) {}
        ~Heading() override = default;

        void setLevel(Level level) noexcept { level_ = level; }
        Level getLevel() const noexcept { return level_; }

    private:
        Level level_ = Level::H1;
    };
}
