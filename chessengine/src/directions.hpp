#pragma once

#include <array>
#include "chessengine/types.hpp"

namespace ChessDirections
{
    inline constexpr std::array<Position, 8> King = {
        Position{-1, 0},
        Position{-1, -1},
        Position{-1, 1},
        Position{0, -1},
        Position{0, 1},
        Position{1, 0},
        Position{1, -1},
        Position{1, 1}};

    inline constexpr std::array<Position, 8> Knight = {
        Position{-2, -1},
        Position{-1, -2},
        Position{-2, 1},
        Position{-1, 2},
        Position{2, -1},
        Position{1, -2},
        Position{2, 1},
        Position{1, 2}};

    inline constexpr std::array<Position, 4> Bishop =
        {
            Position{-1, 1},
            Position{-1, -1},
            Position{1, 1},
            Position{1, -1}};

    inline constexpr std::array<Position, 4> Rook =
        {
            Position{-1, 0},
            Position{1, 0},
            Position{0, -1},
            Position{0, 1}};

    inline constexpr std::array<Position, 8> Queen = {
        Position{-1, 1},
        Position{-1, -1},
        Position{1, 1},
        Position{1, -1},
        Position{-1, 0},
        Position{1, 0},
        Position{0, -1},
        Position{0, 1}};

    inline constexpr std::array<Position, 3> WhitePawnMove = {
        Position{1, 0},
        Position{1, -1},
        Position{1, 1}};

    inline constexpr std::array<Position, 3> BlackPawnMove = {
        Position{-1, 0},
        Position{-1, -1},
        Position{-1, 1}};

    inline constexpr std::array<Position, 2> WhitePawnAttack = {
        Position{-1, -1},
        Position{-1, 1}};

    inline constexpr std::array<Position, 2> BlackPawnAttack = {
        Position{1, -1},
        Position{1, 1}};

}
