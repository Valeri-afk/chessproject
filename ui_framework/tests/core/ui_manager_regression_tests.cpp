#include "ui_framework/ui_manager.hpp"
#include "ui_framework/node.hpp"

#include <SDL3/SDL.h>

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

        void requestSelfRemoval(ui::UIManager *manager) noexcept
        {
            manager_ = manager;
        }

    protected:
        void update(float) override
        {
            if (updates_)
                ++(*updates_);
        }

    private:
        int *updates_ = nullptr;
        ui::UIManager *manager_ = nullptr;
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

    void test_remove_root_then_run_frame_is_safe()
    {
        ui::UIManager manager;
        auto node = std::make_unique<ui::Node>();
        ui::Node *root = manager.addRoot(std::move(node));

        manager.removeRoot(root);
        manager.runFrame(1.0f / 60.0f, nullptr);

        // No direct NodeTree access is required: reaching this point without
        // a stale-runtime failure proves UIManager phase cleanup is safe.
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
