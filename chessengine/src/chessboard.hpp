#pragma once

#include <optional>
#include <array>
#include "chessengine/types.hpp"
#include "chessengine/chessconstants.hpp"

class ChessBoard
{
public:
    static constexpr bool isValidPosition(Position pos)
    {
        return pos.row >= 0 &&
               pos.row < ChessConstants::BOARD_SIZE &&
               pos.col >= 0 &&
               pos.col < ChessConstants::BOARD_SIZE;
    };

    std::optional<Piece> getPieceAt(Position pos) const;
    void setPieceAt(Position pos, Piece piece);
    void removePieceAt(Position pos);
    void movePiece(const Move &move);

    bool isOccupied(Position pos) const;

private:
    std::array<std::array<std::optional<Piece>, ChessConstants::BOARD_SIZE>, ChessConstants::BOARD_SIZE> board;
};
