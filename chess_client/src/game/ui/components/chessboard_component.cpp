#include "game/ui/components/chessboard_component.hpp"
#include "game/ui/general.hpp"

UIChessBoardComponent::UIChessBoardComponent(SDLWrapper &sdl)
    : sdl(sdl)
{
    pieces.fill(nullptr);
}

std::optional<Position> UIChessBoardComponent::getSquare(LayoutPosition mouse) const
{
    float localX = mouse.x - boardPos.x;
    float localY = mouse.y - boardPos.y;

    if (localX < 0 || localY < 0)
        return std::nullopt;

    int row = localY / squareSize.height;
    int col = localX / squareSize.width;

    if (row >= SIZE || col >= SIZE)
        return std::nullopt;

    return Position{row, col};
}

void UIChessBoardComponent::addPiece(Position pos, Texture *texture)
{
    if (!texture)
        return;

    if (pos.row < 0 || pos.row >= SIZE ||
        pos.col < 0 || pos.col >= SIZE)
        return;

    pieces[index(pos.row, pos.col)] = texture;
}

void UIChessBoardComponent::clearPieces()
{
    pieces.fill(nullptr);
}

void UIChessBoardComponent::setMoveSquares(const std::vector<Position> &moves)
{
    moveSquares = moves;
}

void UIChessBoardComponent::clearActiveSquare()
{
    activeSquare.reset();
}

void UIChessBoardComponent::clearCheckedSquare()
{
    checkedSquare.reset();
}

LayoutPosition UIChessBoardComponent::getSquarePosition(Position pos) const
{
    return {
        boardPos.x + pos.col * squareSize.width,
        boardPos.y + pos.row * squareSize.height};
}

void UIChessBoardComponent::setActiveSquare(Position pos)
{
    if (pos.col < 0 || pos.col >= SIZE || pos.row < 0 || pos.row >= SIZE)
        return;

    activeSquare = pos;
}

void UIChessBoardComponent::setCheckedSquare(Position pos)
{
    if (pos.col < 0 || pos.col >= SIZE || pos.row < 0 || pos.row >= SIZE)
        return;

    checkedSquare = pos;
}

void UIChessBoardComponent::clearMoveSquares()
{
    moveSquares.clear();
}

void UIChessBoardComponent::draw() const
{
    for (int row = 0; row < SIZE; ++row)
    {
        for (int col = 0; col < SIZE; ++col)
        {
            DefaultColors squareColor = ((row + col) % 2 == 0) ? DefaultColors::DESERT_SAND : DefaultColors::CAMEL;

            if (checkedSquare.has_value())
            {
                auto [checkedRow, checkedCol] = checkedSquare.value();

                if (row == checkedRow && col == checkedCol)
                    squareColor = DefaultColors::RED;
            }

            float xPos = boardPos.x + col * squareSize.width;
            float yPos = boardPos.y + row * squareSize.height;

            if (activeSquare.has_value())
            {
                auto [activeRow, activeCol] = activeSquare.value();

                if (row == activeRow && col == activeCol)
                    squareColor = DefaultColors::GREEN;
            }

            renderUIBox(sdl, squareSize, {xPos, yPos}, squareColor);
        }
    }

    LayoutSize halfSquareSize = squareSize * 0.25f;

    for (const auto &squarePos : moveSquares)
    {
        auto [row, col] = squarePos;

        float xPos = (boardPos.x + col * squareSize.width) + (squareSize.width - halfSquareSize.width) * 0.5f;
        float yPos = (boardPos.y + row * squareSize.height) + (squareSize.height - halfSquareSize.height) * 0.5f;

        renderUIBox(sdl, halfSquareSize, {xPos, yPos}, DefaultColors::GREEN);
    }

    for (int row = 0; row < SIZE; ++row)
    {
        for (int col = 0; col < SIZE; ++col)
        {
            Texture *tex = pieces[index(row, col)];

            if (!tex)
                continue;

            LayoutPosition pos = getSquarePosition({row, col});

            tex->render(pos.x, pos.y);
        }
    }
};
