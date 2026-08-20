#pragma once

#include "chessengine/chessengine.hpp"
#include "internal_types.hpp"
#include "chessengine/chessconstants.hpp"
#include "chessboard.hpp"
#include "movegenerator.hpp"
#include "position_applier.hpp"
#include "attackedetector.hpp"
#include "moveanalyzer.hpp"

class ChessEngine::Impl
{
public:
    Impl()
        : moveGenerator(),
          positionApplier(),
          attackDetector(),
          moveAnalyzer(attackDetector,
                       positionApplier,
                       moveGenerator) {};

    ChessBoard board;
    PositionState state;
    Side movingSide;

    MoveGenerator moveGenerator;
    PositionApplier positionApplier;
    AttackDetector attackDetector;
    MoveAnalyzer moveAnalyzer;

    void initializeStartingPosition()
    {
        const std::array<PieceType, 8> backRank = {
            PieceType::ROOK, PieceType::KNIGHT, PieceType::BISHOP,
            PieceType::QUEEN, PieceType::KING, PieceType::BISHOP,
            PieceType::KNIGHT, PieceType::ROOK};

        // Белые
        for (int col = 0; col < 8; ++col)
        {
            board.setPieceAt({ChessConstants::WhiteBackRank, col},
                             {backRank[col], Side::WHITE});
            board.setPieceAt({ChessConstants::WhitePawnStartRank, col},
                             {PieceType::PAWN, Side::WHITE});
        }

        // Черные
        for (int col = 0; col < 8; ++col)
        {
            board.setPieceAt({ChessConstants::BlackBackRank, col},
                             {backRank[col], Side::BLACK});
            board.setPieceAt({ChessConstants::BlackPawnStartRank, col},
                             {PieceType::PAWN, Side::BLACK});
        }

        state.whiteKing = {ChessConstants::WhiteBackRank, ChessConstants::KingStartFile};
        state.blackKing = {ChessConstants::BlackBackRank, ChessConstants::KingStartFile};
        state.whiteCastlingRights = {true, true};
        state.blackCastlingRights = {true, true};
        state.enPassantTarget = std::nullopt;
        movingSide = Side::WHITE;
    };

    static bool isPositionValid(Position pos) { return ChessBoard::isValidPosition(pos); }
};