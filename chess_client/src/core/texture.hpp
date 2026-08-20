#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

class Texture
{
public:
    /**
     * @brief Конструктор - захватывает владение текстурой
     * @param renderer Рендерер, которому принадлежит текстура
     * @param tex Указатель на SDL_Texture
     */
    Texture(SDL_Renderer *renderer, SDL_Texture *tex);
    /**
     * @brief Деструктор - уничтожает SDL_Texture
     */
    ~Texture();

    /**
     * @brief Конструктор перемещения
     * @param other Объект, из которого перемещаются данные
     */
    Texture(Texture &&other) noexcept;

    /**
     * @brief Оператор перемещения
     * @param other Объект, из которого перемещаются данные
     * @return Ссылка на текущий объект
     */
    Texture &operator=(Texture &&other) noexcept;

    // Запрет копирования
    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;

    /**
     * @brief Возвращает указатель на SDL_Texture
     * @return Указатель на текстуру SDL
     */
    SDL_Texture *get() const { return texture; }

    /**
     * @brief Возвращает ширину текстуры
     * @return Ширина в пикселях
     */
    float getWidth() const { return width; }

    /**
     * @brief Возвращает высоту текстуры
     * @return Высота в пикселях
     */
    float getHeight() const { return height; }

    /**
     * @brief Отрисовывает текстуру
     * @param renderer Рендерер для отрисовки
     * @param x X-координата верхнего левого угла
     * @param y Y-координата верхнего левого угла
     * @param srcRect Прямоугольник-источник (nullptr = вся текстура)
     * @param scale Масштаб (1.0 = исходный размер)
     * @param angle Угол поворота в градусах
     */
    void render(float x, float y, const SDL_FRect *srcRect = nullptr,
                float scale = 1.0f, float angle = 0.0f) const;

private:
    SDL_Texture *texture;        ///< Указатель на SDL_Texture
    float width, height;         ///< Размеры текстуры (кешированные)
    SDL_Renderer *ownerRenderer; ///< Рендерер-владелец
};
