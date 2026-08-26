#include "layout_system.hpp"
#include "node_tree.hpp"
#include "ui_framework/panel_node.hpp"
#include "ui_framework/stack_panel_node.hpp"

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

    std::unique_ptr<ui::Node> makeNode(float width = 100.0f, float height = 100.0f)
    {
        auto node = std::make_unique<ui::Node>();
        node->setSize(ui::LayoutSizeValue::fixed(width, height));
        return node;
    }

    std::unique_ptr<ui::PanelNode> makePanel(float width = 100.0f, float height = 100.0f)
    {
        auto panel = std::make_unique<ui::PanelNode>();
        panel->setSize(ui::LayoutSizeValue::fixed(width, height));
        return panel;
    }

    class UpdateChildNode final : public ui::Node
    {
    public:
        enum class Action
        {
            None,
            RemoveSibling,
            AddSibling
        };

        UpdateChildNode(
            ui::PanelNode *parent,
            int *updateCount,
            Action action = Action::None,
            ui::Node *sibling = nullptr,
            int *addedUpdateCount = nullptr) noexcept
            : parent_(parent),
              updateCount_(updateCount),
              action_(action),
              sibling_(sibling),
              addedUpdateCount_(addedUpdateCount)
        {
        }

        ui::Node::Id addedNodeId() const noexcept
        {
            return addedNodeId_;
        }

    protected:
        void update(float) override
        {
            if (updateCount_)
                ++(*updateCount_);

            if (!parent_)
                return;

            switch (action_)
            {
            case Action::RemoveSibling:
                if (sibling_ && parent_->getChildCount() > 1)
                    parent_->removeChild(*sibling_);
                action_ = Action::None;
                break;

            case Action::AddSibling:
                if (addedNodeId_ == 0)
                {
                    auto child = std::make_unique<UpdateChildNode>(
                        parent_,
                        addedUpdateCount_);
                    addedNodeId_ = child->getId();
                    parent_->addChild(std::move(child), parent_->getChildCount());
                }
                break;

            case Action::None:
                break;
            }
        }

    private:
        ui::PanelNode *parent_ = nullptr;
        int *updateCount_ = nullptr;
        Action action_ = Action::None;
        ui::Node *sibling_ = nullptr;
        int *addedUpdateCount_ = nullptr;
        ui::Node::Id addedNodeId_ = 0;
    };

    struct LayoutFixture
    {
        ui::NodeTree tree;
        ui::LayoutSystem layout;

        LayoutFixture()
        {
            layout.setViewportSize({100.0f, 100.0f});
        }

        void process()
        {
            layout.requestFullLayout(tree);
            layout.processLayoutQueue(tree);
        }
    };

    void test_update_removing_future_sibling_defers_removal_until_frame_end()
    {
        ui::NodeTree tree;
        auto parent = makePanel();
        ui::PanelNode *panel = parent.get();
        tree.attachRoot(0, std::move(parent));

        int firstUpdates = 0;
        int secondUpdates = 0;

        auto second = std::make_unique<UpdateChildNode>(panel, &secondUpdates);
        UpdateChildNode *secondPtr = second.get();
        tree.attachChild(*panel, std::move(second), 1);

        auto first = std::make_unique<UpdateChildNode>(
            panel,
            &firstUpdates,
            UpdateChildNode::Action::RemoveSibling,
            secondPtr);
        tree.attachChild(*panel, std::move(first), 0);

        tree.update(1.0f / 60.0f);

        expect(firstUpdates == 1, "first child must update once");
        expect(secondUpdates == 1,
               "future sibling must still finish the current update pass before deferred removal");
        expect(panel->getChildCount() == 1,
               "deferred sibling removal must be applied after the update pass");
    }

    void test_update_adding_sibling_starts_next_frame()
    {
        ui::NodeTree tree;
        auto parent = makePanel();
        ui::PanelNode *panel = parent.get();
        tree.attachRoot(0, std::move(parent));

        int sourceUpdates = 0;
        int addedUpdates = 0;

        auto source = std::make_unique<UpdateChildNode>(
            panel,
            &sourceUpdates,
            UpdateChildNode::Action::AddSibling,
            nullptr,
            &addedUpdates);
        UpdateChildNode *sourcePtr = source.get();
        tree.attachChild(*panel, std::move(source), 0);

        tree.update(1.0f / 60.0f);

        expect(sourceUpdates == 1, "source child must update in the current frame");
        expect(sourcePtr->addedNodeId() != 0, "source child must create a sibling");
        expect(addedUpdates == 0, "new sibling must not update in the current frame");
        expect(panel->getChildCount() == 2, "new sibling must be attached after flush");

        tree.update(1.0f / 60.0f);

        expect(addedUpdates == 1, "new sibling must update on the next frame");
    }

    void test_stale_layout_queue_entry_is_ignored_after_root_removal()
    {
        ui::NodeTree tree;
        ui::Node *root = tree.attachRoot(0, makePanel());

        tree.removeRoot(root);

        int callbacks = 0;
        tree.forEachLayoutQueue([&](ui::Node &)
        {
            ++callbacks;
        });

        expect(callbacks == 0, "removed layout root must be ignored when queue is consumed");
    }

    void test_overlay_hit_test_precedes_root()
    {
        LayoutFixture f;
        ui::Node *root = f.tree.attachRoot(0, makeNode());
        ui::Node *overlay = f.tree.attachOverlay(0, makeNode());

        f.process();

        expect(f.tree.hitTest(50.0f, 50.0f) == overlay,
               "overlay must win hit-testing over an overlapping root");
        expect(f.tree.hitTest(50.0f, 50.0f) != root,
               "overlapping root must not win over overlay");
    }

    void test_modal_hit_test_never_escapes_modal_subtree()
    {
        LayoutFixture f;

        ui::Node *root = f.tree.attachRoot(0, makeNode());
        ui::Node *modal = f.tree.attachOverlay(0, makeNode(40.0f, 40.0f));

        f.process();

        expect(f.tree.hitTest(20.0f, 20.0f, modal) == modal,
               "modal hit-test must resolve the modal root inside its subtree");
        expect(f.tree.hitTest(80.0f, 80.0f, modal) == nullptr,
               "modal hit-test must not fall through to another root");
        expect(f.tree.hitTest(80.0f, 80.0f) == root,
               "normal hit-test must still reach the underlying root without modal restriction");
    }

    void test_clip_to_bounds_blocks_outside_descendant()
    {
        LayoutFixture f;
        auto parent = std::make_unique<ui::StackPanelNode>();
        parent->setSize(ui::LayoutSizeValue::fixed(50.0f, 50.0f));
        parent->setClipToBounds(true);
        ui::StackPanelNode *panel = parent.get();
        f.tree.attachRoot(0, std::move(parent));

        auto child = makeNode(40.0f, 40.0f);
        child->setPosition({40.0f, 40.0f});
        child->setPositionMode(ui::PositionMode::Absolute);
        ui::Node *childPtr = f.tree.attachChild(*panel, std::move(child), 0);

        f.process();

        expect(f.tree.hitTest(45.0f, 45.0f) == childPtr,
               "visible part of an overflowing child must remain hittable");
        expect(f.tree.hitTest(60.0f, 60.0f) == nullptr,
               "clipToBounds must block hits outside the parent bounds");
    }

    void test_invisible_and_disabled_subtrees_are_not_hittable()
    {
        LayoutFixture f;

        auto invisible = makePanel();
        invisible->setVisible(false);
        ui::Node *invisibleRoot = f.tree.attachRoot(0, std::move(invisible));

        auto disabled = makePanel();
        disabled->setEnabled(false);
        ui::Node *disabledRoot = f.tree.attachRoot(1, std::move(disabled));

        auto visible = makeNode();
        ui::Node *visibleRoot = f.tree.attachRoot(2, std::move(visible));

        f.process();

        expect(f.tree.hitTest(10.0f, 10.0f) == visibleRoot,
               "visible enabled root must be the hit target");
        expect(f.tree.hitTest(10.0f, 10.0f) != invisibleRoot,
               "invisible root must never become the hit target");
        expect(f.tree.hitTest(10.0f, 10.0f) != disabledRoot,
               "disabled root must never become the hit target");
    }
}

int main()
{
    try
    {
        test_update_removing_future_sibling_defers_removal_until_frame_end();
        test_update_adding_sibling_starts_next_frame();
        test_stale_layout_queue_entry_is_ignored_after_root_removal();
        test_overlay_hit_test_precedes_root();
        test_modal_hit_test_never_escapes_modal_subtree();
        test_clip_to_bounds_blocks_outside_descendant();
        test_invisible_and_disabled_subtrees_are_not_hittable();
    }
    catch (const TestFailure &failure)
    {
        std::cerr << "NodeTree regression tests failed: " << failure.message << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "NodeTree regression tests passed\n";
    return EXIT_SUCCESS;
}
