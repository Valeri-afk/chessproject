#include "node_tree.hpp"
#include "ui_framework/node.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

namespace
{
struct TestFailure{std::string message;};
void expect(bool condition,const char* message){if(!condition)throw TestFailure{message};}
void expectNear(float actual,float expected,float epsilon,const char* message){if(std::fabs(actual-expected)>epsilon)throw TestFailure{message};}

class AnimationNode final : public ui::Node
{
public:
    ui::FloatAnimationProperty valueProperty() noexcept{return makeFloatAnimationProperty(value_,&value_);}
    float value() const noexcept{return value_;}
private:
    float value_=0.0f;
};

void test_linear_animation_advances_through_node_tree()
{
    ui::NodeTree tree; auto node=std::make_unique<AnimationNode>(); AnimationNode* nodePtr=node.get(); tree.attachRoot(0,std::move(node));
    const auto property=nodePtr->valueProperty(); ui::AnimationController animations;
    animations.to(property,10.0f,1.0f,ui::AnimationEasing::Linear);
    tree.advanceTime(0.25f); expectNear(nodePtr->value(),2.5f,0.0001f,"linear animation must interpolate after partial time advance");
    tree.advanceTime(0.75f); expectNear(nodePtr->value(),10.0f,0.0001f,"animation must reach target at duration");
}
void test_easing_is_applied()
{
    ui::NodeTree tree; auto node=std::make_unique<AnimationNode>(); AnimationNode* p=node.get(); tree.attachRoot(0,std::move(node));
    auto easeIn=p->valueProperty(); float outValue=0.0f, inOutValue=0.0f;
    class EasingNode final:public ui::Node{public:ui::FloatAnimationProperty property(float& v)noexcept{return makeFloatAnimationProperty(v,&v);}};
    auto extra=std::make_unique<EasingNode>(); EasingNode* e=extra.get(); tree.attachRoot(1,std::move(extra));
    ui::AnimationController animations; animations.to(easeIn,1.0f,1.0f,ui::AnimationEasing::EaseIn); auto easeOut=e->property(outValue); animations.to(easeOut,1.0f,1.0f,ui::AnimationEasing::EaseOut); auto easeInOut=e->property(inOutValue); animations.to(easeInOut,1.0f,1.0f,ui::AnimationEasing::EaseInOut);
    tree.advanceTime(0.5f); expectNear(p->value(),0.25f,0.0001f,"EaseIn must square normalized time"); expectNear(outValue,0.75f,0.0001f,"EaseOut must invert EaseIn"); expectNear(inOutValue,0.5f,0.0001f,"EaseInOut must preserve midpoint");
}
void test_same_property_replaces_previous_animation()
{
    ui::NodeTree tree; auto node=std::make_unique<AnimationNode>(); AnimationNode* p=node.get(); tree.attachRoot(0,std::move(node)); const auto property=p->valueProperty(); ui::AnimationController animations;
    animations.to(property,10.0f,1.0f); tree.advanceTime(0.4f); expectNear(p->value(),4.0f,0.0001f,"first animation must advance before replacement");
    animations.to(property,8.0f,0.6f); tree.advanceTime(0.3f); expectNear(p->value(),6.0f,0.0001f,"replacement animation must start from current value"); tree.advanceTime(0.3f); expectNear(p->value(),8.0f,0.0001f,"replacement animation must reach its new target");
}
void test_different_properties_can_animate_concurrently()
{
    ui::NodeTree tree; auto node=std::make_unique<AnimationNode>(); AnimationNode* p=node.get(); tree.attachRoot(0,std::move(node)); float y=100.0f; const auto xProperty=p->valueProperty(); const auto yProperty=p->valueProperty(); ui::AnimationController animations;
    animations.to(xProperty,100.0f,1.0f); animations.to(yProperty,0.0f,2.0f); tree.advanceTime(0.5f); expectNear(p->value(),50.0f,0.0001f,"same backing storage must not be treated as independent properties"); (void)y;
}
void test_cancel_keeps_current_value()
{
    ui::NodeTree tree; auto node=std::make_unique<AnimationNode>(); AnimationNode* p=node.get(); tree.attachRoot(0,std::move(node)); const auto property=p->valueProperty(); ui::AnimationController animations;
    animations.to(property,10.0f,1.0f); tree.advanceTime(0.4f); animations.cancel(property); const float cancelled=p->value(); tree.advanceTime(0.6f); expectNear(p->value(),cancelled,0.0001f,"cancel must leave the current property value unchanged");
}
void test_zero_duration_applies_target_immediately()
{
    ui::NodeTree tree; auto node=std::make_unique<AnimationNode>(); AnimationNode* p=node.get(); tree.attachRoot(0,std::move(node)); const auto property=p->valueProperty(); ui::AnimationController animations;
    p->valueProperty().set(2.0f); animations.to(property,7.0f,0.0f); expectNear(p->value(),7.0f,0.0001f,"zero-duration animation must apply target immediately"); tree.advanceTime(1.0f); expectNear(p->value(),7.0f,0.0001f,"completed zero-duration animation must remain finished");
}
void test_animation_is_dropped_when_node_is_destroyed()
{
    ui::NodeTree tree; float value=0.0f; ui::FloatAnimationProperty property; {auto node=std::make_unique<AnimationNode>(); AnimationNode* p=node.get(); tree.attachRoot(0,std::move(node)); property=p->valueProperty(); ui::AnimationController animations; animations.to(property,10.0f,1.0f); tree.removeRoot(p);} tree.advanceTime(1.0f); expectNear(value,0.0f,0.0001f,"destroyed node animation must not invoke unrelated storage"); expect(!property.isValid()||property.value()==0.0f,"destroyed property must not expose a live value");
}
}
int main(){try{test_linear_animation_advances_through_node_tree();test_easing_is_applied();test_same_property_replaces_previous_animation();test_different_properties_can_animate_concurrently();test_cancel_keeps_current_value();test_zero_duration_applies_target_immediately();test_animation_is_dropped_when_node_is_destroyed();}catch(const TestFailure& failure){std::cerr<<"Animation tests failed: "<<failure.message<<'\n';return EXIT_FAILURE;}std::cout<<"Animation tests passed\n";return EXIT_SUCCESS;}
