#pragma once

#include "node.hpp"
#include "ui_framework/types.hpp"

namespace ui
{

    class Node;

    class PanelNode : public Node
    {
    public:
        PanelNode();
        ~PanelNode() override;

        Node *add(std::unique_ptr<Node> child, size_t index);
        void remove(Node &child);

        size_t childCount() const noexcept;
        bool hasChildren() const noexcept;

        Node *getChildAt(size_t index) noexcept;
        const Node *getChildAt(size_t index) const noexcept;

        void forEachChild(const std::function<bool(Node &)> &cb);
        void rForEachChild(const std::function<bool(Node &)> &cb);

        Node *getVisibleChild(size_t visibleIndex) const noexcept override;

        size_t visibleChildCount() const noexcept;
        size_t visibleChildIndexAt(size_t childIndex) const noexcept;

    protected:
        bool canAttach(const Node &child) const noexcept;
        bool isAncestorOf(const Node *node) const noexcept;

    private:
        Node *attachLocal(std::unique_ptr<Node> child, size_t index);
        std::unique_ptr<Node> detachLocal(Node &child);

        void forEachChildImpl(
            const std::function<bool(Node &)> &cb,
            bool reverse);

        std::vector<std::unique_ptr<Node>> children_;

        friend class NodeTree;
    };

}
