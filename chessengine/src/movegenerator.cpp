#include <cmath>
#include <optional>

#include "movegenerator.hpp"
#include "directions.hpp"
#include "utils.hpp"
#include "chessengine/chessconstants.hpp"
#include "internal_types.hpp"

/*
MoveGenerator invariants:

Input:
    ChessBoard
    PositionState

Output:
    Pseudo-legal moves only

Responsibilities:
    - piece movement
    - captures
    - promotion generation
    - castling generation
    - en passant generation

Must NOT:
    - detect check
    - detect checkmate
    - validate king safety
    - inspect attacked squares
    - analyze game result
*/

std::vector<Move> MoveGenerator::generateMoves(
    Position pos,
    const ChessBoard &board,
    const PositionState &state) const
{
    std::optional<Piece> optPiece = board.getPieceAt(pos);

    if (!optPiece.has_value())
        return {};

    const Piece &piece = optPiece.value();
    const Side pieceSide = piece.side;

    switch (piece.type)
    {
    case PieceType::PAWN:
        return generatePawnMoves(pos, board, pieceSide, state);

    case PieceType::KNIGHT:
        return generateKnightMoves(
            pos,
            board,
            pieceSide);

    case PieceType::KING:
        return generateKingMoves(
            pos,
            board,
            pieceSide, state);

    case PieceType::BISHOP:
        return generateSlidingMoves(
            pos,
            board,
            pieceSide,
            ChessDirections::Bishop);

    case PieceType::ROOK:
        return generateSlidingMoves(
            pos,
            board,
            pieceSide,
            ChessDirections::Rook);

    case PieceType::QUEEN:
        return generateSlidingMoves(
            pos,
            board,
            pieceSide,
            ChessDirections::Queen);

    default:
        return {};
    }
}

std::vector<Move> MoveGenerator::generateKnightMoves(
    Position pos,
    const ChessBoard &board,
    Side pieceSide) const
{
    std::vector<Move> moves;

    for (const Position &shift : ChessDirections::Knight)
    {
        Position next = ChessUtils::addShift(pos, shift);

        if (!board.isValidPosition(next))
            continue;

        std::optional<Piece> optPiece = board.getPieceAt(next);

        if (ChessUtils::isAlly(optPiece, pieceSide))
            continue;

        moves.push_back({pos, next});
    }

    return moves;
};

std::vector<Move> MoveGenerator::generateKingMoves(
    Position pos,
    const ChessBoard &board,
    Side pieceSide, const PositionState &state) const
{
    std::vector<Move> moves;

    for (const Position &shift : ChessDirections::King)
    {
        Position next = ChessUtils::addShift(pos, shift);

        if (!board.isValidPosition(next))
            continue;

        std::optional<Piece> optPiece = board.getPieceAt(next);

        if (ChessUtils::isAlly(optPiece, pieceSide))
            continue;

        moves.push_back({pos, next});
    }

    addKingSideCastle(moves, board, state, pieceSide);
    addQueenSideCastle(moves, board, state, pieceSide);

    return moves;
};

void MoveGenerator::addKingSideCastle(
    std::vector<Move> &moves,
    const ChessBoard &board,
    const PositionState &state,
    Side pieceSide) const
{
    CastlingRights castlingRights = pieceSide == Side::WHITE ? state.whiteCastlingRights : state.blackCastlingRights;

    if (!castlingRights.kingSide)
        return;

    int kingCol = ChessConstants::KingStartFile;
    int rookCol = ChessConstants::KingSideRookFile;
    int targetCol = ChessConstants::KingSideKingTarget;
    int castleRow = pieceSide == Side::WHITE ? ChessConstants::WhiteBackRank : ChessConstants::BlackBackRank;
    int shift = 1;

    for (int col = kingCol + shift; col < rookCol; ++col)
    {
        if (board.isOccupied({castleRow, col}))
            return;
    }

    moves.push_back({{castleRow, kingCol}, {castleRow, targetCol}, MoveType::CastleKingSide});
}

