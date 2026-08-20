#include "attackedetector.hpp"
#include "utils.hpp"
#include "directions.hpp"

bool AttackDetector::isSquareAttacked(
    Side defender,
    Position square,
    const ChessBoard &board) const
{
    return isAttackedByKing(defender, square, board) ||
           isAttackedByKnight(defender, square, board) ||
           isAttackedByPawn(defender, square, board) ||
           isAttackedAlongDirections(defender, square, board, ChessDirections::Bishop, PieceType::BISHOP, PieceType::QUEEN) ||
           isAttackedAlongDirections(defender, square, board, ChessDirections::Rook, PieceType::ROOK, PieceType::QUEEN);
}

bool AttackDetector::isAttackedByKnight(
    Side defender,
    Position square,
    const ChessBoard &board) const
{
    for (const Position &shift : ChessDirections::Knight)
    {
        Position next = ChessUtils::addShift(square, shift);

        if (!board.isValidPosition(next))
            continue;

        std::optional<Piece> optPiece = board.getPieceAt(next);

        if (ChessUtils::isEnemy(optPiece, defender) &&
            ChessUtils::isType(optPiece, PieceType::KNIGHT))
        {
            return true;
        }
    }

    return false;
}

bool AttackDetector::isAttackedByKing(
    Side defender,
    Position square,
    const ChessBoard &board) const
{
    for (const Position &shift : ChessDirections::King)
    {
        Position next = ChessUtils::addShift(square, shift);

        if (!board.isValidPosition(next))
            continue;

        std::optional<Piece> optPiece = board.getPieceAt(next);

        if (ChessUtils::isEnemy(optPiece, defender) &&
            ChessUtils::isType(optPiece, PieceType::KING))
        {
            return true;
        }
    }

    return false;
}

bool AttackDetector::isAttackedByPawn(
    Side defender,
    Position square,
    const ChessBoard &board) const
{
    const auto &pawnShifts =
        (defender == Side::WHITE)
            ? ChessDirections::BlackPawnAttack
            : ChessDirections::WhitePawnAttack;

    for (const Position &shift : pawnShifts)
    {
        Position next = ChessUtils::addShift(square, shift);

        if (!board.isValidPosition(next))
            continue;

        std::optional<Piece> optPiece = board.getPieceAt(next);

        if (ChessUtils::isEnemy(optPiece, defender) &&
            ChessUtils::isType(optPiece, PieceType::PAWN))
        {
            return true;
        }
    }

    return false;
}
