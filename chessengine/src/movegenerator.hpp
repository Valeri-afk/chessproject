#pragma once

#include <array>
#include <vector>

#include "internal_types.hpp"
#include "chessengine/types.hpp"
#include "chessboard.hpp"
#include "utils.hpp"

class MoveGenerator
{
public:
    std::vector<Move> generateMoves(
        Position pos,
        const ChessBoard &board,
        const PositionState &state) const;

private:
    std::vector<Move> generatePawnMoves(
        Position pos,
        const ChessBoard &board,
        Side pieceSide,
        const PositionState &state) const;

    void addStraightPawnMoves(
        Position pos,
        Position shift,
        std::vector<Move> &moves,
        const ChessBoard &board,
        Side pieceSide) const;

    void addDiagonalPawnMoves(
        Position pos,
        Position shift,
        std::vector<Move> &moves,
        const ChessBoard &board,
        const PositionState &state,
        Side pieceSide) const;

    void addPromotionPawnMoves(Position from, Position to, std::vector<Move> &moves) const;

    std::vector<Move> generateKingMoves(
        Position pos,
        const ChessBoard &board,
        Side pieceSide, const PositionState &state) const;

    void addKingSideCastle(
        std::vector<Move> &moves,
        const ChessBoard &board,
        const PositionState &state,
        Side pieceSide) const;

    void addQueenSideCastle(
        std::vector<Move> &moves,
        const ChessBoard &board,
        const PositionState &state,
        Side pieceSide) const;

    std::vector<Move> generateKnightMoves(
        Position pos,
        const ChessBoard &board,
        Side pieceSide) const;

    template <size_t N>
    std::vector<Move> generateSlidingMoves(
        Position pos,
        const ChessBoard &board,
        Side pieceSide,
        const std::array<Position, N> &shifts) const
    {
        std::vector<Move> moves;

        for (const Position &shift : shifts)
        {
            Position current = pos;

            while (true)
            {
                current = ChessUtils::addShift(current, shift);

                if (!board.isValidPosition(current))
                    break;

                std::optional<Piece> optPiece = board.getPieceAt(current);

                if (optPiece.has_value())
                {
                    if (ChessUtils::isEnemy(optPiece, pieceSide))
                        moves.push_back({pos, current});

                    break;
                }

                moves.push_back({pos, current});
            }
        }

        return moves;
    }
};