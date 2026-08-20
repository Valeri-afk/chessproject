#include "ui_framework/core/panelnode.hpp"

#include "nodetree.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>

namespace
{

    float finiteOrZero(float value) noexcept
    {
        return std::isfinite(value) ? value : 0.0f;
    }

    ui::LayoutSize sanitizeSize(ui::LayoutSize size) noexcept
    {
        return {
            finiteOrZero(size.width),
            finiteOrZero(size.height)};
    }

    int clampToInt(float value) noexcept
    {
        if (!std::isfinite(value))
            return 0;

        if (value <= static_cast<float>(std::numeric_limits<int>::min()))
            return std::numeric_limits<int>::min();

        if (value >= static_cast<float>(std::numeric_limits<int>::max()))
            return std::numeric_limits<int>::max();

        return static_cast<int>(value);
    }

    SDL_Rect toSDLRect(const ui::Node &node)
    {
        const ui::LayoutPosition position = node.getActualPosition();
        const ui::LayoutSize size = node.getActualSize();

        const float safeX = finiteOrZero(position.x);
        const float safeY = finiteOrZero(position.y);
        const float safeWidth = std::max(0.0f, finiteOrZero(size.width));
        const float safeHeight = std::max(0.0f, finiteOrZero(size.height));

        const int x = clampToInt(std::floor(safeX));
        const int y = clampToInt(std::floor(safeY));

        const int right =
            clampToInt(std::ceil(safeX + safeWidth));

        const int bottom =
            clampToInt(std::ceil(safeY + safeHeight));

        return {
            x,
            y,
            std::max(0, right - x),
            std::max(0, bottom - y)};
    }

    bool getRenderClipRect(SDL_Renderer *renderer, SDL_Rect *rect)
    {
        return SDL_GetRenderClipRect(renderer, rect) == 0;
    }

    SDL_Rect intersectRects(const SDL_Rect &a, const SDL_Rect &b)
    {
        const int left = std::max(a.x, b.x);
        const int top = std::max(a.y, b.y);

        const int right = std::min(a.x + a.w, b.x + b.w);
        const int bottom = std::min(a.y + a.h, b.y + b.h);

        return {
            left,
            top,
            std::max(0, right - left),
            std::max(0, bottom - top)};
    }

    std::optional<std::size_t> findChildIndexById(
        const std::vector<std::unique_ptr<ui::Node>> &children,
        ui::Node::Id id)
    {
        for (std::size_t i = 0; i < children.size(); ++i)
        {
            if (children[i] && children[i]->id() == id)
            {
                return i;
            }
        }

        return std::nullopt;
    }

    class RendererStateScope
    {
    public:
        explicit RendererStateScope(SDL_Renderer *renderer) noexcept
            : renderer_(renderer)
        {
            if (!renderer_)
                return;

            hadPreviousClip_ = getRenderClipRect(renderer_, &previousClip_);

            SDL_GetRenderDrawBlendMode(renderer_, &previousBlendMode_);

            SDL_GetRenderDrawColor(
                renderer_,
                &previousR_,
                &previousG_,
                &previousB_,
                &previousA_);
        }

        ~RendererStateScope()
        {
            if (!renderer_)
                return;

            SDL_SetRenderDrawBlendMode(renderer_, previousBlendMode_);

            SDL_SetRenderDrawColor(
                renderer_,
                previousR_,
                previousG_,
                previousB_,
                previousA_);

            if (hadPreviousClip_)
            {
                SDL_SetRenderClipRect(renderer_, &previousClip_);
            }
            else
            {
                SDL_SetRenderClipRect(renderer_, nullptr);
            }
        }

        RendererStateScope(const RendererStateScope &) = delete;
        RendererStateScope &operator=(const RendererStateScope &) = delete;

    private:
        SDL_Renderer *renderer_ = nullptr;

        bool hadPreviousClip_ = false;
        SDL_Rect previousClip_{};

        SDL_BlendMode previousBlendMode_ = SDL_BLENDMODE_BLEND;

        Uint8 previousR_ = 255;
        Uint8 previousG_ = 255;
        Uint8 previousB_ = 255;
        Uint8 previousA_ = 255;
    };

}

namespace ui
{

    PanelNode::PanelNode() = default;

    PanelNode::~PanelNode() = default;

    Node *PanelNode::add(std::unique_ptr<Node> child, size_t index)
    {
        if (!child)
            return nullptr;

        if (owner_)
        {
            return owner_->attachChild(*this, std::move(child), index);
        }

        return attachLocal(std::move(child), index);
    }

