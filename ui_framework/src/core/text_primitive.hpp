#pragma once
#include <string>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "ui_framework/types.hpp"
namespace ui
{
class TextPrimitive
{
public:
    TextPrimitive()=default;
    ~TextPrimitive();
    TextPrimitive(const TextPrimitive&)=delete;
    TextPrimitive& operator=(const TextPrimitive&)=delete;
    static LayoutSize measure(TTF_Font *font,const std::string &text,float availableWidth=-1.0f) noexcept;
    void draw(SDL_Renderer *renderer,const std::string &text,TTF_Font *font,TextAlignment horizontalAlignment,TextAlignment verticalAlignment,Color color,const LayoutPosition &position,const LayoutSize &size);
private:
    void releaseTextObject() noexcept;
    void releaseRasterFont() noexcept;
    bool ensureTextObject(SDL_Renderer *renderer,TTF_Font *font,const std::string &text);
    bool ensureRasterFont(TTF_Font *font,float scale);
    bool getIntegerPresentationScale(SDL_Renderer *renderer,float &scale,SDL_FRect &presentationRect) const noexcept;
    SDL_Renderer *cachedRenderer_=nullptr;
    TTF_Font *cachedTextFont_=nullptr;
    std::string cachedText_;
    TTF_TextEngine *textEngine_=nullptr;
    TTF_Text *textObject_=nullptr;
    TTF_Font *rasterFont_=nullptr;
    TTF_Font *rasterSourceFont_=nullptr;
    float rasterScale_=1.0f;
    Uint32 rasterFontGeneration_=0;
};
}
