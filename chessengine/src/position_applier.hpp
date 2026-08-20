#pragma once

#include "chessengine/types.hpp"
#include "internal_types.hpp"
#include "chessboard.hpp"

class PositionApplier
{
private:
public:
    PositionApplier() = default;

    void applyMove(
        ChessBoard &board,
        PositionState &state,
        const Move &move) const;

private:
    void applyKingMove(
        ChessBoard &board,
        PositionState &state,
        const Move &move,
        Side movingSide) const;

    void applyRookMove(
        ChessBoard &board,
        PositionState &state,
        const Move &move,
        Side movingSide) const;

    void applyPawnMove(
        ChessBoard &board,
        PositionState &state,
        const Move &move,
        Side movingSide) const;

    void applyDoublePawnMove(
        ChessBoard &board,
        PositionState &state,
        const Move &move,
        Side movingSide) const;

    void applyEnpassantMove(
        ChessBoard &board,
        const Move &move,
        Side movingSide) const;

    void handlePromotion(
        ChessBoard &board,
        const Move &move,
        Side movingSide) const;

    void handleRookTaking(
        ChessBoard &board,
        PositionState &state,
        const Move &move,
        Side movingSide) const;
};