#pragma once

#include <optional>
#include <vector>
#include "core/texture.hpp"
#include <chessengine/types.hpp>
#include "game/ui/general.hpp"

struct PromotionButton
{
    PromotionType type;
    Texture *texture;
};

class UIPromotionComponent
{
private:
    SDLWrapper &sdl;

    bool active = false;

    std::vector<PromotionButton> buttons;

public:
    LayoutPosition position = {0, 0};
    LayoutSize buttonSize = {0, 0};

    UIPromotionComponent(SDLWrapper &sdl);

    std::optional<PromotionType> getButton(LayoutPosition mouse) const;

    void addButton(PromotionButton button);
    void removeButtons();

    bool isOpen() const;

    void close();
    void open();

    void draw() const;
};