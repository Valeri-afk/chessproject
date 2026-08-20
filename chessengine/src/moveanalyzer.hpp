#pragma once

#include <vector>
#include "movegenerator.hpp"
#include "chessboard.hpp"
#include "position_applier.hpp"
#include "attackedetector.hpp"
#include "internal_types.hpp"

class MoveAnalyzer
{
private:
    const AttackDetector &attackDetector;
    const PositionApplier &positionApplier;
    const MoveGenerator &moveGenerator;

    struct PositionSnapshot
    {
        ChessBoard board;
        PositionState state;
    };

    PositionSnapshot simulateMove(
        const Move &move,
        const ChessBoard &board,
        const PositionState &state) const;

    bool hasAnyLegalMove(
        Side defender,
        const ChessBoard &board,
        const PositionState &state) const;

public:
    MoveAnalyzer(
        const AttackDetector &attackDetector,
        const PositionApplier &positionApplier,
        const MoveGenerator &moveGenerator);

    bool isKingInCheck(Side defender,
                       const ChessBoard &board,
                       const PositionState &state) const;

    bool isKingInCheckmate(Side defender,
                           const ChessBoard &board,
                           const PositionState &state) const;

    bool isKingInStalemate(Side defender,
                           const ChessBoard &board,
                           const PositionState &state) const;

    bool isMoveLegal(
        const Move &move,
        const ChessBoard &board,
        const PositionState &state) const;

    PositionStatus analyzePosition(
        Side movingSide,
        const ChessBoard &board,
        const PositionState &state) const;
};
