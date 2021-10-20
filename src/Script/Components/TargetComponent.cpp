#include <morfuse/Script/Components/TargetComponent.h>
#include <morfuse/Script/TargetList.h>
#include <morfuse/Script/Context.h>

using namespace mfuse;

TargetComponent::TargetComponent(Listener& parentRef)
	: Component(parentRef)
{

}

TargetComponent::~TargetComponent()
{
	TargetList& targetList = ScriptContext::Get().GetTargetList();
	targetList.RemoveListener(Parent(), targetName.GetConstString());
}

void TargetComponent::SetTargetName(const StringResolvable& newTargetName)
{
	TargetList& targetList = ScriptContext::Get().GetTargetList();

	targetList.RemoveListener(Parent(), targetName.GetConstString());
	targetName = newTargetName;
	targetList.AddListener(Parent(), targetName.GetConstString());
}

const StringResolvable& TargetComponent::GetTargetName()
{
	return targetName;
}

void TargetComponent::SetTarget(const StringResolvable& newTarget)
{
	target = newTarget;
}

const StringResolvable& TargetComponent::GetTarget()
{
	return target;
}

Listener* TargetComponent::Next() const
{
	const TargetList& targetList = ScriptContext::Get().GetTargetList();
	return targetList.GetTarget(target);
}
