#include "game.hpp"

#include <chessengine/chessconstants.hpp>

GameScene::GameScene(
    SDLWrapper &sdl,
    ResourceManager &rsm)
    : sdl(sdl),
      rsm(rsm),
      chessController(chessEngine),
      uiChessBoardComponent(sdl),
      uiPromotionComponent(sdl)
{
    auto loadPiece = [&](auto &map,
                         PieceType type,
                         const std::string &path)
    {
        if (rsm.loadTexture(path))
            map[type] = rsm.getTexture(path);
    };

    loadPiece(whitePieces, PieceType::PAWN, "pieces_images/white_pawn.png");
    loadPiece(whitePieces, PieceType::KNIGHT, "pieces_images/white_knight.png");
    loadPiece(whitePieces, PieceType::BISHOP, "pieces_images/white_bishop.png");
    loadPiece(whitePieces, PieceType::ROOK, "pieces_images/white_rook.png");
    loadPiece(whitePieces, PieceType::QUEEN, "pieces_images/white_queen.png");
    loadPiece(whitePieces, PieceType::KING, "pieces_images/white_king.png");

    loadPiece(blackPieces, PieceType::PAWN, "pieces_images/black_pawn.png");
    loadPiece(blackPieces, PieceType::KNIGHT, "pieces_images/black_knight.png");
    loadPiece(blackPieces, PieceType::BISHOP, "pieces_images/black_bishop.png");
    loadPiece(blackPieces, PieceType::ROOK, "pieces_images/black_rook.png");
    loadPiece(blackPieces, PieceType::QUEEN, "pieces_images/black_queen.png");
    loadPiece(blackPieces, PieceType::KING, "pieces_images/black_king.png");

    chessEngine.newGame();

    uiChessBoardComponent.squareSize = {16, 16};
    uiChessBoardComponent.boardPos =
        {
            (sdl.getWidth() -
             uiChessBoardComponent.squareSize.width * ChessConstants::BOARD_SIZE) *
                0.5f,
            (sdl.getHeight() -
             uiChessBoardComponent.squareSize.height * ChessConstants::BOARD_SIZE) *
                0.5f};

    updateBoardPieces();
}

Texture *GameScene::getPieceImage(Side side, PieceType type) const
{
    const auto &pieces = side == Side::WHITE ? whitePieces : blackPieces;
    const auto it = pieces.find(type);
    return it != pieces.end() ? it->second : nullptr;
}

void GameScene::updateBoardPieces()
{
    uiChessBoardComponent.clearPieces();

    for (int row = 0; row < ChessConstants::BOARD_SIZE; ++row)
    {
        for (int col = 0; col < ChessConstants::BOARD_SIZE; ++col)
        {
            const auto piece = chessEngine.getPieceAt({row, col});
            if (!piece)
                continue;

            uiChessBoardComponent.addPiece(
                {row, col},
                getPieceImage(piece->side, piece->type));
        }
    }
}

void GameScene::update()
{
    handleClick();

    if (lastMoveOutcome)
    {
        const auto &status = lastMoveOutcome->positionStatus;
        if (status.checkmate || status.stalemate)
        {
            chessEngine.newGame();
            lastMoveOutcome.reset();
            promotionMoves.clear();
            uiPromotionComponent.removeButtons();
            uiPromotionComponent.close();
            uiChessBoardComponent.clearCheckedSquare();
            resetSelect();
            updateBoardPieces();
            return;
        }
    }

    if (!promotionMoves.empty() && !uiPromotionComponent.isOpen())
    {
        uiPromotionComponent.removeButtons();

        for (const Move &move : promotionMoves)
        {
            uiPromotionComponent.addButton(
                {move.promotion,
                 getPieceImage(
                     chessEngine.getSideToMove(),
                     promotionToPieceType(move.promotion))});
        }

        uiPromotionComponent.buttonSize = {16, 16};
        const int count = static_cast<int>(promotionMoves.size());
        uiPromotionComponent.position =
            {
                (sdl.getWidth() - count * uiPromotionComponent.buttonSize.width) * 0.5f,
                (sdl.getHeight() - uiPromotionComponent.buttonSize.height) * 0.5f};
        uiPromotionComponent.open();
    }
}

void GameScene::draw()
{
    sdl.drawRect(
        0, 0, sdl.getWidth(), sdl.getHeight(),
        255, 255, 0, 255, true);

    uiChessBoardComponent.draw();

    if (uiPromotionComponent.isOpen())
    {
        sdl.drawFullscreenOverlay(0, 0, 0, 160);
        uiPromotionComponent.draw();
    }
}

/*
Повторение позиции (Threefold Repetition)
Если одна и та же позиция встречается трижды (не обязательно подряд)
Игрок может требовать ничью

Правило 50 ходов (Fifty-move rule)
Если последние 50 ходов не было взятий и ходов пешек
Игрок может требовать ничью

Невозможность мата (Insufficient material)
Автоматическая ничья, если у обоих игроков недостаточно фигур для мата:
Король против короля
Король + слон против короля
Король + конь против короля
Король + слон против короля + слона (слоны на полях одного цвета)

Контроль времени
Каждому игроку выделяется время на партию
Просрочка времени → поражение (если нет мата на доске)
*/
