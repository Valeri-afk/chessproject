#pragma once

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ui
{
    class Node;

    class EventHandlerStorage
    {
    public:
        using HandlerToken = std::uint64_t;

        template <typename Event>
        HandlerToken add(std::function<void(Event &, Node &)> handler);

        template <typename Event>
        void remove(HandlerToken token);

        template <typename Event>
        void clear();

        template <typename Event, typename Callback>
        bool forEachHandler(Callback &&callback);

    private:
        struct HandlerTableBase
        {
            virtual ~HandlerTableBase() = default;
        };

        template <typename Event>
        struct HandlerTable final : HandlerTableBase
        {
            using Handler = std::function<void(Event &, Node &)>;

            struct Entry
            {
                HandlerToken token{};
                Handler handler;
            };

            std::vector<Entry> entries;
        };

        static HandlerToken nextToken();

        std::unordered_map<
            std::type_index,
            std::unique_ptr<HandlerTableBase>>
            tables_;
    };
}

#include "event_handler_storage.inl"