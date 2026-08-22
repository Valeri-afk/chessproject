#include "layout_system.hpp"
#include "linear_layout.hpp"
#include "layout_constraints.hpp"
#include "ui_framework/stack_panel_node.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
    constexpr float kInfinity = std::numeric_limits<float>::max();
    float finiteOrZero(float value) noexcept { return std::isfinite(value) ? value : 0.0f; }
    float finiteOrInfinity(float value) noexcept { return std::isfinite(value) ? value : kInfinity; }
    float safeAdd(float a, float b) noexcept { if (!std::isfinite(a) || !std::isfinite(b)) return kInfinity; const float result = a + b; return std::isfinite(result) ? result : kInfinity; }
    ui::Padding sanitizePadding(ui::Padding padding) noexcept { padding.left=std::max(0.0f,finiteOrZero(padding.left)); padding.right=std::max(0.0f,finiteOrZero(padding.right)); padding.top=std::max(0.0f,finiteOrZero(padding.top)); padding.bottom=std::max(0.0f,finiteOrZero(padding.bottom)); return padding; }
    ui::Border sanitizeBorder(ui::Border border) noexcept { border.left=std::max(0.0f,finiteOrZero(border.left)); border.right=std::max(0.0f,finiteOrZero(border.right)); border.top=std::max(0.0f,finiteOrZero(border.top)); border.bottom=std::max(0.0f,finiteOrZero(border.bottom)); return border; }
    ui::LayoutSize paddingBorderSize(const ui::Padding &p,const ui::Border &b) noexcept { return {safeAdd(safeAdd(p.left,p.right),safeAdd(b.left,b.right)),safeAdd(safeAdd(p.top,p.bottom),safeAdd(b.top,b.bottom))}; }
    ui::LayoutSize toContentSize(const ui::Node &n,ui::LayoutSize s) noexcept { const auto i=paddingBorderSize(sanitizePadding(n.getPadding()),sanitizeBorder(n.getBorder())); return {std::max(0.0f,finiteOrInfinity(s.width)-finiteOrZero(i.width)),std::max(0.0f,finiteOrInfinity(s.height)-finiteOrZero(i.height))}; }
    ui::LayoutSize toBorderBoxSize(const ui::Node &n,ui::LayoutSize s) noexcept { const auto i=paddingBorderSize(sanitizePadding(n.getPadding()),sanitizeBorder(n.getBorder())); return {safeAdd(s.width,i.width),safeAdd(s.height,i.height)}; }
    ui::LayoutPosition getContentPosition(const ui::Node &n) noexcept { const auto p=n.getActualPosition(); const auto pad=sanitizePadding(n.getPadding()); const auto b=sanitizeBorder(n.getBorder()); return {finiteOrZero(p.x)+b.left+pad.left,finiteOrZero(p.y)+b.top+pad.top}; }
    ui::LayoutSize getContentSize(const ui::Node &n) noexcept { return toContentSize(n,n.getActualSize()); }
}
namespace ui
{
LayoutSystem::LayoutSystem()=default;
void LayoutSystem::setViewportSize(const LayoutSize&size)noexcept{viewportSize_={finiteOrZero(size.width),finiteOrZero(size.height)};} LayoutSize LayoutSystem::getViewportSize()const noexcept{return viewportSize_;}
bool LayoutSystem::syncViewportFromRenderer(SDL_Renderer*renderer){if(!renderer)return false;int width=0,height=0;if(!SDL_GetRenderLogicalPresentation(renderer,&width,&height,nullptr)&&!SDL_GetCurrentRenderOutputSize(renderer,&width,&height))return false;const LayoutSize size{static_cast<float>(width),static_cast<float>(height)};if(size==viewportSize_)return false;viewportSize_=size;return true;}
void LayoutSystem::requestFullLayout(NodeTree&nodeTree){nodeTree.requestFullLayout();}
void LayoutSystem::processLayoutQueue(NodeTree&nodeTree){{NodeTree::ScopedMutationGuard guard(nodeTree);nodeTree.forEachLayoutQueue([this,&nodeTree](Node&root){if(!root.isVisible())return;const Node::Id rootId=root.getId();const LayoutSize available=makeRootAvailableSize(root);measureRecursive(root,available,nodeTree);Node*live=nodeTree.findNode(rootId);if(!live)return;live->actualSize_=internal::resolveFinalSize(*live,available);live->actualPosition_=live->position_;arrangeRecursive(*live,nodeTree);});}nodeTree.flushMutationQueue();}
void LayoutSystem::measureRecursive(Node&node,const LayoutSize&available,NodeTree&nodeTree){if(!node.isVisible())return;const Node::Id nodeId=node.getId();const LayoutSize border=internal::resolveMeasurementProposal(node,available);const LayoutSize content=toContentSize(node,border);LayoutSize desired{};if(auto*panel=dynamic_cast<StackPanelNode*>(&node)){internal::LinearMeasureContext ctx;ctx.availableSize=content;ctx.measureChild=[this,&nodeTree,&node](size_t index,const LayoutSize&childContent)->LayoutSize{Node*child=node.getVisibleChild(index);if(!child)return{};const Node::Id id=child->getId();measureRecursive(*child,toBorderBoxSize(*child,childContent),nodeTree);Node*live=nodeTree.findNode(id);return live?live->getDesiredSize():LayoutSize{};};desired=internal::measureLinearPanel(*panel,ctx);for(size_t i=0;i<panel->getChildCount();++i){Node*child=panel->getChild(i);if(!child||!child->isVisible()||child->getPositionMode()!=PositionMode::Absolute)continue;measureRecursive(*child,toBorderBoxSize(*child,content),nodeTree);}}else desired=node.measureContent(content);Node*live=nodeTree.findNode(nodeId);if(!live)return;live->desiredSize_=toBorderBoxSize(*live,desired);}
void LayoutSystem::arrangeRecursive(Node&node,NodeTree&nodeTree){if(!node.isVisible())return;internal::LinearArrangeContext ctx;ctx.contentPosition=getContentPosition(node);ctx.contentSize=getContentSize(node);if(auto*panel=dynamic_cast<StackPanelNode*>(&node)){ctx.placeChild=[this,&nodeTree,&node](size_t index,const LayoutPosition&position,const LayoutSize&size){Node*child=node.getVisibleChild(index);if(!child||!child->isVisible())return;child->actualPosition_=position;child->actualSize_=internal::resolveFinalSize(*child,size);arrangeRecursive(*child,nodeTree);};internal::arrangeLinearPanel(*panel,ctx);for(size_t i=0;i<panel->getChildCount();++i){Node*child=panel->getChild(i);if(!child||!child->isVisible()||child->getPositionMode()!=PositionMode::Absolute)continue;const LayoutSize proposal=internal::resolveMeasurementProposal(*child,ctx.contentSize);const LayoutSize allocated=child->getSize().width.isValue()||child->getSize().height.isValue()?proposal:child->getDesiredSize();child->actualPosition_=ctx.contentPosition+child->getPosition();child->actualSize_=internal::resolveFinalSize(*child,allocated);arrangeRecursive(*child,nodeTree);}}node.arrangeContent(ctx.contentPosition,ctx.contentSize);}
LayoutSize LayoutSystem::makeRootAvailableSize(const Node&root)const{LayoutSize size=viewportSize_;const auto value=root.getSize();if(value.width.isValue())size.width=value.width.value;if(value.height.isValue())size.height=value.height.value;return size;}
}
