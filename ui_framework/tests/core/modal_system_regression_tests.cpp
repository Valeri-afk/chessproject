#include "ui_framework/ui_manager.hpp"
#include "ui_framework/node.hpp"
#include "ui_framework/panel_node.hpp"

#include <SDL3/SDL.h>

#include <cstdlib>
#include <iostream>
#include <memory>

namespace
{
    struct TestFailure { const char *message; };

    void expect(bool condition, const char *message)
    {
        if (!condition)
            throw TestFailure{message};
    }

    SDL_Event keyDown(SDL_Keycode key)
    {
        SDL_Event event{};
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = key;
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
        explicit UpdateProbe(int *updates) noexcept : updates_(updates) {}

    protected:
        void update(float) override
        {
            if (updates_)
                ++*updates_;
        }

    private:
        int *updates_ = nullptr;
    };

    void prepare(ui::UIManager &manager, ui::Node &node)
    {
        manager.invalidateLayout(node);
        manager.runFrame(0.0f, nullptr);
    }

    void test_modal_owns_interaction_without_pausing_lower_modals()
    {
        ui::UIManager manager;
        int lowerUpdates = 0;
        int upperClicks = 0;
        int lowerClicks = 0;

        auto lower = std::make_unique<ui::PanelNode>();
        lower->setSize(ui::LayoutSizeValue::fixed(100.0f, 100.0f));
        lower->setFocusable(true);
        auto lowerProbe = std::make_unique<UpdateProbe>(&lowerUpdates);
        lowerProbe->setSize(ui::LayoutSizeValue::fixed(20.0f, 20.0f));
        lower->addChild(std::move(lowerProbe), 0);
        ui::Node *lowerNode = manager.addOverlay(std::move(lower));

        auto upper = std::make_unique<ui::Node>();
        upper->setSize(ui::LayoutSizeValue::fixed(100.0f, 100.0f));
        upper->setFocusable(true);
        ui::Node *upperNode = manager.addOverlay(std::move(upper));

        lowerNode->on<ui::MouseDownEvent>([&](ui::MouseDownEvent &, ui::Node &) { ++lowerClicks; });
        upperNode->on<ui::MouseDownEvent>([&](ui::MouseDownEvent &, ui::Node &) { ++upperClicks; });

        prepare(manager, *upperNode);
        expect(manager.showModal(*lowerNode), "first modal must open");
        expect(manager.showModal(*upperNode), "second modal must open above first");
        expect(manager.getActiveModal() == upperNode, "top modal must own interaction");

        manager.processEvent(mouseDown(10.0f, 10.0f));
        expect(upperClicks == 1, "top modal must receive pointer input");
        expect(lowerClicks == 0, "lower modal must not receive pointer input");

        manager.runFrame(1.0f / 60.0f, nullptr);
        expect(lowerUpdates == 1, "lower modal must continue updating while blocked");
    }

    void test_modal_focus_and_tab_trap()
    {
        ui::UIManager manager;
        auto modal = std::make_unique<ui::PanelNode>();
        modal->setSize(ui::LayoutSizeValue::fixed(100.0f, 100.0f));

        auto first = std::make_unique<ui::Node>();
        first->setFocusable(true);
        first->setSize(ui::LayoutSizeValue::fixed(20.0f, 20.0f));
        ui::Node *firstPtr = first.get();
        modal->addChild(std::move(first), 0);

        auto second = std::make_unique<ui::Node>();
        second->setFocusable(true);
        second->setSize(ui::LayoutSizeValue::fixed(20.0f, 20.0f));
        ui::Node *secondPtr = second.get();
        modal->addChild(std::move(second), 1);

        ui::Node *modalPtr = manager.addOverlay(std::move(modal));
        prepare(manager, *modalPtr);

        expect(manager.showModal(*modalPtr), "modal must open");
        expect(manager.getActiveModal() == modalPtr, "opened modal must become active");

        // The modal root is not focusable, so the first focusable descendant is selected.
        // The first TAB moves to the second, and the next TAB wraps to the first.
        expect(firstPtr->isFocusable(), "first child must remain focusable");
        manager.processEvent(keyDown(SDLK_TAB));
        manager.processEvent(keyDown(SDLK_TAB));

        expect(manager.getActiveModal() == modalPtr, "focus trap must keep modal active");
    }

    void test_escape_can_be_consumed_by_focused_node()
    {
        ui::UIManager manager;
        auto modal = std::make_unique<ui::Node>();
        modal->setSize(ui::LayoutSizeValue::fixed(100.0f, 100.0f));
        modal->setFocusable(true);
        ui::Node *modalPtr = manager.addOverlay(std::move(modal));
        prepare(manager, *modalPtr);

        const auto handler = modalPtr->on<ui::KeyDownEvent>(
            [](ui::KeyDownEvent &event, ui::Node &)
            {
                if (event.key == ui::KeyCode::ESCAPE)
                    event.stopPropagation();
            });

        expect(manager.showModal(*modalPtr), "modal must open");
        manager.processEvent(keyDown(SDLK_ESCAPE));
        expect(manager.getActiveModal() == modalPtr, "consumed Escape must not close modal");

        modalPtr->removeEventHandler<ui::KeyDownEvent>(handler);
        manager.processEvent(keyDown(SDLK_ESCAPE));
        expect(manager.getActiveModal() == nullptr, "unconsumed Escape must close modal");
    }

    void test_outside_click_policy_is_independent_of_backdrop()
    {
        ui::UIManager manager;
        auto modal = std::make_unique<ui::Node>();
        modal->setSize(ui::LayoutSizeValue::fixed(20.0f, 20.0f));
        modal->setFocusable(true);
        ui::Node *modalPtr = manager.addOverlay(std::move(modal));
        prepare(manager, *modalPtr);

        ui::ModalOptions options;
        options.showBackdrop = false;
        options.outsideClick = ui::OutsideClickBehavior::Close;

        expect(manager.showModal(*modalPtr, options), "modal with custom options must open");
        manager.processEvent(mouseDown(80.0f, 80.0f));
        expect(manager.getActiveModal() == nullptr,
               "outside click must close modal even without a backdrop");
    }

    void test_removing_lower_modal_closes_entire_modal_branch()
    {
        ui::UIManager manager;
        auto lower = std::make_unique<ui::Node>();
        lower->setSize(ui::LayoutSizeValue::fixed(100.0f, 100.0f));
        lower->setFocusable(true);
        ui::Node *lowerPtr = manager.addOverlay(std::move(lower));

        auto upper = std::make_unique<ui::Node>();
        upper->setSize(ui::LayoutSizeValue::fixed(100.0f, 100.0f));
        upper->setFocusable(true);
        ui::Node *upperPtr = manager.addOverlay(std::move(upper));

        prepare(manager, *upperPtr);
        expect(manager.showModal(*lowerPtr), "lower modal must open");
        expect(manager.showModal(*upperPtr), "upper modal must open");

        manager.removeOverlay(lowerPtr);
        manager.runFrame(0.0f, nullptr);

        expect(manager.getActiveModal() == nullptr,
               "removing the base modal must invalidate the entire modal branch");
    }
}

int main()
{
    try
    {
        test_modal_owns_interaction_without_pausing_lower_modals();
        test_modal_focus_and_tab_trap();
        test_escape_can_be_consumed_by_focused_node();
        test_outside_click_policy_is_independent_of_backdrop();
        test_removing_lower_modal_closes_entire_modal_branch();
    }
    catch (const TestFailure &failure)
    {
        std::cerr << "ModalSystem regression tests failed: " << failure.message << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "ModalSystem regression tests passed\n";
    return EXIT_SUCCESS;
}
