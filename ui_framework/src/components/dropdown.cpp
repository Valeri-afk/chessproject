#include "ui_framework/components/dropdown.hpp"

#include <limits>
#include <utility>

namespace ui
{
    namespace
    {
        constexpr size_t INVALID_INDEX = std::numeric_limits<size_t>::max();
    }

    Dropdown::Dropdown()
    {
        auto trigger = std::make_unique<Button>();
        trigger_ = trigger.get();
        trigger_->setText(placeholder_);
        trigger_->setOnActivate(
            [this](Button &)
            {
                handleTriggerActivate();
            });

        auto menu = std::make_unique<Menu>();
        menu_ = menu.get();
        menu_->setVisible(false);
        menu_->setPositionMode(PositionMode::Absolute);
        menu_->setMainAlignment(MainAxisAlignment::START);
        menu_->setCrossAlignment(CrossAxisAlignment::STRETCH);
        menu_->setOnItemActivate(
            [this](MenuItem &item)
            {
                handleItemActivate(item);
            });

        add(std::move(trigger), 0);
        add(std::move(menu), 1);
    }

    MenuItem *Dropdown::addItem(
        std::unique_ptr<MenuItem> item,
        size_t index)
    {
        if (!item || !menu_)
            return nullptr;

        return menu_->addItem(std::move(item), index);
    }

    void Dropdown::removeItem(MenuItem &item)
    {
        if (menu_)
            menu_->removeItem(item);

        if (selectedItem_ == &item)
        {
            selectedItem_ = nullptr;
            selectedIndex_ = INVALID_INDEX;

            if (trigger_)
                trigger_->setText(placeholder_);
        }
    }

    void Dropdown::open()
    {
        if (!menu_ || !isEnabled())
            return;

        syncMenuGeometry();
        menu_->setVisible(true);
    }

    void Dropdown::close() noexcept
    {
        if (menu_)
            menu_->setVisible(false);
    }

    void Dropdown::toggle()
    {
        if (isOpen())
            close();
        else
            open();
    }

    bool Dropdown::isOpen() const noexcept
    {
        return menu_ && menu_->isVisible();
    }

    MenuItem *Dropdown::getSelectedItem() const noexcept
    {
        return selectedItem_;
    }

    size_t Dropdown::getSelectedIndex() const noexcept
    {
        return selectedIndex_;
    }

    void Dropdown::clearSelection() noexcept
    {
        if (selectedItem_)
            selectedItem_->setSelected(false);

        selectedItem_ = nullptr;
        selectedIndex_ = INVALID_INDEX;

        if (trigger_)
            trigger_->setText(placeholder_);
    }

    Button &Dropdown::getTrigger() noexcept
    {
        return *trigger_;
    }

    const Button &Dropdown::getTrigger() const noexcept
    {
        return *trigger_;
    }

    Menu &Dropdown::getMenu() noexcept
    {
        return *menu_;
    }

    const Menu &Dropdown::getMenu() const noexcept
    {
        return *menu_;
    }

    void Dropdown::setPlaceholder(std::string text)
    {
        placeholder_ = std::move(text);

        if (!selectedItem_ && trigger_)
            trigger_->setText(placeholder_);
    }

    const std::string &Dropdown::getPlaceholder() const noexcept
    {
        return placeholder_;
    }

    void Dropdown::setOnSelectionChanged(SelectionCallback callback)
    {
        onSelectionChanged_ = std::move(callback);
    }

    void Dropdown::handleTriggerActivate()
    {
        toggle();
    }

    void Dropdown::handleItemActivate(MenuItem &item)
    {
        size_t index = INVALID_INDEX;

        if (menu_)
        {
            for (size_t i = 0; i < menu_->childCount(); ++i)
            {
                if (menu_->getChildAt(i) == &item)
                {
                    index = i;
                    break;
                }
            }
        }

        if (index == INVALID_INDEX || !item.isEnabled() || !item.isVisible())
            return;

        if (selectedItem_ && selectedItem_ != &item)
            selectedItem_->setSelected(false);

        selectedItem_ = &item;
        selectedIndex_ = index;
        selectedItem_->setSelected(true);

        if (trigger_)
            trigger_->setText(item.getText());

        close();

        if (onSelectionChanged_)
            onSelectionChanged_(*this, item);
    }

    void Dropdown::syncMenuGeometry()
    {
        if (!trigger_ || !menu_)
            return;

        const LayoutPosition triggerPosition = trigger_->getPosition();
        const LayoutSize triggerSize = trigger_->getDesiredSize();

        menu_->setPosition({
            triggerPosition.x,
            triggerPosition.y + triggerSize.height});

        menu_->setSize(LayoutSizeValue::fixed(
            triggerSize.width,
            menu_->getDesiredSize().height));
    }
}
