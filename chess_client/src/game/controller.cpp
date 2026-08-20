#include "controller.hpp"

ChessController::ChessController(ChessEngine &chessEngine) : chessEngine(chessEngine) {}

void ChessController::selectPiece(Position pos)
{
    activePiece = pos;

    activeMoves = chessEngine.getLegalMovesFor(pos);

    uiChessBoardComponent.setActiveSquare(pos);

    std::vector<Position> moveSquares;
    moveSquares.reserve(activeMoves.size());

    for (const Move &move : activeMoves)
        moveSquares.push_back(move.to);

    uiChessBoardComponent.setMoveSquares(moveSquares);
}

void ChessController::resetSelect()
{
    activePiece.reset();
    activeMoves.clear();
    promotionMoves.clear();

    uiChessBoardComponent.clearActiveSquare();
    uiChessBoardComponent.clearMoveSquares();
}

void ChessController::handlePieceSelect(Position pos)
{
    auto piece = chessEngine.getPieceAt(pos);
    Side movingSide = chessEngine.getSideToMove();

    //----------------------------------------------------------
    // Если уже выбрана фигура — пробуем сделать ход
    //----------------------------------------------------------

    if (activePiece.has_value())
    {
        for (const Move &move : activeMoves)
        {
            if (move.to != pos)
                continue;

            //--------------------------------------------------
            // Promotion
            //--------------------------------------------------

            if (move.promotion != PromotionType::NONE)
            {
                promotionMoves.clear();

                for (const Move &promotionMove : activeMoves)
                {
                    if (promotionMove.to == pos &&
                        promotionMove.promotion != PromotionType::NONE)
                    {
                        promotionMoves.push_back(promotionMove);
                    }
                }

                return;
            }
            //--------------------------------------------------
            // Обычный ход
            //--------------------------------------------------

            lastMoveOutcome = chessEngine.makeMove(move);

            resetSelect();

            updateBoardPieces();

            if (lastMoveOutcome &&
                lastMoveOutcome->positionStatus.check)
            {
                uiChessBoardComponent.setCheckedSquare(
                    chessEngine.getKingPosition(
                        chessEngine.getSideToMove()));
            }
            else
            {
                uiChessBoardComponent.clearCheckedSquare();
            }

            return;
        }
    }

    //----------------------------------------------------------
    // Выбираем новую фигуру
    //----------------------------------------------------------

    if (piece &&
        piece->side == movingSide)
    {
        selectPiece(pos);
    }
    else
    {
        resetSelect();
    }
}

void ChessController::handlePromotion(float x, float y)
{
    auto promotion = uiPromotionComponent.getButton({x, y});

    if (!promotion)
        return;

    for (const Move &move : promotionMoves)
    {
        if (move.promotion != *promotion)
            continue;

        lastMoveOutcome = chessEngine.makeMove(move);
        break;
    }

    promotionMoves.clear();

    uiPromotionComponent.removeButtons();
    uiPromotionComponent.close();

    resetSelect();

    updateBoardPieces();

    if (lastMoveOutcome &&
        lastMoveOutcome->positionStatus.check)
    {
        uiChessBoardComponent.setCheckedSquare(
            chessEngine.getKingPosition(
                chessEngine.getSideToMove()));
    }
    else
    {
        uiChessBoardComponent.clearCheckedSquare();
    }
}

void ChessController::handleClick()
{
    auto click = input.getMouseClick();

    //---------------------------------------------------------
    // Promotion menu
    //---------------------------------------------------------

    if (!promotionMoves.empty())
    {
        if (click.button == MouseButtonType::LEFT)
            handlePromotion(click.x, click.y);

        return;
    }

    //---------------------------------------------------------
    // Chess board
    //---------------------------------------------------------

    auto square =
        uiChessBoardComponent.getSquare(
            {click.x,
             click.y});

    if (!square)
        return;

    if (click.button == MouseButtonType::LEFT)
    {
        handlePieceSelect(*square);
    }
    else
    {
        resetSelect();
    }
}