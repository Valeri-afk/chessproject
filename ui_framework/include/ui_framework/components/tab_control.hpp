#pragma once

#include <cstddef>
#include <functional>
#include <memory>

#include "ui_framework/components/tab_item.hpp"
#include "ui_framework/core/stackpanelnode.hpp"

namespace ui
{
    class TabControl : public StackPanelNode
    {
    public:
        using SelectionCallback = std::function<void(TabItem &, size_t)>;

        TabControl();
        ~TabControl() override = default;

        TabItem *addTab(std::unique_ptr<TabItem> tab, size_t index = static_cast<size_t>(-1));
        void removeTab(TabItem &tab);

        bool selectTab(size_t index);
        bool selectTab(TabItem &tab);
        void clearSelection();

        TabItem *getSelectedTab() const noexcept;
        size_t getSelectedIndex() const noexcept;

        void setOnSelectionChanged(SelectionCallback callback);

    private:
        void handleTabActivation(TabItem &tab);
        TabItem *asTabItem(Node *node) const noexcept;

        TabItem *selectedTab_ = nullptr;
        size_t selectedIndex_ = static_cast<size_t>(-1);
        SelectionCallback onSelectionChanged_;
    };
}
