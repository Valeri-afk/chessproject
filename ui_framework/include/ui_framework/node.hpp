#pragma once

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <typeindex>
#include <utility>
#include <vector>

#include <SDL3/SDL.h>

#include "ui_framework/event_types.hpp"
#include "ui_framework/layout_context.hpp"
#include "ui_framework/types.hpp"

namespace ui
{
    class NodeTree;
    class PanelNode;
    class LayoutSystem;
    class EventDispatcher;

    class Node
    {
    public:
        using Id = std::uint64_t;
        using EventHandlerId = std::uint64_t;
        using CoordinateTransform = std::function<LayoutPosition(const Node &, const LayoutPosition &)>;
        class ScopedCoordinateTransform
        {
        public:
            explicit ScopedCoordinateTransform(CoordinateTransform transform) : previous_(coordinateTransform()) { coordinateTransform() = std::move(transform); }
            ~ScopedCoordinateTransform() { coordinateTransform() = std::move(previous_); }
            ScopedCoordinateTransform(const ScopedCoordinateTransform &) = delete;
            ScopedCoordinateTransform &operator=(const ScopedCoordinateTransform &) = delete;
        private:
            CoordinateTransform previous_;
        };
        Node();
        virtual ~Node();
        Node(const Node &) = delete;
        Node &operator=(const Node &) = delete;
        Id getId() const noexcept;
        Node *getParent() const noexcept;
        void setVisible(bool visible);
        bool isVisible() const noexcept;
        void setEnabled(bool enabled) noexcept;
        bool isEnabled() const noexcept;
        void setFocusable(bool focusable) noexcept;
        bool isFocusable() const noexcept;
        void setCapturable(bool capturable) noexcept;
        bool isCapturable() const noexcept;
        void setPosition(const LayoutPosition &position);
        LayoutPosition getPosition() const noexcept;
        LayoutSize getDesiredSize() const noexcept;
        void setPositionMode(PositionMode positionMode);
        PositionMode getPositionMode() const noexcept;
        void setSize(const LayoutSizeValue &size);
        LayoutSizeValue getSize() const noexcept;
        void setMinSize(const LayoutSize &size);
        void setMaxSize(const LayoutSize &size);
        void setMinWidth(float width);
        void setMinHeight(float height);
        void setMaxWidth(float width);
        void setMaxHeight(float height);
        LayoutSize getMinSize() const noexcept;
        LayoutSize getMaxSize() const noexcept;
        float getMinWidth() const noexcept;
        float getMinHeight() const noexcept;
        float getMaxWidth() const noexcept;
        float getMaxHeight() const noexcept;
        void setPadding(const Padding &padding);
        Padding getPadding() const noexcept;
        void setLeftPadding(float value);
        void setRightPadding(float value);
        void setTopPadding(float value);
        void setBottomPadding(float value);
        void setBorder(const Border &border);
        Border getBorder() const noexcept;
        void setLeftBorder(float value);
        void setRightBorder(float value);
        void setTopBorder(float value);
        void setBottomBorder(float value);
        void setOverflow(Overflow overflow);
        Overflow getOverflow() const noexcept;
        void setClipToBounds(bool clip) noexcept;
        bool getClipToBounds() const noexcept;
        LayoutPosition getActualPosition() const noexcept;
        LayoutSize getActualSize() const noexcept;
        virtual Node *getVisibleChild(size_t visibleIndex) const noexcept;

        template <typename Event>
        EventHandlerId on(std::function<void(Event &, Node &)> handler)
        {
            if (!handler)
                return 0;
            const EventHandlerId token = nextEventHandlerId();
            eventHandlers_.push_back(
                EventHandlerRecord{token, std::type_index(typeid(Event)),
                    [handler = std::move(handler)](UIEvent &event, Node &node)
                    {
                        handler(static_cast<Event &>(event), node);
                    }});
            return token;
        }

        template <typename Event>
        void removeEventHandler(EventHandlerId handlerId)
        {
            const std::type_index eventType(typeid(Event));
            eventHandlers_.erase(
                std::remove_if(eventHandlers_.begin(), eventHandlers_.end(),
                    [eventType, handlerId](const EventHandlerRecord &record)
                    {
                        return record.eventType == eventType && record.token == handlerId;
                    }), eventHandlers_.end());
        }

        template <typename Event>
        void clearEventHandlers()
        {
            const std::type_index eventType(typeid(Event));
            eventHandlers_.erase(
                std::remove_if(eventHandlers_.begin(), eventHandlers_.end(),
                    [eventType](const EventHandlerRecord &record)
                    {
                        return record.eventType == eventType;
                    }), eventHandlers_.end());
        }

    protected:
        template <typename Event> EventHandlerId addHandler(std::function<void(Event &, Node &)> handler) { return on<Event>(std::move(handler)); }
        template <typename Event> void removeHandler(EventHandlerId handlerId) { removeEventHandler<Event>(handlerId); }
        template <typename Event> void clearHandlers() { clearEventHandlers<Event>(); }
        void invalidateLayout() noexcept;
        virtual void update(float dt) {}
        virtual void draw(SDL_Renderer *renderer) {}
        virtual LayoutSize measure(const MeasureContext &context) const { return measureContent(context.availableContentSize); }
        virtual void arrange(const ArrangeContext &context) { arrangeContent(context.contentPosition, context.contentSize); }
        virtual LayoutSize measureContent(const LayoutSize &availableContent) const { (void)availableContent; return {}; }
        virtual void arrangeContent(const LayoutPosition &contentPosition, const LayoutSize &contentSize) { (void)contentPosition; (void)contentSize; }
        virtual void onMount() {}
        virtual void onUnmount() {}
        virtual Node *hitTest(float x, float y) noexcept;

    private:
        struct EventHandlerRecord
        {
            EventHandlerId token = 0;
            std::type_index eventType = typeid(void);
            std::function<void(UIEvent &, Node &)> callback;
        };
        static CoordinateTransform &coordinateTransform() { static thread_local CoordinateTransform transform; return transform; }
        static EventHandlerId nextEventHandlerId() noexcept
        {
            static std::atomic<EventHandlerId> next{1};
            return next.fetch_add(1, std::memory_order_relaxed);
        }
        Node *parent_ = nullptr;
        NodeTree *owner_ = nullptr;
        LayoutSizeValue size_{};
        LayoutPosition position_{};
        PositionMode positionMode_ = PositionMode::Layout;
        LayoutPosition actualPosition_;
        LayoutSize actualSize_;
        LayoutSize desiredSize_;
        LayoutSize minSize_{};
        LayoutSize maxSize_{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
        Padding padding_;
        Border border_;
        Overflow overflow_ = Overflow::VISIBLE;
        bool visible_ = true;
        bool enabled_ = true;
        bool focusable_ = false;
        bool capturable_ = false;
        std::vector<EventHandlerRecord> eventHandlers_;
        const Id id_ = nextId();
        static Id nextId() noexcept { static std::atomic<Id> next{1}; return next.fetch_add(1); }
        LayoutSize clampSize(LayoutSize size, LayoutSize minSize, LayoutSize maxSize) const;
        template <typename Event>
        void dispatchEvent(Event &event, NodeTree &nodeTree)
        {
            dispatchEventImpl(static_cast<UIEvent &>(event), std::type_index(typeid(Event)), nodeTree);
        }
        void dispatchEventImpl(UIEvent &event, const std::type_index &eventType, NodeTree &nodeTree);
        friend class NodeTree;
        friend class PanelNode;
        friend class LayoutSystem;
        friend class EventDispatcher;
    };
}
