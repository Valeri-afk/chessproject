#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "ui_framework/types.hpp"

namespace ui
{
    struct RenderFillRectCommand { float x=0, y=0, width=0, height=0; Color color{}; };
    struct RenderRectCommand { float x=0, y=0, width=0, height=0; Color color{}; };
    struct RenderRoundedFillRectCommand { float x=0, y=0, width=0, height=0, radius=0; Color color{}; };
    struct RenderRoundedRectCommand { float x=0, y=0, width=0, height=0, radius=0; Color color{}; };
    struct RenderLineCommand { float x1=0, y1=0, x2=0, y2=0; Color color{}; };
    struct RenderPointCommand { float x=0, y=0; Color color{}; };
    struct RenderArcCommand { float x=0, y=0, radius=0; Sint16 start=0, end=0; Color color{}; };
    struct RenderFilledCircleCommand { float x=0, y=0, radius=0; Color color{}; };

    struct RenderTextCommand
    {
        std::string text;
        TTF_Font *font = nullptr;
        LayoutPosition position{};
        LayoutSize bounds{};
        TextAlignment horizontalAlignment = TextAlignment::START;
        TextAlignment verticalAlignment = TextAlignment::START;
        Color color{};
    };

    struct RenderPushClipCommand
    {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
    };
    struct RenderPopClipCommand {};

    using RenderCommand = std::variant<
        RenderFillRectCommand,
        RenderRectCommand,
        RenderRoundedFillRectCommand,
        RenderRoundedRectCommand,
        RenderLineCommand,
        RenderPointCommand,
        RenderArcCommand,
        RenderFilledCircleCommand,
        RenderTextCommand,
        RenderPushClipCommand,
        RenderPopClipCommand>;

    class RenderCommandList
    {
    public:
        void clear() noexcept { commands_.clear(); }
        bool empty() const noexcept { return commands_.empty(); }
        std::size_t size() const noexcept { return commands_.size(); }
        const RenderCommand &operator[](std::size_t index) const noexcept { return commands_[index]; }
        RenderCommand &operator[](std::size_t index) noexcept { return commands_[index]; }
        const std::vector<RenderCommand> &getCommands() const noexcept { return commands_; }
        std::vector<RenderCommand> &getCommands() noexcept { return commands_; }

        template <typename Command>
        void push(Command command) { commands_.emplace_back(std::move(command)); }

    private:
        std::vector<RenderCommand> commands_;
    };

    class RenderCommandRecorder final
    {
    public:
        explicit RenderCommandRecorder(RenderCommandList &list) noexcept : previous_(current_()) { current_() = &list; }
        ~RenderCommandRecorder() { current_() = previous_; }
        RenderCommandRecorder(const RenderCommandRecorder &) = delete;
        RenderCommandRecorder &operator=(const RenderCommandRecorder &) = delete;
        static RenderCommandList *current() noexcept { return current_(); }

    private:
        static RenderCommandList *&current_() noexcept
        {
            static thread_local RenderCommandList *current = nullptr;
            return current;
        }
        RenderCommandList *previous_ = nullptr;
    };
}
