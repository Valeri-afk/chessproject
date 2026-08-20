#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <SDL3/SDL.h>

/**
 * @file sdl_wrapper.hpp
 * @brief Обёртка над SDL3 для управления окном и рендерером
 *
 * Предоставляет упрощённый интерфейс для работы с SDL3:
 * - Создание и управление окном
 * - Отрисовка графических примитивов
 * - Подсчёт FPS
 */

/**
 * @brief Класс-обёртка для управления SDL3 окном и рендерером
 *
 * Инкапсулирует создание и управление окном, рендерером,
 * предоставляет базовые методы для отрисовки и подсчёта FPS.
 */
class SDLWrapper
{
public:
    /**
     * @brief Конструктор - создаёт окно и рендерер
     * @param title Заголовок окна
     * @param width Ширина окна в пикселях
     * @param height Высота окна в пикселях
     *
     * Создаёт окно с флагами SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_FOCUS.
     * При ошибке выводит сообщение в cerr.
     */
    SDLWrapper(const char *title, int width, int height, bool startFullscreen);

    /**
     * @brief Деструктор - освобождает ресурсы SDL3
     *
     * Уничтожает рендерер, окно и вызывает SDL_Quit, TTF_Quit, MIX_Quit.
     */
    ~SDLWrapper();

    /**
     * @brief Очищает экран указанным цветом
     * @param r Красная компонента (0-255)
     * @param g Зелёная компонента (0-255)
     * @param b Синяя компонента (0-255)
     * @param a Альфа-компонента (0-255, по умолчанию непрозрачный)
     *
     * Вызывается в начале каждого кадра перед отрисовкой.
     */
    void clear(Uint8 r = 0, Uint8 g = 0, Uint8 b = 0, Uint8 a = 255);

    /**
     * @brief Отображает отрисованный кадр на экране
     *
     * Вызывается после завершения всей отрисовки кадра.
     * Показывает буфер рендерера на экране.
     */
    void present();

    /**
     * @brief Рисует прямоугольник на экране
     * @param x X-координата верхнего левого угла
     * @param y Y-координата верхнего левого угла
     * @param w Ширина прямоугольника
     * @param h Высота прямоугольника
     * @param r Красная компонента (0-255)
     * @param g Зелёная компонента (0-255)
     * @param b Синяя компонента (0-255)
     * @param filled true - закрашенный прямоугольник, false - только контур
     */
    void drawRect(float x, float y, int w, int h, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool filled = true);

    void drawFullscreenOverlay(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

    void drawCircle(float centerX, float centerY, float radius);

    /**
     * @brief Обновляет счётчик FPS
     *
     * Должен вызываться один раз в кадр.
     * Подсчитывает количество кадров за секунду.
     * Результат можно получить через getCurrentFPS().
     */
    void updateFPS() const;

    /**
     * @brief Возвращает указатель на SDL_Renderer
     * @return Указатель на рендерер
     */
    SDL_Renderer *getRenderer() const { return renderer; }

    /**
     * @brief Возвращает указатель на SDL_Window
     * @return Указатель на окно
     */
    SDL_Window *getWindow() const { return window; }

    /**
     * @brief Возвращает ширину окна
     * @return Ширина в пикселях
     */
    int getWidth() const { return width; }

    /**
     * @brief Возвращает высоту окна
     * @return Высота в пикселях
     */
    int getHeight() const { return height; }

    int getCurrentFps() const
    {
        return currentFPS;
    }

    SDL_Texture *getRenderTarget() const { return renderTarget; }

    void setWindowScale(int scale, bool linearFilter = false);

    void setFullscreen(bool fullscreen);

private:
    SDL_Window *window;     ///< Указатель на SDL окно
    SDL_Renderer *renderer; ///< Указатель на SDL рендерер
    SDL_Texture *renderTarget = nullptr;
    int width, height; ///< Размеры окна

    mutable Uint32 lastFPSTime; ///< Время последнего подсчёта FPS
    mutable int frameCount;     ///< Счётчик кадров с последнего подсчёта
    mutable int currentFPS;     ///< Текущее значение FPS
};
