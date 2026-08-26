#include "layout_system.hpp"
#include "node_tree.hpp"
#include "ui_framework/node.hpp"
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

    void clearLayoutQueue(ui::NodeTree &tree)
    {
        tree.forEachLayoutQueue([](ui::Node &) {});
    }

    void expectRootQueued(ui::NodeTree &tree, ui::Node *root, const char *message)
    {
        int matches = 0;
        tree.forEachLayoutQueue([&](ui::Node &node)
        {
            if (&node == root)
                ++matches;
        });
        expect(matches == 1, message);
    }

    void test_layout_affecting_node_setters_invalidate_tree()
    {
        ui::NodeTree tree;
        auto node = std::make_unique<ui::Node>();
        ui::Node *root = tree.attachRoot(0, std::move(node));
        clearLayoutQueue(tree);

        root->setSize(ui::LayoutSizeValue::fixed(40.0f, 30.0f));
        expectRootQueued(tree, root, "setSize must invalidate layout");
        clearLayoutQueue(tree);

        root->setPosition({10.0f, 20.0f});
        expectRootQueued(tree, root, "setPosition must invalidate layout");
        clearLayoutQueue(tree);

        root->setPositionMode(ui::PositionMode::Absolute);
        expectRootQueued(tree, root, "setPositionMode must invalidate layout");
        clearLayoutQueue(tree);

        root->setMinSize({10.0f, 10.0f});
        expectRootQueued(tree, root, "setMinSize must invalidate layout");
        clearLayoutQueue(tree);

        root->setMaxSize({80.0f, 70.0f});
        expectRootQueued(tree, root, "setMaxSize must invalidate layout");
        clearLayoutQueue(tree);

        root->setPadding({1.0f, 2.0f, 3.0f, 4.0f});
        expectRootQueued(tree, root, "setPadding must invalidate layout");
        clearLayoutQueue(tree);

        root->setBorder({1.0f, 1.0f, 1.0f, 1.0f});
        expectRootQueued(tree, root, "setBorder must invalidate layout");
        clearLayoutQueue(tree);

        root->setVisible(false);
        expectRootQueued(tree, root, "setVisible must invalidate layout");
    }

    void test_redundant_node_setter_does_not_add_duplicate_layout_work()
    {
        ui::NodeTree tree;
        auto node = std::make_unique<ui::Node>();
        ui::Node *root = tree.attachRoot(0, std::move(node));
        clearLayoutQueue(tree);

        const ui::LayoutSizeValue size = ui::LayoutSizeValue::fixed(40.0f, 30.0f);
        root->setSize(size);
        root->setSize(size);

        int queued = 0;
        tree.forEachLayoutQueue([&](ui::Node &node)
        {
            if (&node == root)
                ++queued;
        });

        expect(queued == 1, "repeated layout invalidation must remain deduplicated");
    }

    void test_stack_panel_properties_invalidate_layout()
    {
        ui::NodeTree tree;
        auto panel = std::make_unique<ui::StackPanelNode>();
        ui::StackPanelNode *root = panel.get();
        tree.attachRoot(0, std::move(panel));
        clearLayoutQueue(tree);

        root->setOrientation(ui::StackPanelNode::Orientation::Horizontal);
        expectRootQueued(tree, root, "orientation change must invalidate layout");
        clearLayoutQueue(tree);

        root->setGap(8.0f);
        expectRootQueued(tree, root, "gap change must invalidate layout");
        clearLayoutQueue(tree);

        root->setMainAlignment(ui::MainAxisAlignment::CENTER);
        expectRootQueued(tree, root, "main alignment change must invalidate layout");
        clearLayoutQueue(tree);

        root->setCrossAlignment(ui::CrossAxisAlignment::END);
        expectRootQueued(tree, root, "cross alignment change must invalidate layout");
    }

    void test_visibility_change_reflows_stack_children()
    {
        ui::NodeTree tree;
        ui::LayoutSystem layout;
        layout.setViewportSize({100.0f, 100.0f});

        auto panel = std::make_unique<ui::StackPanelNode>();
        panel->setSize(ui::LayoutSizeValue::fixed(100.0f, 100.0f));
        ui::StackPanelNode *root = panel.get();
        tree.attachRoot(0, std::move(panel));

        auto first = std::make_unique<ui::Node>();
        first->setSize(ui::LayoutSizeValue::fixed(100.0f, 20.0f));
        ui::Node *firstPtr = tree.attachChild(*root, std::move(first), 0);

        auto second = std::make_unique<ui::Node>();
        second->setSize(ui::LayoutSizeValue::fixed(100.0f, 20.0f));
        ui::Node *secondPtr = tree.attachChild(*root, std::move(second), 1);

        layout.requestFullLayout(tree);
        layout.processLayoutQueue(tree);
        expect(secondPtr->getActualPosition().y > firstPtr->getActualPosition().y,
               "visible stack children must occupy distinct flow positions");

        secondPtr->setVisible(false);
        expectRootQueued(tree, root, "hiding stack child must invalidate containing layout root");

        layout.processLayoutQueue(tree);

        expect(secondPtr->getActualSize().height == 0.0f ||
                   secondPtr->getActualPosition().y == firstPtr->getActualPosition().y,
               "hidden stack child must no longer occupy normal flow layout");
    }

    void test_position_mode_change_reflows_stack_layout()
    {
        ui::NodeTree tree;
        ui::LayoutSystem layout;
        layout.setViewportSize({100.0f, 100.0f});

        auto panel = std::make_unique<ui::StackPanelNode>();
        panel->setSize(ui::LayoutSizeValue::fixed(100.0f, 100.0f));
        ui::StackPanelNode *root = panel.get();
        tree.attachRoot(0, std::move(panel));

        auto first = std::make_unique<ui::Node>();
        first->setSize(ui::LayoutSizeValue::fixed(100.0f, 20.0f));
        ui::Node *firstPtr = tree.attachChild(*root, std::move(first), 0);

        auto second = std::make_unique<ui::Node>();
        second->setSize(ui::LayoutSizeValue::fixed(100.0f, 20.0f));
        ui::Node *secondPtr = tree.attachChild(*root, std::move(second), 1);

        layout.requestFullLayout(tree);
        layout.processLayoutQueue(tree);
        const float initialSecondY = secondPtr->getActualPosition().y;

        secondPtr->setPositionMode(ui::PositionMode::Absolute);
        secondPtr->setPosition({0.0f, 60.0f});
        expectRootQueued(tree, root, "position mode and position changes must invalidate layout");

        layout.processLayoutQueue(tree);

        expect(secondPtr->getActualPosition().y != initialSecondY,
               "absolute position mode must change arranged position after re-layout");
        expect(firstPtr->getActualPosition().y == 0.0f,
               "flow child must remain at the beginning of the stack");
    }
}

int main()
{
    try
    {
        test_layout_affecting_node_setters_invalidate_tree();
        test_redundant_node_setter_does_not_add_duplicate_layout_work();
        test_stack_panel_properties_invalidate_layout();
        test_visibility_change_reflows_stack_children();
        test_position_mode_change_reflows_stack_layout();
    }
    catch (const TestFailure &failure)
    {
        std::cerr << "Node regression tests failed: " << failure.message << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "Node regression tests passed\n";
    return EXIT_SUCCESS;
}
