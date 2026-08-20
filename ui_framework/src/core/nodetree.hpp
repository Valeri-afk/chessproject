#pragma once

#include <cstddef>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "ui_framework/core/node.hpp"
#include "ui_framework/core/panelnode.hpp"
#include "ui_framework/core/rendering_state.hpp"

namespace ui
{

    class NodeTree
    {
    public:
        using NodeId = Node::Id;

        class ScopedMutationGuard
        {
        public:
            explicit ScopedMutationGuard(NodeTree &tree) noexcept;
            ~ScopedMutationGuard();

            ScopedMutationGuard(const ScopedMutationGuard &) = delete;
            ScopedMutationGuard &operator=(const ScopedMutationGuard &) = delete;

        private:
            NodeTree *tree_ = nullptr;
        };

        NodeTree() = default;

        NodeTree(const NodeTree &) = delete;
        NodeTree &operator=(const NodeTree &) = delete;

        Node *attachRoot(size_t index, std::unique_ptr<Node> node);
        Node *attachOverlay(size_t index, std::unique_ptr<Node> node);

        void removeRoot(Node *node);
        void removeOverlay(Node *node);

        Node *attachChild(
            PanelNode &parent,
            std::unique_ptr<Node> child,
            size_t index = 0);

        void removeChild(
            PanelNode &parent,
            Node &child);

        Node *hitTest(
            float x,
            float y,
            const Node *modalRoot = nullptr);

        Node *findNode(NodeId id);
        const Node *findNode(NodeId id) const;

        bool isNodeLive(NodeId id) const;

        bool isRoot(const Node *node) const noexcept;
        bool isOverlay(const Node *node) const noexcept;

        size_t rootsCount() const noexcept
        {
            return roots_.size();
        }

        size_t overlaysCount() const noexcept
        {
            return overlays_.size();
        }

        Node *getRoot(size_t index) noexcept
        {
            return index < roots_.size() ? roots_[index].get() : nullptr;
        }

        Node *getOverlay(size_t index) noexcept
        {
            return index < overlays_.size() ? overlays_[index].get() : nullptr;
        }

        const Node *getRoot(size_t index) const noexcept
        {
            return index < roots_.size() ? roots_[index].get() : nullptr;
        }

        const Node *getOverlay(size_t index) const noexcept
        {
            return index < overlays_.size() ? overlays_[index].get() : nullptr;
        }

        template <typename Callback>
        void forEachRoot(Callback &&cb)
        {
            {
                ScopedMutationGuard guard(*this);

                std::vector<NodeId> ids;
                ids.reserve(roots_.size());

                for (const auto &root : roots_)
                {
                    if (root)
                        ids.push_back(root->id());
                }

                for (NodeId id : ids)
                {
                    if (Node *node = findNode(id))
                    {
                        if (cb(*node))
                            break;
                    }
                }
            }

            flushMutationQueue();
        }

        template <typename Callback>
        void rForEachRoot(Callback &&cb)
        {
            {
                ScopedMutationGuard guard(*this);

                std::vector<NodeId> ids;
                ids.reserve(roots_.size());

                for (auto it = roots_.rbegin(); it != roots_.rend(); ++it)
                {
                    if (*it)
                        ids.push_back((*it)->id());
                }

                for (NodeId id : ids)
                {
                    if (Node *node = findNode(id))
                    {
                        if (cb(*node))
                            break;
                    }
                }
            }

            flushMutationQueue();
        }

        template <typename Callback>
        void forEachOverlay(Callback &&cb)
        {
            {
                ScopedMutationGuard guard(*this);

                std::vector<NodeId> ids;
                ids.reserve(overlays_.size());

                for (const auto &overlay : overlays_)
                {
                    if (overlay)
                        ids.push_back(overlay->id());
                }

                for (NodeId id : ids)
                {
                    if (Node *node = findNode(id))
                    {
                        if (cb(*node))
                            break;
                    }
                }
            }

            flushMutationQueue();
        }

        template <typename Callback>
        void rForEachOverlay(Callback &&cb)
        {
            {
                ScopedMutationGuard guard(*this);

                std::vector<NodeId> ids;
                ids.reserve(overlays_.size());

                for (auto it = overlays_.rbegin(); it != overlays_.rend(); ++it)
                {
                    if (*it)
                        ids.push_back((*it)->id());
                }

                for (NodeId id : ids)
                {
                    if (Node *node = findNode(id))
                    {
                        if (cb(*node))
                            break;
                    }
                }
            }

            flushMutationQueue();
        }

