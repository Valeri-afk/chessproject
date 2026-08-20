#include "utils.hpp"
#include "chessengine_impl.hpp"

ChessEngine::ChessEngine()
    : impl(std::make_unique<Impl>())
{
    newGame();
}

ChessEngine::~ChessEngine() = default;

void ChessEngine::newGame()
{
    impl->board = ChessBoard();
    impl->state = PositionState();
    impl->initializeStartingPosition();
}

std::string ChessEngine::getFEN() const
{
    // TODO: Реализовать генерацию FEN
    return "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
}

std::optional<Piece> ChessEngine::getPieceAt(Position pos) const
{
    if (!impl->isPositionValid(pos))
    {
        return std::nullopt;
    }
    return impl->board.getPieceAt(pos);
}

Position ChessEngine::getKingPosition(Side side) const
{
    return side == Side::WHITE ? impl->state.whiteKing : impl->state.blackKing;
};

Side ChessEngine::getSideToMove() const
{
    return impl->movingSide;
}

std::vector<Move> ChessEngine::getLegalMovesFor(Position pos) const
{
    if (!impl->isPositionValid(pos))
    {
        return {};
    }

    std::optional<Piece> piece = impl->board.getPieceAt(pos);
    if (!piece.has_value())
    {
        return {};
    }

    // Только свои фигуры
    if (piece->side != impl->movingSide)
    {
        return {};
    }

    std::vector<Move> pseudoLegal = impl->moveGenerator.generateMoves(pos, impl->board, impl->state);
    std::vector<Move> legal;

    for (const Move &move : pseudoLegal)
    {
        if (impl->moveAnalyzer.isMoveLegal(move, impl->board, impl->state))
            legal.push_back(move);
    }

    return legal;
}

MoveOutcome ChessEngine::makeMove(Move move)
{
    impl->positionApplier.applyMove(impl->board, impl->state, move);

    Side opponent = ChessUtils::getOpponentSide(impl->movingSide);
    impl->movingSide = opponent;

    PositionStatus status = impl->moveAnalyzer.analyzePosition(impl->movingSide, impl->board, impl->state);

    return {move, status};
}
