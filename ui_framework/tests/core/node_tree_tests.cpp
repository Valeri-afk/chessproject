#include "layout_system.hpp"
#include "node_tree.hpp"
#include "panel_node.hpp"
#include "stack_panel_node.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace
{
    struct TestFailure
    {
        std::string message;
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

    class ReentrantUnmountNode final : public ui::Node
    {
    public:
        ReentrantUnmountNode(ui::PanelNode *parent, int *unmountCount) noexcept
            : parent_(parent), unmountCount_(unmountCount)
        {
        }

    protected:
        void onUnmount() override
        {
            if (unmountCount_)
                ++(*unmountCount_);

            if (parent_)
                parent_->removeChild(*this);
        }

    private:
        ui::PanelNode *parent_ = nullptr;
        int *unmountCount_ = nullptr;
    };

    class MountMutationNode final : public ui::Node
    {
    public:
        MountMutationNode(ui::PanelNode *parent, int *mountCount, bool removeSelf) noexcept
            : parent_(parent), mountCount_(mountCount), removeSelf_(removeSelf)
        {
        }

    protected:
        void onMount() override
        {
            if (mountCount_)
                ++(*mountCount_);

            if (!parent_ || !removeSelf_)
                return;

            parent_->removeChild(*this);
        }

    private:
        ui::PanelNode *parent_ = nullptr;
        int *mountCount_ = nullptr;
        bool removeSelf_ = false;
    };

    class ParentUnmountNode final : public ui::PanelNode
    {
    public:
        explicit ParentUnmountNode(int *unmountCount) noexcept
            : unmountCount_(unmountCount)
        {
        }

        void setChildToRemove(ui::Node *child) noexcept
        {
            child_ = child;
        }

    protected:
        void onUnmount() override
        {
            if (unmountCount_)
                ++(*unmountCount_);

            if (child_)
                removeChild(*child_);
        }

    private:
        int *unmountCount_ = nullptr;
        ui::Node *child_ = nullptr;
    };

    class UpdateMutationNode final : public ui::Node
    {
    public:
        enum class Action
        {
            None,
            RemoveSelf,
            AddRoot
        };

        UpdateMutationNode(
            ui::NodeTree *tree,
            int *updateCount,
            Action action = Action::None,
            int *addedNodeUpdateCount = nullptr) noexcept
            : tree_(tree),
              updateCount_(updateCount),
              action_(action),
              addedNodeUpdateCount_(addedNodeUpdateCount)
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

            if (!tree_)
                return;

            switch (action_)
            {
            case Action::RemoveSelf:
                tree_->removeRoot(this);
                break;

            case Action::AddRoot:
            {
                if (addedNodeId_ != 0)
                    break;

                auto added = std::make_unique<UpdateMutationNode>(
                    tree_,
                    addedNodeUpdateCount_);
                addedNodeId_ = added->getId();
                tree_->attachRoot(1, std::move(added));
                break;
            }

            case Action::None:
                break;
            }
        }

    private:
        ui::NodeTree *tree_ = nullptr;
        int *updateCount_ = nullptr;
        Action action_ = Action::None;
        int *addedNodeUpdateCount_ = nullptr;
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

        void processLayout()
        {
            layout.requestFullLayout(tree);
            layout.processLayoutQueue(tree);
        }
    };

    void test_attach_root_registers_node()
    {
        ui::NodeTree tree;
        auto node = makeNode();
        const ui::Node::Id id = node->getId();

        ui::Node *root = tree.attachRoot(0, std::move(node));

        expect(root != nullptr, "attachRoot must return the live node");
        expect(tree.rootsCount() == 1, "root count must increase after attach");
        expect(tree.getRoot(0) == root, "getRoot must return the attached root");
        expect(tree.isRoot(root), "attached node must be recognized as a root");
        expect(tree.findNode(id) == root, "attached root must be registered in liveNodes");
        expect(tree.isNodeLive(id), "attached root must be live");
        expect(root->getParent() == nullptr, "root must not have a parent");
    }

    void test_remove_root_unregisters_node()
    {
        ui::NodeTree tree;
        ui::Node *root = tree.attachRoot(0, makeNode());
        const ui::Node::Id id = root->getId();

        tree.removeRoot(root);

        expect(tree.rootsCount() == 0, "root count must decrease after removal");
        expect(tree.getRoot(0) == nullptr, "removed root must not remain in root storage");
        expect(tree.findNode(id) == nullptr, "removed root must be absent from liveNodes");
        expect(!tree.isNodeLive(id), "removed root must not remain live");
        expect(!tree.isRoot(root), "removed node must no longer be a root");
    }

    void test_attach_roots_preserves_requested_order()
    {
        ui::NodeTree tree;
        ui::Node *first = tree.attachRoot(0, makeNode());
        ui::Node *second = tree.attachRoot(1, makeNode());
        ui::Node *inserted = tree.attachRoot(1, makeNode());

        expect(tree.rootsCount() == 3, "three roots must be present");
        expect(tree.getRoot(0) == first, "first root must remain at index 0");
        expect(tree.getRoot(1) == inserted, "indexed insertion must place root at requested index");
        expect(tree.getRoot(2) == second, "existing root must shift after indexed insertion");

        ui::Node *appended = tree.attachRoot(100, makeNode());
        expect(tree.getRoot(3) == appended, "out-of-range root index must append");
    }

    void test_overlay_lifecycle_is_separate_from_roots()
    {
        ui::NodeTree tree;
        ui::Node *root = tree.attachRoot(0, makeNode());
        ui::Node *overlay = tree.attachOverlay(0, makeNode());

        expect(tree.rootsCount() == 1, "root count must not include overlays");
        expect(tree.overlaysCount() == 1, "overlay count must increase after attach");
        expect(tree.getRoot(0) == root, "root storage must remain unchanged");
        expect(tree.getOverlay(0) == overlay, "overlay storage must contain the overlay");
        expect(tree.isRoot(root), "root must be recognized as root");
        expect(tree.isOverlay(overlay), "overlay must be recognized as overlay");
        expect(tree.findNode(overlay->getId()) == overlay, "overlay must be registered as live");

        const ui::Node::Id overlayId = overlay->getId();
        tree.removeOverlay(overlay);

        expect(tree.overlaysCount() == 0, "overlay count must decrease after removal");
        expect(tree.findNode(overlayId) == nullptr, "removed overlay must be unregistered");
        expect(tree.findNode(root->getId()) == root, "removing overlay must not affect roots");
    }

    void test_attach_and_remove_child_subtree_updates_live_nodes()
    {
        ui::NodeTree tree;
        auto parent = makePanel();
        ui::PanelNode *parentPtr = parent.get();
        ui::Node *root = tree.attachRoot(0, std::move(parent));

        expect(root == parentPtr, "attached panel must remain the root instance");

        auto childPanel = makePanel();
        ui::PanelNode *childPanelPtr = childPanel.get();
        const ui::Node::Id childId = childPanelPtr->getId();

        auto grandchild = makeNode();
        const ui::Node::Id grandchildId = grandchild->getId();
        childPanelPtr->addChild(std::move(grandchild), 0);

        ui::Node *attachedChild = tree.attachChild(*parentPtr, std::move(childPanel), 0);

        expect(attachedChild == childPanelPtr, "attachChild must return the live child");
        expect(parentPtr->getChildCount() == 1, "parent must contain the attached child");
        expect(tree.findNode(childId) == childPanelPtr, "child panel must be registered");
        expect(tree.findNode(grandchildId) != nullptr, "entire child subtree must be registered");
        expect(childPanelPtr->getParent() == parentPtr, "child parent relationship must be established");

        tree.removeChild(*parentPtr, *childPanelPtr);

        expect(parentPtr->getChildCount() == 0, "removed child must leave the parent");
        expect(tree.findNode(childId) == nullptr, "removed child must be unregistered");
        expect(tree.findNode(grandchildId) == nullptr, "removed descendant must also be unregistered");
        expect(tree.isNodeLive(root->getId()), "parent root must remain live");
    }

    void test_remove_during_root_traversal_is_deferred_and_flushed()
    {
        ui::NodeTree tree;
        ui::Node *first = tree.attachRoot(0, makeNode());
        ui::Node *second = tree.attachRoot(1, makeNode());
        const ui::Node::Id firstId = first->getId();

        int callbacks = 0;
        tree.forEachRoot(
            [&](ui::Node &node)
            {
                ++callbacks;
                if (&node == first)
                {
                    tree.removeRoot(&node);
                    expect(tree.findNode(firstId) == first,
                           "root removal must be deferred while traversal is active");
                }
                return false;
            });

        expect(callbacks == 2, "root traversal must use a stable snapshot");
        expect(tree.findNode(firstId) == nullptr, "deferred root removal must flush after traversal");
        expect(tree.rootsCount() == 1, "one root must remain after deferred removal");
        expect(tree.getRoot(0) == second, "remaining root must stay accessible");
    }

    void test_attach_during_root_traversal_is_deferred_and_flushed()
    {
        ui::NodeTree tree;
        ui::Node *first = tree.attachRoot(0, makeNode());
        ui::Node *second = tree.attachRoot(1, makeNode());
        (void)second;

        ui::Node *attachedDuringTraversal = nullptr;
        int callbacks = 0;

        tree.forEachRoot(
            [&](ui::Node &node)
            {
                ++callbacks;
                if (&node == first)
                {
                    attachedDuringTraversal = tree.attachRoot(2, makeNode());
                    expect(attachedDuringTraversal == nullptr,
                           "attachRoot must defer while traversal is active");
                }
                return false;
            });

        expect(callbacks == 2, "new root must not enter the active traversal snapshot");
        expect(tree.rootsCount() == 3, "deferred root attach must flush after traversal");
        expect(tree.getRoot(2) != nullptr, "deferred root must be present after traversal");
    }

    void test_reverse_root_traversal_uses_reverse_snapshot()
    {
        ui::NodeTree tree;
        ui::Node *first = tree.attachRoot(0, makeNode());
        ui::Node *second = tree.attachRoot(1, makeNode());
        ui::Node *third = tree.attachRoot(2, makeNode());

        std::vector<ui::Node::Id> visited;
        tree.rForEachRoot(
            [&](ui::Node &node)
            {
                visited.push_back(node.getId());
                return false;
            });

        expect(visited.size() == 3, "reverse traversal must visit every root");
        expect(visited[0] == third->getId(), "reverse traversal must start at last root");
        expect(visited[1] == second->getId(), "reverse traversal must visit middle root second");
        expect(visited[2] == first->getId(), "reverse traversal must end at first root");
    }

    void test_nested_mutation_scope_does_not_flush_early()
    {
        ui::NodeTree tree;
        ui::Node *root = tree.attachRoot(0, makeNode());
        const ui::Node::Id id = root->getId();

        {
            ui::NodeTree::ScopedMutationGuard guard(tree);
            tree.removeRoot(root);
            expect(tree.findNode(id) == root, "mutation must remain deferred inside mutation scope");

            {
                ui::NodeTree::ScopedMutationGuard nestedGuard(tree);
                tree.attachRoot(0, makeNode());
                expect(tree.rootsCount() == 1, "nested mutation scope must not flush mutations");
            }

            expect(tree.rootsCount() == 1, "leaving nested scope must not flush outer mutations");
        }

        expect(tree.rootsCount() == 1, "leaving mutation scope alone must not drain the queue");
        expect(tree.findNode(id) == root, "removed root must remain live until explicit flush");

        tree.flushMutationQueue();

        expect(tree.rootsCount() == 1, "queued removal and attach must both be applied after flush");
        expect(tree.findNode(id) == nullptr, "removed root must be unregistered after flush");
        expect(tree.getRoot(0) != nullptr, "queued replacement root must be live after flush");
    }

    void test_layout_invalidation_is_deduplicated_per_root()
    {
        ui::NodeTree tree;
        ui::Node *root = tree.attachRoot(0, makePanel());

        tree.forEachLayoutQueue([](ui::Node &) {});

        auto first = makeNode();
        auto second = makeNode();
        tree.attachChild(*static_cast<ui::PanelNode *>(root), std::move(first), 0);
        tree.attachChild(*static_cast<ui::PanelNode *>(root), std::move(second), 1);

        std::vector<ui::Node::Id> queuedRoots;
        tree.forEachLayoutQueue([&](ui::Node &node)
        {
            queuedRoots.push_back(node.getId());
        });

        expect(queuedRoots.size() == 1, "multiple child mutations must queue the containing root once");
        expect(queuedRoots[0] == root->getId(), "layout queue must contain the containing root");
    }

    void test_layout_invalidation_is_requeued_after_previous_layout_pass()
    {
        LayoutFixture f;
        auto parent = makePanel();
        ui::PanelNode *parentPtr = parent.get();
        ui::Node *root = f.tree.attachRoot(0, std::move(parent));
        auto child = makeNode();
        ui::Node *childPtr = f.tree.attachChild(*parentPtr, std::move(child), 0);

        f.processLayout();

        parentPtr->removeChild(*childPtr);

        std::vector<ui::Node::Id> queuedRoots;
        f.tree.forEachLayoutQueue([&](ui::Node &node)
        {
            queuedRoots.push_back(node.getId());
        });

        expect(queuedRoots.size() == 1, "removing a child after a layout pass must requeue one root");
        expect(queuedRoots[0] == root->getId(), "removed-child invalidation must target the containing root");
    }

    void test_deferred_multiple_child_mutations_coalesce_layout_root()
    {
        ui::NodeTree tree;
        auto parent = makePanel();
        ui::PanelNode *parentPtr = parent.get();
        ui::Node *root = tree.attachRoot(0, std::move(parent));

        tree.forEachLayoutQueue([](ui::Node &) {});

        {
            ui::NodeTree::ScopedMutationGuard guard(tree);
            tree.attachChild(*parentPtr, makeNode(), 0);
            tree.attachChild(*parentPtr, makeNode(), 1);
            tree.attachChild(*parentPtr, makeNode(), 2);
        }

        expect(parentPtr->getChildCount() == 0, "queued child mutations must remain deferred until flush");

        tree.flushMutationQueue();

        expect(parentPtr->getChildCount() == 3, "all deferred child mutations must be applied");

        std::vector<ui::Node::Id> queuedRoots;
        tree.forEachLayoutQueue([&](ui::Node &node)
        {
            queuedRoots.push_back(node.getId());
        });

        expect(queuedRoots.size() == 1, "deferred child mutations must coalesce to one layout root");
        expect(queuedRoots[0] == root->getId(), "coalesced layout root must be the containing root");
    }

    void test_reentrant_remove_child_during_unmount_is_safe()
    {
        ui::NodeTree tree;
        auto parent = makePanel();
        ui::PanelNode *parentPtr = parent.get();
        tree.attachRoot(0, std::move(parent));

        int unmountCount = 0;
        auto child = std::make_unique<ReentrantUnmountNode>(parentPtr, &unmountCount);
        ReentrantUnmountNode *childPtr = child.get();
        const ui::Node::Id childId = childPtr->getId();

        tree.attachChild(*parentPtr, std::move(child), 0);
        tree.removeChild(*parentPtr, *childPtr);

        expect(unmountCount == 1, "reentrant removal must not invoke onUnmount twice");
        expect(parentPtr->getChildCount() == 0, "child must remain detached after reentrant removal");
        expect(tree.findNode(childId) == nullptr, "reentrantly removed child must be unregistered");
    }

    void test_on_mount_self_removal_is_safe()
    {
        ui::NodeTree tree;
        auto parent = makePanel();
        ui::PanelNode *parentPtr = parent.get();
        tree.attachRoot(0, std::move(parent));

        int mountCount = 0;
        auto child = std::make_unique<MountMutationNode>(parentPtr, &mountCount, true);
        MountMutationNode *childPtr = child.get();
        const ui::Node::Id childId = childPtr->getId();

        tree.attachChild(*parentPtr, std::move(child), 0);

        expect(mountCount == 1, "self-removing node must mount exactly once");
        expect(parentPtr->getChildCount() == 0, "self-removing node must be detached after mount flush");
        expect(tree.findNode(childId) == nullptr, "self-removing node must be unregistered after mount");
    }

    void test_parent_unmount_does_not_unmount_already_unmounted_child_twice()
    {
        ui::NodeTree tree;
        auto parent = std::make_unique<ParentUnmountNode>(new int(0));
        ParentUnmountNode *parentPtr = parent.get();
        tree.attachRoot(0, std::move(parent));

        int childUnmountCount = 0;
        auto child = std::make_unique<ReentrantUnmountNode>(parentPtr, &childUnmountCount);
        ReentrantUnmountNode *childPtr = child.get();
        parentPtr->setChildToRemove(childPtr);
        tree.attachChild(*parentPtr, std::move(child), 0);

        tree.removeRoot(parentPtr);

        expect(childUnmountCount == 1,
               "post-order unmount must not allow parent cleanup to unmount the child twice");
    }

    void test_update_self_removal_is_safe()
    {
        ui::NodeTree tree;
        int updateCount = 0;
        auto node = std::make_unique<UpdateMutationNode>(
            &tree,
            &updateCount,
            UpdateMutationNode::Action::RemoveSelf);
        const ui::Node::Id id = node->getId();
        tree.attachRoot(0, std::move(node));

        tree.update(1.0f / 60.0f);

        expect(updateCount == 1, "self-removing root must update exactly once");
        expect(tree.findNode(id) == nullptr, "self-removing root must be gone after update flush");
        expect(tree.rootsCount() == 0, "self-removing root must be removed after update");
    }

    void test_update_added_root_starts_next_frame()
    {
        ui::NodeTree tree;
        int firstUpdateCount = 0;
        int addedUpdateCount = 0;

        auto node = std::make_unique<UpdateMutationNode>(
            &tree,
            &firstUpdateCount,
            UpdateMutationNode::Action::AddRoot,
            &addedUpdateCount);
        UpdateMutationNode *nodePtr = node.get();
        tree.attachRoot(0, std::move(node));

        tree.update(1.0f / 60.0f);

        expect(firstUpdateCount == 1, "existing root must update in the current frame");
        expect(addedUpdateCount == 0, "root added during update must not update in the current frame");
        expect(tree.rootsCount() == 2, "deferred root must exist after update flush");
        expect(nodePtr->addedNodeId() != 0, "adder node must record the deferred root id");
        expect(tree.findNode(nodePtr->addedNodeId()) != nullptr,
               "deferred root must be live after the update flush");

        tree.update(1.0f / 60.0f);

        expect(firstUpdateCount == 2, "existing root must continue updating on the next frame");
        expect(addedUpdateCount == 1, "deferred root must start updating on the next frame");
    }

    void test_hit_test_prefers_deepest_and_topmost_visible_node()
    {
        LayoutFixture f;

        auto parent = std::make_unique<ui::StackPanelNode>();
        parent->setSize(ui::LayoutSizeValue::fixed(100.0f, 100.0f));
        ui::StackPanelNode *parentPtr = parent.get();
        f.tree.attachRoot(0, std::move(parent));

        auto firstChild = makeNode(60.0f, 60.0f);
        firstChild->setPosition({10.0f, 10.0f});
        firstChild->setPositionMode(ui::PositionMode::Absolute);
        ui::Node *firstChildPtr = f.tree.attachChild(*parentPtr, std::move(firstChild), 0);

        auto secondChild = makeNode(60.0f, 60.0f);
        secondChild->setPosition({20.0f, 20.0f});
        secondChild->setPositionMode(ui::PositionMode::Absolute);
        ui::Node *secondChildPtr = f.tree.attachChild(*parentPtr, std::move(secondChild), 1);

        f.processLayout();

        expect(
            f.tree.hitTest(30.0f, 30.0f) == secondChildPtr,
            "hitTest must prefer the topmost deepest overlapping child");

        expect(
            f.tree.hitTest(15.0f, 15.0f) == firstChildPtr,
            "hitTest must return the child covering a point when no later child overlaps it");

        expect(
            f.tree.hitTest(90.0f, 90.0f) == parentPtr,
            "hitTest must fall back to the parent when the point is outside children");

        expect(
            f.tree.hitTest(150.0f, 150.0f) == nullptr,
            "hitTest must return null outside the tree bounds");
    }
}

int main()
{
    try
    {
        test_attach_root_registers_node();
        test_remove_root_unregisters_node();
        test_attach_roots_preserves_requested_order();
        test_overlay_lifecycle_is_separate_from_roots();
        test_attach_and_remove_child_subtree_updates_live_nodes();
        test_remove_during_root_traversal_is_deferred_and_flushed();
        test_attach_during_root_traversal_is_deferred_and_flushed();
        test_reverse_root_traversal_uses_reverse_snapshot();
        test_nested_mutation_scope_does_not_flush_early();
        test_layout_invalidation_is_deduplicated_per_root();
        test_layout_invalidation_is_requeued_after_previous_layout_pass();
        test_deferred_multiple_child_mutations_coalesce_layout_root();
        test_reentrant_remove_child_during_unmount_is_safe();
        test_on_mount_self_removal_is_safe();
        test_parent_unmount_does_not_unmount_already_unmounted_child_twice();
        test_update_self_removal_is_safe();
        test_update_added_root_starts_next_frame();
        test_hit_test_prefers_deepest_and_topmost_visible_node();
    }
    catch (const TestFailure &failure)
    {
        std::cerr << "NodeTree tests failed: " << failure.message << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "NodeTree tests passed\n";
    return EXIT_SUCCESS;
}