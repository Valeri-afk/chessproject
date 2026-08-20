#pragma once

#include <array>
#include <optional>
#include "utils.hpp"
#include "chessengine/types.hpp"
#include "chessboard.hpp"

class AttackDetector
{
public:
    bool isSquareAttacked(
        Side defender,
        Position square,
        const ChessBoard &board) const;

private:
    bool isAttackedByKnight(Side defender, Position square, const ChessBoard &board) const;
    bool isAttackedByKing(Side defender, Position square, const ChessBoard &board) const;
    bool isAttackedByPawn(Side defender, Position square, const ChessBoard &board) const;

    template <size_t N>
    bool isAttackedAlongDirections(
        Side defender,
        Position square,
        const ChessBoard &board,
        const std::array<Position, N> &directions,
        PieceType primary,
        PieceType secondary) const
    {
        for (const auto direction : directions)
        {
            Position current = square;

            while (true)
            {
                current = ChessUtils::addShift(current, direction);

                if (!board.isValidPosition(current))
                    break;

                std::optional<Piece> optPiece = board.getPieceAt(current);

                if (!optPiece.has_value())
                    continue;

                if (!ChessUtils::isEnemy(optPiece, defender))
                    break;

                if (ChessUtils::isType(optPiece, primary) ||
                    ChessUtils::isType(optPiece, secondary))
                    return true;

                break;
            }
        }

        return false;
    };
};
