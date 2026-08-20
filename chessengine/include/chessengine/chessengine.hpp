#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <string>
#include "types.hpp"

class ChessEngine
{
public:
    ChessEngine();
    ~ChessEngine();

    ChessEngine(const ChessEngine &) = delete;
    ChessEngine &operator=(const ChessEngine &) = delete;

    ChessEngine(ChessEngine &&) noexcept = default;
    ChessEngine &operator=(ChessEngine &&) noexcept = default;

    void newGame();
    std::string getFEN() const;

    std::optional<Piece> getPieceAt(Position pos) const;
    Position getKingPosition(Side side) const;

    Side getSideToMove() const;
    std::vector<Move> getLegalMovesFor(Position pos) const;

    MoveOutcome makeMove(Move move);

private:
    class Impl;

    std::unique_ptr<Impl> impl;
};