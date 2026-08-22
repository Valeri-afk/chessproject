#include "ui_framework/components/button.hpp"
#include "ui_framework/primitives.hpp"
#include "ui_framework/event_types.hpp"
#include "../core/text_primitive.hpp"
#include <algorithm>
#include <cmath>
#include <utility>
namespace ui
{
    Button::Button() { setDefaultGeometry(); setFocusable(true); setCapturable(true); addHandler<MouseDownEvent>([this](MouseDownEvent &e, Node &) { handleMouseDown(e); }); addHandler<MouseUpEvent>([this](MouseUpEvent &e, Node &) { handleMouseUp(e); }); addHandler<MouseClickEvent>([this](MouseClickEvent &e, Node &) { handleMouseClick(e); }); addHandler<MouseEnterEvent>([this](MouseEnterEvent &e, Node &) { handleMouseEnter(e); }); addHandler<MouseLeaveEvent>([this](MouseLeaveEvent &e, Node &) { handleMouseLeave(e); }); }
    Button::Button(float borderRadius) : Button() { setBorderRadius(borderRadius); }
    void Button::setDefaultGeometry() { setPadding({12.0f,12.0f,8.0f,8.0f}); setBorder({1.0f,1.0f,1.0f,1.0f}); }
    void Button::setText(std::string text) { if (text_ == text) return; deferLayoutMutation([text=std::move(text)](Node &n){ static_cast<Button&>(n).text_=std::move(text); }); }
    const std::string &Button::getText() const noexcept { return text_; }
    void Button::setFont(TTF_Font *font) { if (font_ == font) return; deferLayoutMutation([font](Node &n){ static_cast<Button&>(n).font_=font; }); }
    TTF_Font *Button::getFont() const noexcept { return font_; }
    void Button::setTextColor(Color c) noexcept { textColor_=c; }
    Color Button::getTextColor() const noexcept { return textColor_; }
    void Button::setVariant(Variant v) noexcept { variant_=v; }
    Button::Variant Button::getVariant() const noexcept { return variant_; }
    void Button::setBackgroundColor(Color c) noexcept { backgroundColor_=c; }
    Color Button::getBackgroundColor() const noexcept { return backgroundColor_; }
    void Button::setBorderColor(Color c) noexcept { borderColor_=c; }
    Color Button::getBorderColor() const noexcept { return borderColor_; }
    void Button::setBorderRadius(float r) noexcept { borderRadius_=std::max(0.0f,r); }
    float Button::getBorderRadius() const noexcept { return borderRadius_; }
    void Button::setPressScale(float s) noexcept { pressScale_=std::clamp(s,0.0f,1.0f); }
    float Button::getPressScale() const noexcept { return pressScale_; }
    void Button::setPressAnimationEnabled(bool e) noexcept { pressAnimationEnabled_=e; if(!e) currentScale_=targetScale_=1.0f; }
    bool Button::isPressAnimationEnabled() const noexcept { return pressAnimationEnabled_; }
    void Button::setPressAnimationSpeed(float s) noexcept { pressAnimationSpeed_=std::max(0.0f,s); }
    float Button::getPressAnimationSpeed() const noexcept { return pressAnimationSpeed_; }
    bool Button::isPressed() const noexcept { return pressed_; }
    bool Button::isHovered() const noexcept { return hovered_; }
    void Button::setOnActivate(ActivateCallback cb) { onActivate_=std::move(cb); }
    void Button::activate() { if(!isVisible() || !isEnabled()) return; onActivate(); }
    void Button::onActivate() { if(onActivate_) onActivate_(*this); }
    Color Button::presentationBackgroundColor() const noexcept { return backgroundColor_; }
    Color Button::presentationBorderColor() const noexcept { return borderColor_; }
    Color Button::presentationTextColor() const noexcept { return textColor_; }
    void Button::update(float dt) { targetScale_=pressed_?pressScale_:1.0f; if(!pressAnimationEnabled_) { currentScale_=targetScale_; return; } const float t=1.0f-std::exp(-pressAnimationSpeed_*std::max(0.0f,dt)); currentScale_ += (targetScale_-currentScale_)*t; }
    LayoutSize Button::measureContent(const LayoutSize &available) const { return TextPrimitive::measure(font_, text_, available.width); }
    void Button::draw(SDL_Renderer *renderer) { if(!renderer) return; const auto p=getActualPosition(); const auto s=getActualSize(); const auto bg=presentationBackgroundColor(); const auto border=presentationBorderColor(); const auto tc=presentationTextColor(); if(variant_!=Variant::TEXT) primitives::roundedBoxRGBA(renderer,p.x,p.y,p.x+s.width,p.y+s.height,borderRadius_,bg.r,bg.g,bg.b,bg.a); if(variant_==Variant::OUTLINED) primitives::roundedRectangleRGBA(renderer,p.x,p.y,p.x+s.width,p.y+s.height,borderRadius_,border.r,border.g,border.b,border.a); textPrimitive().draw(renderer,text_,font_,TextAlignment::CENTER,TextAlignment::CENTER,tc,p,s); }
    void Button::handleMouseDown(MouseDownEvent &e) { if(e.button==MouseButton::Left && isEnabled()) pressed_=true; }
    void Button::handleMouseUp(MouseUpEvent &e) { if(e.button==MouseButton::Left) pressed_=false; }
    void Button::handleMouseClick(MouseClickEvent &e) { if(e.button==MouseButton::Left) activate(); }
    void Button::handleMouseEnter(MouseEnterEvent &) { hovered_=true; }
    void Button::handleMouseLeave(MouseLeaveEvent &) { hovered_=false; pressed_=false; }
    Color Button::multiplyAlpha(Color c,float f) noexcept { c.a=static_cast<uint8_t>(std::clamp(c.a*f,0.0f,255.0f)); return c; }
    Color Button::lighten(Color c,float a) noexcept { a=std::clamp(a,0.0f,1.0f); c.r=static_cast<uint8_t>(c.r+(255-c.r)*a); c.g=static_cast<uint8_t>(c.g+(255-c.g)*a); c.b=static_cast<uint8_t>(c.b+(255-c.b)*a); return c; }
}