void MoveGenerator::addQueenSideCastle(std::vector<Move> &moves,
                                       const ChessBoard &board,
                                       const PositionState &state,
                                       Side pieceSide) const
{
    CastlingRights castlingRights = pieceSide == Side::WHITE ? state.whiteCastlingRights : state.blackCastlingRights;

    if (!castlingRights.queenSide)
        return;

    int kingCol = ChessConstants::KingStartFile;
    int rookCol = ChessConstants::QueenSideRookFile;
    int targetCol = ChessConstants::QueenSideKingTarget;
    int castleRow = pieceSide == Side::WHITE ? ChessConstants::WhiteBackRank : ChessConstants::BlackBackRank;
    int shift = 1;

    for (int col = rookCol + shift; col < kingCol; ++col)
    {
        if (board.isOccupied({castleRow, col}))
            return;
    }

    moves.push_back({{castleRow, kingCol}, {castleRow, targetCol}, MoveType::CastleQueenSide});
}

std::vector<Move> MoveGenerator::generatePawnMoves(
    Position pos,
    const ChessBoard &board,
    Side pieceSide,
    const PositionState &state) const
{
    std::vector<Move> moves;

    const auto &shifts =
        pieceSide == Side::WHITE
            ? ChessDirections::WhitePawnMove
            : ChessDirections::BlackPawnMove;

    for (const Position &shift : shifts)
    {
        if (shift.col == 0)
            addStraightPawnMoves(pos, shift, moves, board, pieceSide);
        else
            addDiagonalPawnMoves(pos, shift, moves, board, state, pieceSide);
    }

    return moves;
}

void MoveGenerator::addStraightPawnMoves(
    Position pos,
    Position shift,
    std::vector<Move> &moves,
    const ChessBoard &board,
    Side pieceSide) const
{
    Position next = ChessUtils::addShift(pos, shift);

    if (!board.isValidPosition(next))
        return;

    std::optional<Piece> optPiece = board.getPieceAt(next);

    if (optPiece.has_value())
        return;

    int pawnStartRank = pieceSide == Side::WHITE ? ChessConstants::WhitePawnStartRank : ChessConstants::BlackPawnStartRank;
    int promotionRank = pieceSide == Side::WHITE ? ChessConstants::WhitePromotionRank : ChessConstants::BlackPromotionRank;

    if (pos.row == pawnStartRank)
    {
        moves.push_back({pos, next});

        Position doubleNext = ChessUtils::addShift(next, shift);
        std::optional<Piece> optionPiece = board.getPieceAt(doubleNext);

        if (optionPiece.has_value())
            return;

        moves.push_back({pos, doubleNext, MoveType::DoublePawnMove});
    }
    else
    {
        if (next.row == promotionRank)
            addPromotionPawnMoves(pos, next, moves);
        else
            moves.push_back({pos, next});
    }
}

void MoveGenerator::addDiagonalPawnMoves(
    Position pos,
    Position shift,
    std::vector<Move> &moves,
    const ChessBoard &board,
    const PositionState &state,
    Side pieceSide) const
{
    Position next = ChessUtils::addShift(pos, shift);

    if (!board.isValidPosition(next))
        return;

    std::optional<Piece> optPiece = board.getPieceAt(next);

    if (ChessUtils::isAlly(optPiece, pieceSide))
        return;

    if (ChessUtils::isEnemy(optPiece, pieceSide))
    {
        int promotionRank = pieceSide == Side::WHITE ? ChessConstants::WhitePromotionRank : ChessConstants::BlackPromotionRank;

        if (next.row == promotionRank)
            addPromotionPawnMoves(pos, next, moves);
        else
            moves.push_back({pos, next});
    }
    else
    {
        if (state.enPassantTarget.has_value())
        {
            Position ep = state.enPassantTarget.value();

            if (ep == next)
                moves.push_back({pos, next, MoveType::EnPassant});
        }
    }
}

void MoveGenerator::addPromotionPawnMoves(Position from, Position to, std::vector<Move> &moves) const
{
    std::array<PromotionType, 4> promotions = {PromotionType::QUEEN, PromotionType::ROOK, PromotionType::KNIGHT, PromotionType::BISHOP};

    for (const auto &promotion : promotions)
    {
        moves.push_back({from, to, MoveType::Normal, promotion});
    }
};
