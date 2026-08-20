#include "input_manager.hpp"

InputManager::InputManager(SDL_Renderer *ren) : ren(ren) {}

InputManager::~InputManager() {}

void InputManager::update()
{
    // Сохраняем предыдущее состояние
    prevKeys = keys;
    prevMouseButtons = mouseButtons;

    SDL_Event event;
    resetMouseEvents();

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            quitFlag = true;
        }
        else if (event.type == SDL_EVENT_KEY_DOWN)
        {
            keys[event.key.key] = true;
        }
        else if (event.type == SDL_EVENT_KEY_UP)
        {
            keys[event.key.key] = false;
        }
        else if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST)
        {
            keys.clear();
            prevKeys.clear();
            mouseButtons.clear();
            prevMouseButtons.clear();
        }
        else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        {
            MouseButtonType type = getMouseButtonType(event.button.button);
            if (type != MouseButtonType::NONE)
            {
                // Обновляем состояние
                mouseButtons[type] = true;

                // Сохраняем событие
                mouseClick.button = type;

                SDL_RenderCoordinatesFromWindow(ren, event.button.x, event.button.y, &mouseClick.x, &mouseClick.y);
            }
        }
        else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
        {
            MouseButtonType type = getMouseButtonType(event.button.button);
            if (type != MouseButtonType::NONE)
            {
                // Обновляем состояние
                mouseButtons[type] = false;

                // Сохраняем событие
                mouseRelease.button = type;

                SDL_RenderCoordinatesFromWindow(ren, event.button.x, event.button.y, &mouseRelease.x, &mouseRelease.y);
            }
        }
        else if (event.type == SDL_EVENT_MOUSE_MOTION)
        {
            SDL_RenderCoordinatesFromWindow(ren, event.motion.x, event.motion.y, &mousePos.first, &mousePos.second);
        }
    }
}

void InputManager::resetMouseEvents()
{
    mouseClick = {MouseButtonType::NONE, 0.0f, 0.0f};
    mouseRelease = {MouseButtonType::NONE, 0.0f, 0.0f};
}

MouseButtonType InputManager::getMouseButtonType(Uint8 button) const
{
    if (button == SDL_BUTTON_LEFT)
        return MouseButtonType::LEFT;
    if (button == SDL_BUTTON_MIDDLE)
        return MouseButtonType::MIDDLE;
    if (button == SDL_BUTTON_RIGHT)
        return MouseButtonType::RIGHT;
    return MouseButtonType::NONE;
}

// Состояние кнопок
bool InputManager::isMouseButtonHeld(MouseButtonType button) const
{
    auto it = mouseButtons.find(button);
    return it != mouseButtons.end() && it->second;
}

bool InputManager::isMouseButtonJustPressed(MouseButtonType button) const
{
    bool curr = isMouseButtonHeld(button);
    bool prev = false;
    auto it = prevMouseButtons.find(button);
    if (it != prevMouseButtons.end())
        prev = it->second;
    return curr && !prev;
}

bool InputManager::isMouseButtonJustReleased(MouseButtonType button) const
{
    bool curr = isMouseButtonHeld(button);
    bool prev = false;
    auto it = prevMouseButtons.find(button);
    if (it != prevMouseButtons.end())
        prev = it->second;
    return !curr && prev;
}

// События
const MouseButtonClick &InputManager::getMouseClick() const
{
    return mouseClick;
}

const MouseButtonRelease &InputManager::getMouseRelease() const
{
    return mouseRelease;
}

// Позиция
std::pair<float, float> InputManager::getMousePosition() const
{
    return mousePos;
}

std::unordered_set<MouseButtonType> InputManager::getHeldMouseButtons() const
{
    std::unordered_set<MouseButtonType> result;
    for (const auto &[button, held] : mouseButtons)
    {
        if (held)
            result.insert(button);
    }
    return result;
}

// Клавиатура (без изменений)
bool InputManager::isKeyPressed(SDL_Keycode key) const
{
    auto it = keys.find(key);
    return it != keys.end() && it->second;
}

bool InputManager::isKeyJustPressed(SDL_Keycode key) const
{
    bool curr = false;
    auto currIt = keys.find(key);
    if (currIt != keys.end())
        curr = currIt->second;

    bool prev = false;
    auto prevIt = prevKeys.find(key);
    if (prevIt != prevKeys.end())
        prev = prevIt->second;

    return curr && !prev;
}

bool InputManager::isKeyJustReleased(SDL_Keycode key) const
{
    bool curr = false;
    auto currIt = keys.find(key);
    if (currIt != keys.end())
        curr = currIt->second;

    bool prev = false;
    auto prevIt = prevKeys.find(key);
    if (prevIt != prevKeys.end())
        prev = prevIt->second;

    return !curr && prev;
}

bool InputManager::shouldQuit() const { return quitFlag; }
void InputManager::resetQuitFlag() { quitFlag = false; }