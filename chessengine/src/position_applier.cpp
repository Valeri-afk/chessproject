#include "position_applier.hpp"
#include "utils.hpp"
#include "chessengine/chessconstants.hpp"

/*
PositionApplier invariants:

Input:
    ChessBoard
    PositionState
    Move

Output:
    Modified board/state

Responsibilities:
    - apply move to position
    - update castling rights
    - update en passant target
    - perform promotion
    - perform castling
    - perform en passant capture

Must NOT:
    - validate move legality
    - detect check
    - detect mate/stalemate
    - generate moves
    - analyze attacks
*/

void PositionApplier::applyMove(
    ChessBoard &board,
    PositionState &state,
    const Move &move) const
{
    std::optional<Piece> optPiece = board.getPieceAt(move.from);

    if (!optPiece.has_value())
        return;

    const Piece &movingPiece = optPiece.value();
    const Side movingSide = movingPiece.side;

    handleRookTaking(board, state, move, movingSide);

    switch (movingPiece.type)
    {
    case PieceType::KING:
        applyKingMove(board, state, move, movingSide);
        break;
    case PieceType::PAWN:
        applyPawnMove(board, state, move, movingSide);
        break;
    case PieceType::ROOK:
        applyRookMove(board, state, move, movingSide);
        break;
    default:
        board.movePiece(move);
        break;
    }

    if (move.type != MoveType::DoublePawnMove)
        state.enPassantTarget.reset();
}

void PositionApplier::applyKingMove(
    ChessBoard &board,
    PositionState &state,
    const Move &move,
    Side movingSide) const
{
    Position &currentKingPos = movingSide == Side::WHITE ? state.whiteKing : state.blackKing;

    if (move.type == MoveType::CastleKingSide || move.type == MoveType::CastleQueenSide)
    {
        board.movePiece(move);

        int castleRow = move.from.row;
        int rookColTarget = move.type == MoveType::CastleKingSide ? ChessConstants::KingSideRookTarget : ChessConstants::QueenSideRookTarget;
        int rookCol = move.type == MoveType::CastleKingSide ? ChessConstants::KingSideRookFile : ChessConstants::QueenSideRookFile;

        board.movePiece({{castleRow, rookCol}, {castleRow, rookColTarget}});

        currentKingPos = move.to;
    }
    else
    {
        board.movePiece(move);
        currentKingPos = move.to;
    }

    CastlingRights &castlingRights = movingSide == Side::BLACK ? state.blackCastlingRights : state.whiteCastlingRights;

    castlingRights.kingSide = false;
    castlingRights.queenSide = false;
};

void PositionApplier::applyRookMove(
    ChessBoard &board,
    PositionState &state,
    const Move &move,
    Side movingSide) const
{
    CastlingRights &castlingRights = movingSide == Side::BLACK ? state.blackCastlingRights : state.whiteCastlingRights;
    int rookRow = movingSide == Side::WHITE ? ChessConstants::WhiteBackRank : ChessConstants::BlackBackRank;

    if (move.from == Position{rookRow, ChessConstants::KingSideRookFile})
        castlingRights.kingSide = false;

    if (move.from == Position{rookRow, ChessConstants::QueenSideRookFile})
        castlingRights.queenSide = false;

    board.movePiece(move);
};

void PositionApplier::applyPawnMove(
    ChessBoard &board,
    PositionState &state,
    const Move &move,
    Side movingSide) const
{
    switch (move.type)
    {
    case MoveType::EnPassant:
        applyEnpassantMove(board, move, movingSide);
        break;
    case MoveType::DoublePawnMove:
        applyDoublePawnMove(board, state, move, movingSide);
        break;
    case MoveType::Normal:
        board.movePiece(move);
        handlePromotion(board, move, movingSide);
        break;
    default:
        break;
    }
};

void PositionApplier::applyDoublePawnMove(
    ChessBoard &board,
    PositionState &state,
    const Move &move,
    Side movingSide) const
{
    int dir = ChessUtils::getPawnDirection(movingSide);

    state.enPassantTarget = Position{
        move.from.row + dir,
        move.from.col};

    board.movePiece(move);
};

void PositionApplier::applyEnpassantMove(
    ChessBoard &board,
    const Move &move,
    Side movingSide) const
{
    int direction = ChessUtils::getPawnDirection(movingSide);

    Position capturedPawnPos{
        move.to.row - direction,
        move.to.col};

    board.removePieceAt(capturedPawnPos);
    board.movePiece(move);
};

void PositionApplier::handlePromotion(
    ChessBoard &board,
    const Move &move,
    Side movingSide) const
{
    PieceType type;

    switch (move.promotion)
    {
    case PromotionType::QUEEN:
        type = PieceType::QUEEN;
        break;
    case PromotionType::ROOK:
        type = PieceType::ROOK;
        break;
    case PromotionType::BISHOP:
        type = PieceType::BISHOP;
        break;
    case PromotionType::KNIGHT:
        type = PieceType::KNIGHT;
        break;
    default:
        return;
    }

    board.setPieceAt(move.to, Piece{type, movingSide});
}

void PositionApplier::handleRookTaking(
    ChessBoard &board,
    PositionState &state,
    const Move &move,
    Side movingSide) const
{
    Side defender = ChessUtils::getOpponentSide(movingSide);

    CastlingRights &castlingRights = defender == Side::BLACK ? state.blackCastlingRights : state.whiteCastlingRights;

    if (!castlingRights.kingSide && !castlingRights.queenSide)
        return;

    std::optional<Piece> optPiece = board.getPieceAt(move.to);

    if (ChessUtils::isType(optPiece, PieceType::ROOK) && ChessUtils::isAlly(optPiece, defender))
    {
        int rookRow = defender == Side::BLACK ? ChessConstants::BlackBackRank : ChessConstants::WhiteBackRank;

        if (move.to == Position{rookRow, ChessConstants::KingSideRookFile})
            castlingRights.kingSide = false;

        if (move.to == Position{rookRow, ChessConstants::QueenSideRookFile})
            castlingRights.queenSide = false;
    }
};
