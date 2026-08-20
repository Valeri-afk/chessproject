#pragma once

#include <optional>

enum class PieceType
{
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
};

enum class Side
{
    WHITE,
    BLACK
};

enum class PromotionType
{
    NONE,
    QUEEN,
    ROOK,
    BISHOP,
    KNIGHT
};

struct Position
{
    int row;
    int col;

    bool operator==(const Position &pos) const
    {
        return (row == pos.row) && (col == pos.col);
    }
};

enum class MoveType
{
    Normal,
    CastleKingSide,
    CastleQueenSide,
    EnPassant,
    DoublePawnMove
};

struct Move
{
    Position from;
    Position to;

    MoveType type = MoveType::Normal;

    PromotionType promotion = PromotionType::NONE;

    bool operator==(const Move &move) const
    {
        return (from == move.from) && (to == move.to) && (type == move.type) && (promotion == move.promotion);
    }
};

struct PositionStatus
{
    bool check = false;
    bool checkmate = false;
    bool stalemate = false;
};

struct MoveOutcome
{
    Move move;
    PositionStatus positionStatus;
};

struct Piece
{
    PieceType type;
    Side side;

    bool operator==(const Piece &piece) const
    {
        return type == piece.type && side == piece.side;
    }
};
