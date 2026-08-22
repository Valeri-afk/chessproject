#pragma once

#include <array>
#include <optional>
#include <vector>

#include <chessengine/types.hpp>

#include "core/texture.hpp"
#include "game/ui/general.hpp"

class UIChessBoardComponent
{
private:
    SDLWrapper &sdl;

    static constexpr int SIZE = 8;

    std::optional<Position> activeSquare;
    std::optional<Position> checkedSquare;

    std::vector<Position> moveSquares;

    std::array<Texture *, SIZE * SIZE> pieces{};

    constexpr int index(int row, int col) const
    {
        return row * SIZE + col;
    }

public:
    LayoutPosition boardPos{0, 0};
    LayoutSize squareSize{0, 0};

    UIChessBoardComponent(SDLWrapper &sdl);

    std::optional<Position> getSquare(LayoutPosition mouse) const;
    LayoutPosition getSquarePosition(Position pos) const;

    void setActiveSquare(Position pos);
    void clearActiveSquare();

    void setCheckedSquare(Position pos);
    void clearCheckedSquare();

    void setMoveSquares(const std::vector<Position> &moves);
    void clearMoveSquares();

    void addPiece(Position pos, Texture *texture);
    void clearPieces();

    void draw() const;
};