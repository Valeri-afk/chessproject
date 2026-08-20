#pragma once

#include "chessengine/types.hpp"

struct CastlingRights
{
    bool kingSide = true;
    bool queenSide = true;
};

struct PositionState
{
    Position whiteKing;
    Position blackKing;

    CastlingRights whiteCastlingRights;
    CastlingRights blackCastlingRights;

    std::optional<Position> enPassantTarget;
};
