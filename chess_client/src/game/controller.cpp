#include "controller.hpp"

ChessController::ChessController(ChessEngine &chessEngine)
    : chessEngine(chessEngine)
{
}

void ChessController::selectSquare(Position position)
{
    const auto piece = chessEngine.getPieceAt(position);
    if (!piece || piece->side != chessEngine.getSideToMove())
    {
        resetSelection();
        return;
    }

    selectedPosition_ = position;
    legalMoves_ = chessEngine.getLegalMovesFor(position);
    promotionMoves_.clear();
}

bool ChessController::selectPromotion(PromotionType promotion)
{
    for (const Move &move : promotionMoves_)
    {
        if (move.promotion != promotion)
            continue;

        lastMoveOutcome_ = chessEngine.makeMove(move);
        clearSelectionState();
        return true;
    }

    return false;
}

void ChessController::resetSelection()
{
    clearSelectionState();
}

bool ChessController::hasSelection() const noexcept
{
    return selectedPosition_.has_value();
}

const std::optional<Position> &ChessController::selectedPosition() const noexcept
{
    return selectedPosition_;
}

const std::vector<Move> &ChessController::legalMoves() const noexcept
{
    return legalMoves_;
}

const std::vector<Move> &ChessController::promotionMoves() const noexcept
{
    return promotionMoves_;
}

const std::optional<MoveOutcome> &ChessController::lastMoveOutcome() const noexcept
{
    return lastMoveOutcome_;
}

void ChessController::preparePromotionMoves(const Position &target)
{
    promotionMoves_.clear();

    for (const Move &move : legalMoves_)
    {
        if (move.to == target && move.promotion != PromotionType::NONE)
            promotionMoves_.push_back(move);
    }
}

void ChessController::clearSelectionState()
{
    selectedPosition_.reset();
    legalMoves_.clear();
    promotionMoves_.clear();
}
