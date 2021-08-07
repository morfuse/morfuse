#include <morfuse/Script/StateScript.h>
#include <morfuse/Script/ScriptClass.h>
#include <morfuse/Script/ScriptException.h>
#include <morfuse/Script/ProgramScript.h>
#include <morfuse/Script/ScriptMaster.h>
#include <morfuse/Script/Context.h>
#include <morfuse/Script/Archiver.h>

using namespace mfuse;

StateScript::StateScript(ProgramScript* inParent)
{
	m_Parent = inParent;
	label_list.getAllocator().SetAllocator(inParent->GetAllocator());
}

StateScript::~StateScript()
{

}

#if 0
template<>
void con::Entry<const_str, script_label_t>::Archive(Archiver& arc)
{
	/*
	unsigned int offset;

	arc.ArchiveConstString(key);

	if (arc.Saving())
	{
		offset = value.codepos - current_progBuffer;
		arc.ArchiveUnsigned(&offset);
	}
	else
	{
		arc.ArchiveUnsigned(&offset);
		value.codepos = current_progBuffer + offset;
		value.key = key;
	}

	arc.ArchiveBool(&value.isprivate);
	*/
}
#endif

void StateScript::Archive(Archiver& arc)
{
	//label_list.Archive(arc);
}

bool StateScript::AddLabel(const_str label, const opval_t *pos, bool private_section)
{
	if (label_list.findKeyValue(label)) {
		return false;
	}

	script_label_t &s = label_list.addKeyValue(label);

	s.codepos = pos;
	//s.key = label;
	s.isprivate = private_section;

	//reverse_label_list.AddObject(&s);

	return true;
}

bool StateScript::AddLabel(const rawchar_t* label, const opval_t *pos, bool private_section)
{
	return AddLabel(ScriptContext::Get().GetDirector().AddString(label), pos, private_section);
}

const opval_t *StateScript::FindLabel(const_str label) const
{
	const script_label_t* s = label_list.findKeyValue(label);

	if (s)
	{
		// check if the label is a private function
		if (s->isprivate)
		{
			const ScriptClass* scriptClass = ScriptContext::Get().GetDirector().CurrentScriptClass();

			if (scriptClass)
			{
				const ProgramScript* script = scriptClass->GetScript();

				// now check if the label's statescript matches this statescript
				if (&script->GetStateScript() != this)
				{
					ScriptError("Cannot call a private function.");
					return NULL;
				}
			}
		}

		return s->codepos;
	}
	else
	{
		return NULL;
	}
}

const opval_t *StateScript::FindLabel(const rawchar_t* label) const
{
	return FindLabel(ScriptContext::Get().GetDirector().GetString(label));
}

ProgramScript* StateScript::GetParent() const
{
	return m_Parent;
}

void StateScript::Reserve(size_t count)
{
	label_list.resize(count);
	//reverse_label_list.Resize(count);
}
