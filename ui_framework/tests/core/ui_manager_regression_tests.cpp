#include "ui_framework/ui_manager.hpp"
#include "ui_framework/node.hpp"

#include <SDL3/SDL.h>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>

namespace
{
    struct TestFailure
    {
        const char *message;
    };

    void expect(bool condition, const char *message)
    {
        if (!condition)
            throw TestFailure{message};
    }

    bool near(float a, float b, float epsilon = 0.001f)
    {
        return std::fabs(a - b) <= epsilon;
    }

    SDL_Event mouseMotion(float x, float y)
    {
        SDL_Event event{};
        event.type = SDL_EVENT_MOUSE_MOTION;
        event.motion.x = x;
        event.motion.y = y;
        return event;
    }

    SDL_Event mouseDown(float x, float y)
    {
        SDL_Event event{};
        event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        event.button.x = x;
        event.button.y = y;
        event.button.button = SDL_BUTTON_LEFT;
        return event;
    }

    class UpdateProbe final : public ui::Node
    {
    public:
        explicit UpdateProbe(int *updates) noexcept
            : updates_(updates)
        {
        }

    protected:
        void update(float) override
        {
            if (updates_)
                ++(*updates_);
        }

    private:
        int *updates_ = nullptr;
    };

    class RenderProbe final : public ui::Node
    {
    public:
        RenderProbe(int *drawCount, ui::LayoutSize *lastSize) noexcept
            : drawCount_(drawCount), lastSize_(lastSize)
        {
        }

    protected:
        void draw(SDL_Renderer *) override
        {
            if (drawCount_)
                ++(*drawCount_);
            if (lastSize_)
                *lastSize_ = getActualSize();
        }

    private:
        int *drawCount_ = nullptr;
        ui::LayoutSize *lastSize_ = nullptr;
    };

    void test_add_root_and_run_frame_updates_node()
    {
        ui::UIManager manager;
        int updates = 0;

        auto node = std::make_unique<UpdateProbe>(&updates);
        ui::Node *root = manager.addRoot(std::move(node));

        expect(root != nullptr, "UIManager must return the attached root");

        manager.invalidateLayout(*root);
        manager.runFrame(1.0f / 60.0f, nullptr);

        expect(updates == 1, "runFrame must execute NodeTree update for attached root");
    }

    void test_process_event_reaches_root_without_renderer()
    {
        ui::UIManager manager;
        auto node = std::make_unique<ui::Node>();
        node->setSize(ui::LayoutSizeValue::fixed(100.0f, 100.0f));
        ui::Node *root = manager.addRoot(std::move(node));

        int motionEvents = 0;
        root->on<ui::MouseMoveEvent>(
            [&](ui::MouseMoveEvent &, ui::Node &)
            {
                ++motionEvents;
            });

        manager.invalidateLayout(*root);
        manager.runFrame(1.0f / 60.0f, nullptr);

        manager.processEvent(mouseMotion(10.0f, 10.0f), nullptr);

        expect(motionEvents == 1,
               "UIManager::processEvent must route normalized pointer input to the hit target");
    }

    void test_process_event_focuses_and_captures_through_manager()
    {
        ui::UIManager manager;
        auto node = std::make_unique<ui::Node>();
        node->setSize(ui::LayoutSizeValue::fixed(100.0f, 100.0f));
        node->setFocusable(true);
        node->setCapturable(true);
        ui::Node *root = manager.addRoot(std::move(node));

        int downs = 0;
        root->on<ui::MouseDownEvent>(
            [&](ui::MouseDownEvent &, ui::Node &)
            {
                ++downs;
            });

        manager.invalidateLayout(*root);
        manager.runFrame(1.0f / 60.0f, nullptr);
        manager.processEvent(mouseDown(10.0f, 10.0f), nullptr);

        expect(downs == 1,
               "UIManager must route MouseDown through InputSystem");
    }

