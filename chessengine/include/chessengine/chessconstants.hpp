#pragma once

namespace ChessConstants
{
    constexpr int BOARD_SIZE = 8;

    constexpr int WhiteBackRank = 0;
    constexpr int BlackBackRank = 7;

    constexpr int WhitePawnStartRank = 1;
    constexpr int BlackPawnStartRank = 6;

    constexpr int WhitePromotionRank = 7;
    constexpr int BlackPromotionRank = 0;

    constexpr int KingStartFile = 4;

    constexpr int QueenSideRookFile = 0;
    constexpr int KingSideRookFile = 7;

    constexpr int KingSideKingTarget = 6;
    constexpr int QueenSideKingTarget = 2;

    constexpr int KingSideRookTarget = 5;
    constexpr int QueenSideRookTarget = 3;
}