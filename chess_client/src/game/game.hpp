#pragma once

#include <optional>
#include <unordered_map>
#include <vector>

#include "core/resource_manager.hpp"
#include "core/sdl_wrapper.hpp"
#include "core/texture.hpp"

#include "colors.hpp"
#include "ui/components/chessboard_component.hpp"
#include "ui/components/promotion_component.hpp"

#include "controller.hpp"

#include <chessengine/chessengine.hpp>

class GameScene
{
private:
    SDLWrapper &sdl;
    ResourceManager &rsm;

    ChessEngine chessEngine;
    ChessController chessController;

    UIChessBoardComponent uiChessBoardComponent;
    UIPromotionComponent uiPromotionComponent;

    std::unordered_map<PieceType, Texture *> whitePieces;
    std::unordered_map<PieceType, Texture *> blackPieces;

private:
    void updateBoardPieces();
    Texture *getPieceImage(Side side, PieceType type) const;

public:
    GameScene(
        SDLWrapper &sdl,
        ResourceManager &rsm);

    void update();
    void draw();
};

inline PieceType promotionToPieceType(PromotionType p)
{
    switch (p)
    {
    case PromotionType::QUEEN:
        return PieceType::QUEEN;
    case PromotionType::ROOK:
        return PieceType::ROOK;
    case PromotionType::KNIGHT:
        return PieceType::KNIGHT;
    case PromotionType::BISHOP:
        return PieceType::BISHOP;
    default:
        throw std::runtime_error("Invalid promotion");
    }
}