    void PanelNode::remove(Node &child)
    {
        if (owner_)
        {
            owner_->removeChild(*this, child);
            return;
        }

        detachLocal(child).reset();
    }

    size_t PanelNode::childCount() const noexcept
    {
        return children_.size();
    }

    bool PanelNode::hasChildren() const noexcept
    {
        return !children_.empty();
    }

    Node *PanelNode::getChildAt(size_t index) noexcept
    {
        return index < children_.size() ? children_[index].get() : nullptr;
    }

    const Node *PanelNode::getChildAt(size_t index) const noexcept
    {
        return index < children_.size() ? children_[index].get() : nullptr;
    }

    void PanelNode::forEachChild(
        const std::function<bool(Node &)> &cb)
    {
        if (!cb)
            return;

        forEachChildImpl(cb, false);
    }

    void PanelNode::rForEachChild(
        const std::function<bool(Node &)> &cb)
    {
        if (!cb)
            return;

        forEachChildImpl(cb, true);
    }

    void PanelNode::forEachChildImpl(
        const std::function<bool(Node &)> &cb,
        bool reverse)
    {
        if (!cb)
            return;

        auto iterate = [&]()
        {
            std::vector<Node::Id> snapshot;
            snapshot.reserve(children_.size());

            if (reverse)
            {
                for (auto it = children_.rbegin();
                     it != children_.rend();
                     ++it)
                {
                    if (*it)
                        snapshot.push_back((*it)->id());
                }
            }
            else
            {
                for (const auto &child : children_)
                {
                    if (child)
                        snapshot.push_back(child->id());
                }
            }

            for (Node::Id id : snapshot)
            {
                const auto index =
                    findChildIndexById(children_, id);

                if (!index)
                    continue;

                Node *child = children_[*index].get();

                if (!child || child->id() != id)
                    continue;

                if (cb(*child))
                    return;
            }
        };

        NodeTree *tree = owner_;

        if (!tree)
        {
            iterate();
            return;
        }

        {
            NodeTree::ScopedMutationGuard guard(*tree);
            iterate();
        }

        tree->flushMutationQueue();
    }

    Node *PanelNode::getVisibleChild(
        size_t visibleIndex) const noexcept
    {
        size_t current = 0;

        for (const auto &child : children_)
        {
            if (!child || !child->isVisible())
                continue;

            if (current == visibleIndex)
                return child.get();

            ++current;
        }

        return nullptr;
    }

    size_t PanelNode::visibleChildCount() const noexcept
    {
        size_t result = 0;

        for (const auto &child : children_)
        {
            if (child && child->isVisible())
                ++result;
        }

        return result;
    }

    size_t PanelNode::visibleChildIndexAt(
        size_t childIndex) const noexcept
    {
        if (childIndex >= children_.size())
            return visibleChildCount();

        size_t result = 0;

        for (size_t i = 0; i < childIndex; ++i)
        {
            if (children_[i] && children_[i]->isVisible())
                ++result;
        }

        return result;
    }

    Node *PanelNode::attachLocal(
        std::unique_ptr<Node> child,
        size_t index)
    {
        if (!child)
            return nullptr;

        if (!canAttach(*child))
            return nullptr;

        if (index > children_.size())
            index = children_.size();

        children_.insert(
            children_.begin() + static_cast<std::ptrdiff_t>(index),
            std::move(child));

        children_[index]->parent_ = this;

        return children_[index].get();
    }

    std::unique_ptr<Node> PanelNode::detachLocal(Node &child)
    {
        for (auto it = children_.begin(); it != children_.end(); ++it)
        {
            if (it->get() == &child)
            {
                auto result = std::move(*it);
                children_.erase(it);

                result->parent_ = nullptr;

                return result;
            }
        }

        return nullptr;
    }

    bool PanelNode::canAttach(const Node &child) const noexcept
    {
        if (isAncestorOf(&child))
        {
            SDL_LogError(
                SDL_LOG_CATEGORY_APPLICATION,
                "PanelNode hierarchy cycle detected.");

            return false;
        }

        if (child.parent_)
        {
            SDL_LogError(
                SDL_LOG_CATEGORY_APPLICATION,
                "PanelNode child already has parent.");

            return false;
        }

        if (child.owner_)
        {
            SDL_LogError(
                SDL_LOG_CATEGORY_APPLICATION,
                "PanelNode child already belongs to a tree.");

            return false;
        }

        return true;
    }

    bool PanelNode::isAncestorOf(const Node *node) const noexcept
    {
        const Node *current = node;

        while (current)
        {
            if (current == this)
                return true;

            current = current->parent_;
        }

        return false;
    }

}
