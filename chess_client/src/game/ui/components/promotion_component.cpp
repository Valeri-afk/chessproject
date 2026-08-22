#include "promotion_component.hpp"

UIPromotionComponent::UIPromotionComponent(SDLWrapper &sdl) : sdl(sdl) {};

void UIPromotionComponent::addButton(PromotionButton button)
{
    buttons.push_back(button);
};
void UIPromotionComponent::removeButtons()
{
    buttons.clear();
};

std::optional<PromotionType> UIPromotionComponent::getButton(LayoutPosition mouse) const
{
    if (!active || buttons.empty())
        return std::nullopt;

    float localX = mouse.x - position.x;
    float localY = mouse.y - position.y;

    if (localY < 0 || localY >= buttonSize.height)
        return std::nullopt;

    int index = static_cast<int>(localX / buttonSize.width);

    if (index < 0 || index >= buttons.size())
        return std::nullopt;

    return buttons[index].type;
}

bool UIPromotionComponent::isOpen() const
{
    return active;
}

void UIPromotionComponent::close()
{
    active = false;
}

void UIPromotionComponent::open()
{
    active = true;
}

void UIPromotionComponent::draw() const
{
    for (size_t i = 0; i < buttons.size(); ++i)
    {
        LayoutPosition nextPos = {position.x + i * buttonSize.width, position.y};
        renderUIBox(sdl, buttonSize, nextPos, DefaultColors::WHITE, 1, DefaultColors::BLACK);

        if (buttons[i].texture)
            buttons[i].texture->render(nextPos.x, nextPos.y);
    }
};
