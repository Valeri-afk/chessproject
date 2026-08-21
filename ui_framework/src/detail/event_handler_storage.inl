#pragma once

#include "event_handler_storage.hpp"

namespace ui
{

    inline EventHandlerStorage::HandlerToken
    EventHandlerStorage::nextToken()
    {
        static std::atomic<HandlerToken> next{1};
        return next.fetch_add(1, std::memory_order_relaxed);
    }

    template <typename Event>
    EventHandlerStorage::HandlerToken
    EventHandlerStorage::add(
        std::function<void(Event &, Node &)> handler)
    {
        if (!handler)
            return 0;

        auto it = tables_.find(typeid(Event));

        if (it == tables_.end())
        {
            auto table = std::make_unique<HandlerTable<Event>>();
            it = tables_.emplace(typeid(Event), std::move(table)).first;
        }

        auto *table =
            static_cast<HandlerTable<Event> *>(it->second.get());

        HandlerToken token = nextToken();

        table->entries.push_back({token,
                                  std::move(handler)});

        return token;
    }

    template <typename Event>
    void EventHandlerStorage::remove(HandlerToken token)
    {
        auto it = tables_.find(typeid(Event));

        if (it == tables_.end())
            return;

        auto *table =
            static_cast<HandlerTable<Event> *>(it->second.get());

        table->entries.erase(
            std::remove_if(
                table->entries.begin(),
                table->entries.end(),
                [token](const auto &e)
                {
                    return e.token == token;
                }),
            table->entries.end());

        if (table->entries.empty())
            tables_.erase(it);
    }

    template <typename Event>
    void EventHandlerStorage::clear()
    {
        tables_.erase(typeid(Event));
    }

    template <typename Event, typename Callback>
    bool EventHandlerStorage::forEachHandler(Callback &&callback)
    {
        auto it = tables_.find(typeid(Event));

        if (it == tables_.end())
            return true;

        auto *table =
            static_cast<HandlerTable<Event> *>(it->second.get());

        auto snapshot = std::vector<Entry>{};
        snapshot.reserve(table->entries.size());

        for (const auto &entry : table->entries)
        {
            snapshot.push_back({entry.token,
                                entry.handler});
        }

        for (auto &entry : snapshot)
        {
            callback(entry.handler);
        }

        return true;
    }

}