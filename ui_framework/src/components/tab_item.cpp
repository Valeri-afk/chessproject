#include "ui_framework/components/tab_item.hpp"
#include "ui_framework/primitives.hpp"
#include "../core/text_primitive.hpp"
#include <algorithm>
#include <utility>
namespace ui
{
    TabItem::TabItem() : textPrimitive_(std::make_unique<TextPrimitive>()) { setPadding({12.0f,12.0f,8.0f,8.0f}); setFocusable(true); setCapturable(true); addHandler<MouseClickEvent>([this](MouseClickEvent &event, Node &) { handleMouseClick(event); }); }
    TabItem::~TabItem() = default;
    void TabItem::setText(std::string text) { if(textLayout_.getText()==text) return; textLayout_.setText(std::move(text)); }
    const std::string &TabItem::getText() const noexcept { return textLayout_.getText(); }
    void TabItem::setFont(TTF_Font *font) { if(textLayout_.getFont()==font) return; textLayout_.setFont(font); }
    TTF_Font *TabItem::getFont() const noexcept { return textLayout_.getFont(); }
    void TabItem::setTextColor(Color color) noexcept { textColor_=color; }
    Color TabItem::getTextColor() const noexcept { return textColor_; }
    void TabItem::setBackgroundColor(Color color) noexcept { backgroundColor_=color; }
    Color TabItem::getBackgroundColor() const noexcept { return backgroundColor_; }
    void TabItem::setActive(bool active) noexcept { active_=active; }
    bool TabItem::isActive() const noexcept { return active_; }
    void TabItem::setOnActivate(ActivateCallback callback) { onActivate_=std::move(callback); }
    void TabItem::activate() { if(!isVisible() || !isEnabled()) return; if(onActivate_) onActivate_(*this); }
    LayoutSize TabItem::measureContent(const LayoutSize &availableContent) const { return textLayout_.measure(availableContent.width); }
    void TabItem::draw(SDL_Renderer *renderer) { if(!renderer||!textPrimitive_) return; const auto position=getActualPosition(); const auto size=getActualSize(); Color background=backgroundColor_; if(active_){ background.r=static_cast<uint8_t>(background.r+(255-background.r)*0.16f); background.g=static_cast<uint8_t>(background.g+(255-background.g)*0.16f); background.b=static_cast<uint8_t>(background.b+(255-background.b)*0.16f); } if(background.a>0) primitives::boxRGBA(renderer,position.x,position.y,position.x+size.width,position.y+size.height,background.r,background.g,background.b,background.a); const auto padding=getPadding(); const auto border=getBorder(); textPrimitive_->draw(renderer,textLayout_.getText(),textLayout_.getFont(),TextAlignment::CENTER,TextAlignment::CENTER,textColor_,{position.x+border.left+padding.left,position.y+border.top+padding.top},{std::max(0.0f,size.width-border.left-border.right-padding.left-padding.right),std::max(0.0f,size.height-border.top-border.bottom-padding.top-padding.bottom)}); }
    void TabItem::handleMouseClick(MouseClickEvent &event) { if(event.button==MouseButton::Left) activate(); }
}
