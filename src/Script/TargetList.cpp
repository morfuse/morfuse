#include <morfuse/Script/TargetList.h>
#include <morfuse/Script/Listener.h>
#include <morfuse/Script/Context.h>
#include <morfuse/Script/ScriptException.h>

using namespace mfuse;

TargetList::TargetList()
{

}

TargetList::~TargetList()
{
}

void TargetList::AddListener(Listener& ent, const_str targetName)
{
	if (!targetName)
	{
		// useless
		return;
	}

	ConTarget* const list = GetTargetList(targetName);
	list->AddObject(&ent);
}

void TargetList::RemoveListener(Listener& ent, const_str targetName)
{
	if (!targetName)
	{
		// useless
		return;
	}

	ConTarget* const list = GetExistingTargetList(targetName);
	if (list)
	{
		list->RemoveObject(&ent);

		if (list->NumObjects() <= 0)
		{
			m_targetList.remove(targetName);
		}
	}
}

void TargetList::FreeTargetList()
{
	/*
	con::set_enum it = m_targetList;
	for (it.NextElement(); it.CurrentElement(); it.NextElement())
	{
		const ConTarget& targets = it.CurrentElement()->Value();
		// clear all targets
		const size_t numElements = targets.NumObjects();
		for (size_t i = 0; i < numElements; i++)
		{
			targets[i]->Clear();
		}
	}
	*/

	m_targetList.clear();
}

Listener* TargetList::GetNextTarget(const Listener* ent, const_str targetname) const
{
	const ConTarget* list = GetExistingConstTargetList(targetname);
	if (!list) {
		return nullptr;
	}

	uintptr_t index;

	if (ent) {
		index = FindTarget(*list, ent);
	}
	else {
		index = 1;
	}

	if (index <= list->NumObjects()) {
		return list->ObjectAt(index);
	}
	else {
		return nullptr;
	}
}

uintptr_t TargetList::FindTarget(const ConTarget& list, const Listener* target) const
{
	for (uintptr_t i = 1; i <= list.NumObjects(); ++i)
	{
		if (list.ObjectAt(i) == target) {
			return i;
		}
	}

	return list.NumObjects() + 1;
}

Listener* TargetList::GetTarget(const_str targetname) const
{
	const ConTarget* const list = GetExistingConstTargetList(targetname);
	if (!list)
	{
		// no container
		return nullptr;
	}

	if (!list->NumObjects())
	{
		// empty container
		return nullptr;
	}
	else if (list->NumObjects() == 1)
	{
		return list->ObjectAt(1);
	}
	else
	{
		throw TargetListErrors::MultipleTargetsException(list->NumObjects(), targetname);
	}
}

uintptr_t TargetList::GetTargetnameIndex(Listener& ent, const_str targetname) const
{
	const ConTarget* const list = GetExistingConstTargetList(targetname);
	return list ? list->IndexOfObject(&ent) : 0;
}

ConTarget* TargetList::GetExistingTargetList(const_str targetname)
{
	return m_targetList.findKeyValue(targetname);
}

const mfuse::ConTarget* TargetList::GetExistingConstTargetList(const_str targetname) const
{
	return m_targetList.findKeyValue(targetname);
}

ConTarget* TargetList::GetTargetList(const_str targetname)
{
	return &m_targetList.addKeyValue(targetname);
}

TargetListErrors::MultipleTargetsException::MultipleTargetsException(size_t numTargetsValue, const_str targetNameValue)
	: numTargets(numTargetsValue)
	, targetName(targetNameValue)
{
}

size_t TargetListErrors::MultipleTargetsException::GetNumTargets() const noexcept
{
	return numTargets;
}

const_str TargetListErrors::MultipleTargetsException::GetTargetName() const noexcept
{
	return targetName;
}

const char* TargetListErrors::MultipleTargetsException::what() const noexcept
{
	if (!filled())
	{
		const ScriptMaster& director = ScriptContext::Get().GetDirector();

		fill(
			"There are " + xstr(numTargets) + " entities with targetname '" + director.GetDictionary().Get(targetName)
			+ ". You are using a command that requires exactly one."
		);
	}

	return Messageable::what();
}

TargetListErrors::NoTargetException::NoTargetException(const_str targetNameValue)
	: targetName(targetNameValue)
{
}

const_str TargetListErrors::NoTargetException::GetTargetName() const noexcept
{
	return targetName;
}

const char* TargetListErrors::NoTargetException::what() const noexcept
{
	if (!filled())
	{
		const ScriptMaster& director = ScriptContext::Get().GetDirector();
		fill("Can't find target name '" + director.GetDictionary().Get(targetName) + "'");
	}

	return Messageable::what();
}
