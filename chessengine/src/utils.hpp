#pragma once

#include <cmath>
#include "chessengine/types.hpp"

namespace ChessUtils
{

    inline Side getOpponentSide(Side side)
    {
        return (side == Side::WHITE) ? Side::BLACK : Side::WHITE;
    }

    inline int getPawnDirection(Side side)
    {
        return (side == Side::WHITE) ? 1 : -1;
    }

    inline bool isEnemy(const std::optional<Piece> &piece, Side side)
    {
        if (!piece.has_value())
            return false;

        return piece.value().side != side;
    }

    inline bool isAlly(const std::optional<Piece> &piece, Side side)
    {
        if (!piece.has_value())
            return false;

        return piece.value().side == side;
    }

    inline bool isType(const std::optional<Piece> &piece, PieceType type)
    {
        if (!piece.has_value())
            return false;

        return piece.value().type == type;
    }

    inline Piece makePiece(PieceType type, Side side)
    {
        return Piece{type, side};
    }

    inline Position addShift(const Position &pos, const Position &shift)
    {
        return Position{pos.row + shift.row, pos.col + shift.col};
    }
}
