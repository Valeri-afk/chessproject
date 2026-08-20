#include "utils.hpp"
#include "moveanalyzer.hpp"
#include "chessengine/chessconstants.hpp"

/*
Input:
    ChessBoard
    PositionState
    Move

Dependencies:
    MoveGenerator
    PositionApplier
    AttackDetector

Responsibilities:
    - determine legality of a move
    - detect check
    - detect checkmate
    - detect stalemate

Must NOT:
    - modify real board
    - generate attacks itself
    - generate moves itself
*/

MoveAnalyzer::MoveAnalyzer(
    const AttackDetector &attackDetector,
    const PositionApplier &positionApplier,
    const MoveGenerator &moveGenerator)
    : attackDetector(attackDetector),
      positionApplier(positionApplier),
      moveGenerator(moveGenerator)
{
}

MoveAnalyzer::PositionSnapshot MoveAnalyzer::simulateMove(
    const Move &move,
    const ChessBoard &board,
    const PositionState &state) const
{
    ChessBoard tempBoard = board;
    PositionState tempState = state;

    positionApplier.applyMove(tempBoard, tempState, move);

    return {tempBoard, tempState};
};

bool MoveAnalyzer::hasAnyLegalMove(
    Side defender,
    const ChessBoard &board,
    const PositionState &state) const
{
    for (int row = 0; row < ChessConstants::BOARD_SIZE; ++row)
    {
        for (int col = 0; col < ChessConstants::BOARD_SIZE; ++col)
        {
            Position pos{row, col};

            std::optional<Piece> optPiece = board.getPieceAt(pos);

            if (!ChessUtils::isAlly(optPiece, defender))
                continue;

            const auto moves = moveGenerator.generateMoves(pos, board, state);

            for (const Move &move : moves)
            {
                if (isMoveLegal(move, board, state))
                    return true;
            }
        }
    }

    return false;
}

bool MoveAnalyzer::isKingInCheck(
    Side defender,
    const ChessBoard &board,
    const PositionState &state) const
{
    const Position &currentKingPos = defender == Side::WHITE ? state.whiteKing : state.blackKing;

    return attackDetector.isSquareAttacked(
        defender,
        currentKingPos,
        board);
}

bool MoveAnalyzer::isKingInCheckmate(
    Side defender,
    const ChessBoard &board,
    const PositionState &state) const
{
    return isKingInCheck(defender, board, state) &&
           !hasAnyLegalMove(defender, board, state);
}

bool MoveAnalyzer::isKingInStalemate(
    Side defender,
    const ChessBoard &board,
    const PositionState &state) const
{
    return !isKingInCheck(defender, board, state) &&
           !hasAnyLegalMove(defender, board, state);
}

bool MoveAnalyzer::isMoveLegal(
    const Move &move,
    const ChessBoard &board,
    const PositionState &state) const
{
    auto piece = board.getPieceAt(move.from);

    if (!piece)
        return false;

    Side movingSide = piece->side;

    if (move.type == MoveType::CastleKingSide ||
        move.type == MoveType::CastleQueenSide)
    {
        if (isKingInCheck(movingSide, board, state))
            return false;

        const int shift =
            move.type == MoveType::CastleKingSide
                ? 1
                : -1;

        if (attackDetector.isSquareAttacked(
                movingSide,
                {move.from.row, move.from.col + shift},
                board))
        {
            return false;
        }
    }

    PositionSnapshot snapshot =
        simulateMove(move, board, state);

    return !isKingInCheck(
        movingSide,
        snapshot.board,
        snapshot.state);
}

PositionStatus MoveAnalyzer::analyzePosition(
    Side movingSide,
    const ChessBoard &board,
    const PositionState &state) const
{
    PositionStatus status;

    status.check =
        isKingInCheck(movingSide, board, state);

    bool hasMoves =
        hasAnyLegalMove(movingSide, board, state);

    status.checkmate =
        status.check && !hasMoves;

    status.stalemate =
        !status.check && !hasMoves;

    return status;
}