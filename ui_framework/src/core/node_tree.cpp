#include "node_tree.hpp"
#include "rendering_state.hpp"

#include <algorithm>
#include <optional>
#include <utility>

namespace
{
    static std::optional<size_t> findChildIndexById(
        const std::vector<std::unique_ptr<ui::Node>> &children,
        ui::Node::Id id)
    {
        for (size_t i = 0; i < children.size(); ++i)
        {
            if (children[i] && children[i]->getId() == id)
                return i;
        }
        return std::nullopt;
    }
}

namespace ui
{
    NodeTree::ScopedMutationGuard::ScopedMutationGuard(NodeTree &tree) noexcept : tree_(&tree) { tree_->enterMutationScope(); }
    NodeTree::ScopedMutationGuard::~ScopedMutationGuard() { if (tree_) tree_->leaveMutationScope(); }
    void NodeTree::enterMutationScope() noexcept { ++mutationDepth_; }
    void NodeTree::leaveMutationScope() noexcept { SDL_assert(mutationDepth_ > 0); if (mutationDepth_ > 0) --mutationDepth_; }
    bool NodeTree::isMutationScopeActive() const noexcept { return mutationDepth_ > 0; }
    void NodeTree::flushMutationQueue() { if (isMutationScopeActive()) return; ScopedMutationGuard guard(*this); drainMutationQueue(); }
    void NodeTree::drainMutationQueue()
    {
        while (!mutationQueue_.empty())
        {
            std::vector<std::unique_ptr<Mutation>> queue;
            queue.swap(mutationQueue_);
            for (auto &mutation : queue) if (mutation) (*mutation)();
        }
    }
    void NodeTree::assertSubtreeOwner(const Node &node, NodeTree *owner) const
    {
#ifndef NDEBUG
        SDL_assert(node.owner_ == owner);
        if (const PanelNode *panel = dynamic_cast<const PanelNode *>(&node)) for (const auto &child : panel->children_) if (child) assertSubtreeOwner(*child, owner);
#else
        (void)node; (void)owner;
#endif
    }
    void NodeTree::assertSubtreeLive(const Node &node) const
    {
#ifndef NDEBUG
        SDL_assert(findNode(node.getId()) == &node);
        if (const PanelNode *panel = dynamic_cast<const PanelNode *>(&node)) for (const auto &child : panel->children_) if (child) assertSubtreeLive(*child);
#else
        (void)node;
#endif
    }
    void NodeTree::assertLiveSubtree(NodeId id, NodeTree *owner) const
    {
#ifndef NDEBUG
        if (const Node *live = findNode(id)) { assertSubtreeOwner(*live, owner); assertSubtreeLive(*live); }
#else
        (void)id; (void)owner;
#endif
    }
    bool NodeTree::containsNodeInContainer(const std::vector<std::unique_ptr<Node>> &container, NodeId id) const noexcept
    {
        return std::any_of(container.begin(), container.end(), [id](const std::unique_ptr<Node> &node) { return node && node->getId() == id; });
    }
    PanelNode *NodeTree::resolveLivePanelNode(NodeId id)
    {
        Node *liveNode = findNode(id); if (!liveNode) return nullptr;
        PanelNode *panel = dynamic_cast<PanelNode *>(liveNode);
        if (!panel) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "NodeTree: parent is not a PanelNode."); SDL_assert(false); return nullptr; }
        return panel;
    }
    PanelNode *NodeTree::resolveLivePanelParent(Node &parent) { return resolveLivePanelNode(parent.getId()); }
    void NodeTree::registerNode(Node &node) { liveNodes_[node.getId()] = &node; }
    void NodeTree::unregisterNode(Node &node) { liveNodes_.erase(node.getId()); }
    void NodeTree::registerSubtree(Node &root) { traversePreOrder(root, [this](Node &node) { registerNode(node); return TraversalResult::Continue; }); }
    void NodeTree::unregisterSubtree(Node &root) { traversePostOrder(root, [this](Node &node) { unregisterNode(node); return TraversalResult::Continue; }); }
    void NodeTree::setSubtreeOwner(Node &root, NodeTree *owner) { traversePreOrder(root, [owner](Node &node) { node.owner_ = owner; return TraversalResult::Continue; }); }
    void NodeTree::attachOwnedSubtree(Node &root, NodeId rootId) { setSubtreeOwner(root, this); registerSubtree(root); if (findNode(rootId)) mountSubtree(root); }
    void NodeTree::detachOwnedSubtree(Node &root, NodeId rootId) { unmountSubtree(root); if (findNode(rootId)) unregisterSubtree(root); setSubtreeOwner(root, nullptr); }
    Node *NodeTree::findNode(NodeId id) { auto it = liveNodes_.find(id); return it == liveNodes_.end() ? nullptr : it->second; }
    const Node *NodeTree::findNode(NodeId id) const { auto it = liveNodes_.find(id); return it == liveNodes_.end() ? nullptr : it->second; }
    bool NodeTree::isNodeLive(NodeId id) const { return liveNodes_.find(id) != liveNodes_.end(); }
    bool NodeTree::isRoot(const Node *node) const noexcept { return node && containsNodeInContainer(roots_, node->getId()); }
    bool NodeTree::isOverlay(const Node *node) const noexcept { return node && containsNodeInContainer(overlays_, node->getId()); }
    void NodeTree::requestFullLayout() { for (const auto &root : roots_) insertLayoutQueue(root.get()); for (const auto &overlay : overlays_) insertLayoutQueue(overlay.get()); }
    Node *NodeTree::attachRoot(size_t index, std::unique_ptr<Node> node) { return attachToContainer(index, std::move(node), roots_); }
    Node *NodeTree::attachOverlay(size_t index, std::unique_ptr<Node> node) { return attachToContainer(index, std::move(node), overlays_); }
    void NodeTree::removeRoot(Node *node) { if (node) removeFromContainer(node->getId(), roots_); }
    void NodeTree::removeOverlay(Node *node) { if (node) removeFromContainer(node->getId(), overlays_); }
    Node *NodeTree::attachChild(PanelNode &parent, std::unique_ptr<Node> child, size_t index)
    {
        if (isMutationScopeActive()) { const NodeId parentId = parent.getId(); if (!findNode(parentId)) return nullptr; enqueueMutation([this, parentId, child = std::move(child), index]() mutable { if (PanelNode *panelParent = resolveLivePanelNode(parentId)) attachChildInternal(*panelParent, std::move(child), index); }); return nullptr; }
        PanelNode *panelParent = resolveLivePanelParent(parent); return panelParent ? attachChildInternal(*panelParent, std::move(child), index) : nullptr;
    }
    void NodeTree::removeChild(PanelNode &parent, Node &child)
    {
        if (isMutationScopeActive()) { const NodeId parentId = parent.getId(); const NodeId childId = child.getId(); if (!findNode(parentId) || !findNode(childId)) return; enqueueMutation([this, parentId, childId] { if (PanelNode *panelParent = resolveLivePanelNode(parentId)) if (Node *liveChild = findNode(childId)) removeChildInternal(*panelParent, *liveChild); }); return; }
        if (PanelNode *panelParent = resolveLivePanelParent(parent)) removeChildInternal(*panelParent, child);
    }
    Node *NodeTree::attachToContainer(size_t index, std::unique_ptr<Node> node, std::vector<std::unique_ptr<Node>> &container)
    {
        if (isMutationScopeActive()) { enqueueMutation([this, index, node = std::move(node), &container]() mutable { attachInternal(index, std::move(node), container); }); return nullptr; }
        return attachInternal(index, std::move(node), container);
    }
    void NodeTree::removeFromContainer(NodeId id, std::vector<std::unique_ptr<Node>> &container)
    {
        if (isMutationScopeActive()) { enqueueMutation([this, id, &container] { removeInternal(id, container); }); return; }
        removeInternal(id, container);
    }
    void NodeTree::flushMutationQueueAndInsertLayout(NodeId id) { flushMutationQueue(); if (Node *liveNode = findNode(id)) insertLayoutQueue(liveNode); }
    Node *NodeTree::attachInternal(size_t index, std::unique_ptr<Node> node, std::vector<std::unique_ptr<Node>> &container)
    {
        if (!node) return nullptr;
        if (node->getParent() || node->owner_) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "NodeTree: node already belongs to a tree."); SDL_assert(false); return nullptr; }
        if (index > container.size()) index = container.size();
        Node *raw = node.get(); const NodeId nodeId = raw->getId();
        container.insert(container.begin() + static_cast<std::ptrdiff_t>(index), std::move(node));
        attachOwnedSubtree(*raw, nodeId); flushMutationQueueAndInsertLayout(nodeId); return findNode(nodeId);
    }
    void NodeTree::removeInternal(NodeId id, std::vector<std::unique_ptr<Node>> &container)
    {
        auto it = std::find_if(container.begin(), container.end(), [id](const std::unique_ptr<Node> &node) { return node && node->getId() == id; });
        if (it == container.end()) return; auto removed = std::move(*it); container.erase(it); detachOwnedSubtree(*removed, id);
    }
}