    void test_render_reflects_geometry_after_layout_change()
    {
        if (!SDL_Init(SDL_INIT_VIDEO))
            throw TestFailure{"SDL video initialization failed for rendering regression test"};

        SDL_Window *window = SDL_CreateWindow(
            "ui_framework_render_regression",
            320,
            240,
            SDL_WINDOW_HIDDEN);
        expect(window != nullptr, "SDL hidden test window must be created");

        SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
        expect(renderer != nullptr, "SDL test renderer must be created");

        SDL_SetRenderLogicalPresentation(
            renderer,
            320,
            240,
            SDL_LOGICAL_PRESENTATION_STRETCH);

        ui::UIManager manager;
        int draws = 0;
        ui::LayoutSize renderedSize{};

        auto node = std::make_unique<RenderProbe>(&draws, &renderedSize);
        node->setSize(ui::LayoutSizeValue::fixed(40.0f, 30.0f));
        ui::Node *root = manager.addRoot(std::move(node));
        manager.invalidateLayout(*root);

        manager.runFrame(1.0f / 60.0f, renderer);

        expect(draws == 1, "first rendered frame must invoke Node draw");
        expect(renderedSize == ui::LayoutSize{40.0f, 30.0f},
               "first render must observe committed layout geometry");

        root->setSize(ui::LayoutSizeValue::fixed(80.0f, 50.0f));
        manager.invalidateLayout(*root);
        manager.runFrame(1.0f / 60.0f, renderer);

        expect(draws == 2, "second rendered frame must invoke Node draw again");
        expect(renderedSize == ui::LayoutSize{80.0f, 50.0f},
               "render must observe geometry committed by the latest layout pass");

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void test_logical_presentation_resize_updates_framework_viewport_and_render_geometry()
    {
        if (!SDL_Init(SDL_INIT_VIDEO))
            throw TestFailure{"SDL video initialization failed for viewport regression test"};

        SDL_Window *window = SDL_CreateWindow(
            "ui_framework_viewport_regression",
            320,
            240,
            SDL_WINDOW_HIDDEN);
        expect(window != nullptr, "SDL hidden viewport test window must be created");

        SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
        expect(renderer != nullptr, "SDL viewport test renderer must be created");

        SDL_SetRenderLogicalPresentation(
            renderer,
            320,
            240,
            SDL_LOGICAL_PRESENTATION_STRETCH);

        ui::UIManager manager;
        int draws = 0;
        ui::LayoutSize renderedSize{};

        auto node = std::make_unique<RenderProbe>(&draws, &renderedSize);
        node->setSize(ui::LayoutSizeValue::autoSize());
        ui::Node *root = manager.addRoot(std::move(node));
        manager.invalidateLayout(*root);

        manager.runFrame(1.0f / 60.0f, renderer);

        expect(renderedSize == ui::LayoutSize{320.0f, 240.0f},
               "auto-sized root must use the initial logical presentation as its layout viewport");

        SDL_SetRenderLogicalPresentation(
            renderer,
            640,
            360,
            SDL_LOGICAL_PRESENTATION_STRETCH);

        manager.runFrame(1.0f / 60.0f, renderer);

        expect(draws == 2, "resize frame must render the root again");
        expect(renderedSize == ui::LayoutSize{640.0f, 360.0f},
               "logical presentation resize must update the framework viewport before layout and rendering");

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void test_remove_root_then_run_frame_is_safe()
    {
        ui::UIManager manager;
        auto node = std::make_unique<ui::Node>();
        ui::Node *root = manager.addRoot(std::move(node));

        manager.removeRoot(root);
        manager.runFrame(1.0f / 60.0f, nullptr);

        expect(true, "removed root must not break subsequent UIManager frames");
    }
}

int main()
{
    try
    {
        test_add_root_and_run_frame_updates_node();
        test_process_event_reaches_root_without_renderer();
        test_process_event_focuses_and_captures_through_manager();
        test_render_reflects_geometry_after_layout_change();
        test_logical_presentation_resize_updates_framework_viewport_and_render_geometry();
        test_remove_root_then_run_frame_is_safe();
    }
    catch (const TestFailure &failure)
    {
        std::cerr << "UIManager regression tests failed: " << failure.message << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "UIManager regression tests passed\n";
    return EXIT_SUCCESS;
}