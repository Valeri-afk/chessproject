#pragma once

#include <unordered_map>
#include <unordered_set>
#include <SDL3/SDL.h>

/**
 * @brief Типы кнопок мыши
 */
enum class MouseButtonType
{
    LEFT,   ///< Левая кнопка мыши
    RIGHT,  ///< Правая кнопка мыши
    MIDDLE, ///< Средняя кнопка мыши (колесо)
    NONE    ///< Неопределенная кнопка
};

/**
 * @brief Структура события нажатия кнопки мыши
 */
struct MouseButtonClick
{
    MouseButtonType button = MouseButtonType::NONE; ///< Тип нажатой кнопки
    float x = 0.0f;                                 ///< Координата X на момент нажатия (относительно окна)
    float y = 0.0f;                                 ///< Координата Y на момент нажатия (относительно окна)
};

/**
 * @brief Структура события отпускания кнопки мыши
 */
struct MouseButtonRelease
{
    MouseButtonType button = MouseButtonType::NONE; ///< Тип отпущенной кнопки
    float x = 0.0f;                                 ///< Координата X на момент отпускания (относительно окна)
    float y = 0.0f;                                 ///< Координата Y на момент отпускания (относительно окна)
};

/**
 * @brief Класс для управления вводом с клавиатуры и мыши
 *
 * Обеспечивает отслеживание состояния клавиш и кнопок мыши,
 * позволяя различать удержание, однократные нажатия и отпускания.
 */
class InputManager
{
private:
    SDL_Renderer *ren;
    // Клавиатура
    std::unordered_map<SDL_Keycode, bool> keys;     ///< Текущее состояние клавиш
    std::unordered_map<SDL_Keycode, bool> prevKeys; ///< Состояние клавиш в предыдущем кадре
    bool quitFlag = false;                          ///< Флаг запроса на выход из приложения

    // Мышь - состояние для отслеживания JustPressed/JustReleased
    std::unordered_map<MouseButtonType, bool> mouseButtons;     ///< Текущее состояние кнопок мыши
    std::unordered_map<MouseButtonType, bool> prevMouseButtons; ///< Состояние кнопок мыши в предыдущем кадре

    // Мышь - одноразовые события
    MouseButtonClick mouseClick;     ///< Событие нажатия кнопки мыши в текущем кадре
    MouseButtonRelease mouseRelease; ///< Событие отпускания кнопки мыши в текущем кадре

    // Мышь - позиция
    std::pair<float, float> mousePos; ///< Текущая позиция курсора мыши (относительно окна)

    /**
     * @brief Преобразует SDL код кнопки мыши в MouseButtonType
     * @param button SDL код кнопки (SDL_BUTTON_LEFT, SDL_BUTTON_RIGHT и т.д.)
     * @return Соответствующий тип кнопки или NONE если кнопка не поддерживается
     */
    MouseButtonType getMouseButtonType(Uint8 button) const;

    /**
     * @brief Сбрасывает одноразовые события мыши
     *
     * Вызывается в начале каждого кадра для очистки событий предыдущего кадра
     */
    void resetMouseEvents();

public:
    InputManager(SDL_Renderer *ren);

    ~InputManager();
    /**
     * @brief Обновляет состояние всех устройств ввода
     *
     * Должен вызываться один раз в начале каждого кадра.
     * Сохраняет текущее состояние в prev, обрабатывает очередь событий SDL
     * и обновляет состояние клавиш и кнопок мыши.
     */
    void update();

    /**
     * @brief Проверяет, удерживается ли указанная кнопка мыши
     * @param button Тип кнопки мыши
     * @return true если кнопка зажата в текущем кадре, false в противном случае
     *
     * Используется для непрерывных действий (например, перетаскивание, рисование)
     */
    bool isMouseButtonHeld(MouseButtonType button) const;

    /**
     * @brief Проверяет, была ли кнопка мыши только что нажата
     * @param button Тип кнопки мыши
     * @return true если кнопка была нажата в этом кадре, false в противном случае
     *
     * Используется для однократных действий (например, начало перетаскивания, клик по объекту)
     */
    bool isMouseButtonJustPressed(MouseButtonType button) const;

    /**
     * @brief Проверяет, была ли кнопка мыши только что отпущена
     * @param button Тип кнопки мыши
     * @return true если кнопка была отпущена в этом кадре, false в противном случае
     *
     * Используется для завершения действий (например, окончание перетаскивания)
     */
    bool isMouseButtonJustReleased(MouseButtonType button) const;

    /**
     * @brief Возвращает событие нажатия кнопки мыши в текущем кадре
     * @return Константная ссылка на структуру MouseButtonClick
     *
     * Событие содержит тип кнопки и координаты нажатия.
     * Сбрасывается в начале каждого кадра.
     */
    const MouseButtonClick &getMouseClick() const;

    /**
     * @brief Возвращает событие отпускания кнопки мыши в текущем кадре
     * @return Константная ссылка на структуру MouseButtonRelease
     *
     * Событие содержит тип кнопки и координаты отпускания.
     * Сбрасывается в начале каждого кадра.
     */
    const MouseButtonRelease &getMouseRelease() const;

    /**
     * @brief Возвращает текущую позицию курсора мыши
     * @return Пара координат (x, y) относительно окна
     */
    std::pair<float, float> getMousePosition() const;

    /**
     * @brief Возвращает набор кнопок мыши, удерживаемых в текущем кадре
     * @return Множество типов зажатых кнопок мыши
     *
     * Удобно для проверки наличия любой зажатой кнопки или итерации по ним
     */
    std::unordered_set<MouseButtonType> getHeldMouseButtons() const;

    /**
     * @brief Проверяет, зажата ли указанная клавиша клавиатуры
     * @param key SDL код клавиши (например, SDLK_W, SDLK_SPACE)
     * @return true если клавиша зажата в текущем кадре, false в противном случае
     *
     * Используется для непрерывных действий (ходьба, движение камеры)
     */
    bool isKeyPressed(SDL_Keycode key) const;

    /**
     * @brief Проверяет, была ли клавиша только что нажата
     * @param key SDL код клавиши
     * @return true если клавиша была нажата в этом кадре, false в противном случае
     *
     * Используется для однократных действий (открытие инвентаря, пауза).
     * Отличает новое нажатие от удержания клавиши.
     */
    bool isKeyJustPressed(SDL_Keycode key) const;

    /**
     * @brief Проверяет, была ли клавиша только что отпущена
     * @param key SDL код клавиши
     * @return true если клавиша была отпущена в этом кадре, false в противном случае
     */
    bool isKeyJustReleased(SDL_Keycode key) const;

    /**
     * @brief Проверяет, был ли запрошен выход из приложения
     * @return true если пользователь закрыл окно или система запросила выход
     */
    bool shouldQuit() const;

    /**
     * @brief Сбрасывает флаг выхода
     *
     * Должен вызываться после обработки события выхода
     */
    void resetQuitFlag();
};