        template <typename Fn>
        void enqueueNodeMutation(Node &, Fn &&fn);

        void forEachLayoutQueue(const std::function<void(Node &)> &cb);
        void insertLayoutQueueById(NodeId id);

        void flushMutationQueue();
        void requestFullLayout();

        void update(float dt);
        void draw(
            SDL_Renderer *renderer,
            std::optional<NodeId> topModalId = std::nullopt);

        bool isDescendant(
            const Node *node,
            const Node *ancestor) const noexcept;

        void drawSubtree(
            Node &node,
            SDL_Renderer *renderer);

        Node *hitTestSubtree(
            Node &node,
            float x,
            float y);

    private:
        struct Mutation
        {
            virtual ~Mutation() = default;
            virtual void operator()() = 0;
        };

        template <typename Fn>
        struct MutationImpl final : Mutation
        {
            explicit MutationImpl(Fn &&function)
                : function(std::move(function))
            {
            }

            void operator()() override
            {
                function();
            }

            Fn function;
        };

        enum class TraversalResult
        {
            Continue,
            SkipChildren,
            Stop
        };

        using TraversalCallback =
            std::function<TraversalResult(Node &)>;

        TraversalResult traversePreOrder(
            Node &node,
            const TraversalCallback &callback,
            bool reverse = false);

        TraversalResult traversePostOrder(
            Node &node,
            const TraversalCallback &callback,
            bool reverse = false);

        std::vector<std::unique_ptr<Node>> roots_;
        std::vector<std::unique_ptr<Node>> overlays_;
        std::vector<std::unique_ptr<Mutation>> mutationQueue_;
        std::deque<NodeId> layoutQueue_;
        std::unordered_set<NodeId> layoutQueueSet_;
        std::unordered_map<NodeId, Node *> liveNodes_;
        std::size_t mutationDepth_ = 0;

        template <typename Fn>
        void enqueueMutation(Fn &&fn);

        Node *attachToContainer(
            size_t index,
            std::unique_ptr<Node> node,
            std::vector<std::unique_ptr<Node>> &container);

        void removeFromContainer(
            NodeId id,
            std::vector<std::unique_ptr<Node>> &container);

        PanelNode *resolveLivePanelParent(Node &parent);
        PanelNode *resolveLivePanelNode(NodeId id);
        void flushMutationQueueAndInsertLayout(NodeId id);

        Node *attachInternal(
            size_t index,
            std::unique_ptr<Node> node,
            std::vector<std::unique_ptr<Node>> &container);

        void removeInternal(
            NodeId id,
            std::vector<std::unique_ptr<Node>> &container);

        Node *attachChildInternal(
            PanelNode &parent,
            std::unique_ptr<Node> child,
            size_t index);

        void removeChildInternal(
            PanelNode &parent,
            Node &child);

        void insertLayoutQueue(Node *node);
        void drainMutationQueue();
        void enterMutationScope() noexcept;
        void leaveMutationScope() noexcept;
        bool isMutationScopeActive() const noexcept;

        void assertSubtreeOwner(const Node &node, NodeTree *owner) const;
        void assertSubtreeLive(const Node &node) const;
        void assertLiveSubtree(NodeId id, NodeTree *owner) const;
        void registerSubtree(Node &node);
        void unregisterSubtree(Node &node);
        void registerNode(Node &node);
        void unregisterNode(Node &node);
        void mountSubtree(Node &root);
        void unmountSubtree(Node &root);
        void setSubtreeOwner(Node &root, NodeTree *owner);

        bool containsNodeInContainer(
            const std::vector<std::unique_ptr<Node>> &container,
            NodeId id) const noexcept;

        friend class Node;
        friend class PanelNode;
    };

    template <typename Fn>
    void NodeTree::enqueueMutation(Fn &&fn)
    {
        using DecayedFn = std::decay_t<Fn>;

        mutationQueue_.push_back(
            std::make_unique<MutationImpl<DecayedFn>>(
                std::forward<Fn>(fn)));
    }

    template <typename Fn>
    void NodeTree::enqueueNodeMutation(Node &, Fn &&fn)
    {
        enqueueMutation(std::forward<Fn>(fn));
    }

}
