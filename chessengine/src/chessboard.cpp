#include "chessboard.hpp"

std::optional<Piece> ChessBoard::getPieceAt(Position pos) const
{
    if (!isValidPosition(pos))
        return std::nullopt;
    return board[pos.row][pos.col];
}

void ChessBoard::setPieceAt(Position pos, Piece piece)
{
    if (!isValidPosition(pos))
        return;

    board[pos.row][pos.col] = piece;
}

void ChessBoard::removePieceAt(Position pos)
{
    if (!isValidPosition(pos))
        return;

    board[pos.row][pos.col].reset();
}

void ChessBoard::movePiece(const Move &move)
{
    if (!isValidPosition(move.from) || !isValidPosition(move.to))
        return;
    if (!isOccupied(move.from))
        return;

    if (move.from == move.to)
        return;

    board[move.to.row][move.to.col] = board[move.from.row][move.from.col];
    board[move.from.row][move.from.col].reset();
}

bool ChessBoard::isOccupied(Position pos) const
{
    return isValidPosition(pos) &&
           board[pos.row][pos.col].has_value();
}
