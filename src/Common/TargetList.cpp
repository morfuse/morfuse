#include <morfuse/Script/TargetList.h>
#include <morfuse/Script/SimpleEntity.h>
#include <morfuse/Script/Context.h>
#include <morfuse/Script/ScriptException.h>

using namespace mfuse;

void TargetList::AddTargetEntity(SimpleEntity* ent)
{
	const xstr& targetname = ent->TargetName();

	if (!targetname.length())
	{
		return;
	}

	ConSimple* list = GetTargetList(targetname.c_str());

	list->AddObject(ent);
}

void TargetList::AddTargetEntityAt(SimpleEntity* ent, int index)
{
	const xstr& targetname = ent->TargetName();

	if (!targetname.length())
	{
		return;
	}

	ConSimple* list = GetTargetList(targetname.c_str());

	list->AddObjectAt(index, ent);
}

void TargetList::RemoveTargetEntity(SimpleEntity* ent)
{
	const xstr& targetname = ent->TargetName();

	if (!targetname.length())
	{
		return;
	}

	ConSimple* list = GetExistingTargetList(targetname.c_str());

	if (list)
	{
		list->RemoveObject(ent);

		if (list->NumObjects() <= 0)
		{
			m_targetList.remove(ScriptContext::Get().GetDirector().AddString(targetname));
		}
	}
}

void TargetList::FreeTargetList()
{
	m_targetList.clear();
}

SimpleEntity* TargetList::GetNextEntity(const rawchar_t* targetname, SimpleEntity* ent)
{
	return GetNextEntity(ScriptContext::Get().GetDirector().AddString(targetname), ent);
}

SimpleEntity* TargetList::GetNextEntity(const_str targetname, SimpleEntity* ent)
{
	ConSimple* list = GetTargetList(targetname);
	uintptr_t index;

	if (ent)
	{
		index = list->IndexOfObject(ent) + 1;
	}
	else
	{
		index = 1;
	}

	if (list->NumObjects() >= index)
	{
		return list->ObjectAt(index);
	}
	else
	{
		return NULL;
	}
}

SimpleEntity* TargetList::GetScriptTarget(const rawchar_t* targetname)
{
	return GetScriptTarget(ScriptContext::Get().GetDirector().AddString(targetname));
}

SimpleEntity* TargetList::GetScriptTarget(const_str targetname)
{
	ConSimple* list = GetTargetList(targetname);

	if (list->NumObjects() == 1)
	{
		return list->ObjectAt(1);
	}
	else if (list->NumObjects() > 1)
	{
		ScriptError(
			"There are %d entities with targetname '%s'. You are using a command that requires exactly one.",
			list->NumObjects(),
			ScriptContext::Get().GetDirector().GetString(targetname).c_str()
		);
	}

	return NULL;
}

SimpleEntity* TargetList::GetTarget(const rawchar_t* targetname, bool quiet)
{
	return GetTarget(ScriptContext::Get().GetDirector().AddString(targetname), quiet);
}

SimpleEntity* TargetList::GetTarget(const_str targetname, bool quiet)
{
	ConSimple* list = GetTargetList(targetname);

	if (list->NumObjects() == 1)
	{
		return list->ObjectAt(1);
	}
	else if (list->NumObjects() > 1)
	{
		if (!quiet) {
			// FIXME
			//warning("World::GetTarget", "There are %d entities with targetname '%s'. You are using a command that requires exactly one.", list->NumObjects(), ScriptContext::Get().GetDirector().GetString(targetname).c_str());
		}
	}

	return NULL;
}

uintptr_t TargetList::GetTargetnameIndex(SimpleEntity* ent)
{
	ConSimple* list = GetTargetList(ent->TargetName().c_str());

	return list->IndexOfObject(ent);
}

ConSimple* TargetList::GetExistingTargetList(const rawchar_t* targetname)
{
	return GetExistingTargetList(ScriptContext::Get().GetDirector().AddString(targetname));
}

ConSimple* TargetList::GetExistingTargetList(const_str targetname)
{
	return m_targetList.findKeyValue(targetname);
}

ConSimple* TargetList::GetTargetList(const rawchar_t* targetname)
{
	return GetTargetList(ScriptContext::Get().GetDirector().AddString(targetname));
}

ConSimple* TargetList::GetTargetList(const_str targetname)
{
	return &m_targetList.addKeyValue(targetname);
}
