#include <morfuse/Script/ScriptThreadLabel.h>
#include <morfuse/Script/ScriptThread.h>
#include <morfuse/Script/ScriptClass.h>
#include <morfuse/Script/ScriptException.h>
#include <morfuse/Script/ScriptVariable.h>
#include <morfuse/Script/Level.h>
#include <morfuse/Script/ConstStr.h>
#include <morfuse/Script/ProgramScript.h>
#include <morfuse/Script/StateScript.h>
#include <morfuse/Script/Context.h>
#include <morfuse/Script/ScriptMaster.h>

using namespace mfuse;

ScriptThreadLabel::ScriptThreadLabel()
{
	m_Script = NULL;
	m_Label = STRING_EMPTY;
}

ScriptThread *ScriptThreadLabel::Create(Listener *listener)
{
	if (!m_Script)
	{
		return NULL;
	}

	ScriptClass *scriptClass = new ScriptClass(m_Script, listener);
	ScriptThread *thread = ScriptContext::Get().GetDirector().CreateScriptThread(scriptClass, m_Label);

	return thread;
}

void ScriptThreadLabel::Execute(Listener *listener)
{
	if (!m_Script) {
		return;
	}

	ScriptThread *thread = Create(listener);

	if (thread)
	{
		thread->Execute();
	}
}

void ScriptThreadLabel::Execute(Listener *listener, Event &ev)
{
	Execute(listener, &ev);
}

void ScriptThreadLabel::Execute(Listener *listener, Event *ev)
{
	if (!m_Script) {
		return;
	}

	ScriptThread *thread = Create(listener);

	if (thread)
	{
		thread->Execute(ev);
	}
}

void ScriptThreadLabel::Set(const rawchar_t *label)
{
	xstr script;
	rawchar_t buffer[1023];
	rawchar_t *p = buffer;
	bool foundLabel = false;

	if (!label || !*label)
	{
		m_Script = NULL;
		m_Label = STRING_EMPTY;
		return;
	}

	strcpy(buffer, label);

	while (*p != '\0')
	{
		if (p[0] == ':' && p[1] == ':')
		{
			*p = '\0';

			script = buffer;
			m_Label = ScriptContext::Get().GetDirector().AddString(&p[2]);
			foundLabel = true;

			break;
		}

		p++;
	}

	ScriptContext& context = ScriptContext::Get();
	ScriptMaster& director = context.GetDirector();

	if (!foundLabel)
	{
		script = context.GetLevel()->GetCurrentScript();
		m_Label = director.AddString(buffer);
	}

	m_Script = director.GetGameScript(script.c_str());

	if (!m_Script->GetStateScript().FindLabel(m_Label))
	{
		const xstr& l = director.GetString(m_Label);

		m_Script = NULL;
		m_Label = STRING_EMPTY;

		ScriptError("^~^~^ Could not find label '%s' in '%s'", l.c_str(), script.c_str());
	}
}

void ScriptThreadLabel::SetScript(const ScriptVariable& label)
{
	if (label.GetType() == variableType_e::String || label.GetType() == variableType_e::ConstString)
	{
		m_Script = ScriptContext::Get().GetDirector().GetGameScript(label.stringValue());
		m_Label = STRING_EMPTY;
	}
	else if (label.GetType() == variableType_e::ConstArray && label.arraysize() > 1)
	{
		ScriptVariable *script = label[1];
		ScriptVariable *labelname = label[2];

		m_Script = ScriptContext::Get().GetDirector().GetGameScript(script->stringValue());
		m_Label = labelname->constStringValue();

		if (!m_Script->GetStateScript().FindLabel(m_Label))
		{
			m_Script = NULL;
			m_Label = STRING_EMPTY;

			ScriptError("^~^~^ Could not find label '%s' in '%s'", labelname->stringValue().c_str(), script->stringValue().c_str());
		}
	}
	else
	{
		ScriptError("ScriptThreadLabel::SetScript: bad label type '%s'", label.GetTypeName());
	}
}

void ScriptThreadLabel::SetScript(const rawchar_t *label)
{
	Set(label);
}

void ScriptThreadLabel::SetThread(const ScriptVariable& label)
{
	ScriptThread *thread = NULL;

	if (label.GetType() == variableType_e::String || label.GetType() == variableType_e::ConstString)
	{
		m_Script = ScriptContext::Get().GetDirector().CurrentScriptClass()->GetScript();
		m_Label = label.constStringValue();
	}
	else if (label.GetType() == variableType_e::ConstArray && label.arraysize() > 1)
	{
		ScriptVariable *script = label[1];
		ScriptVariable *labelname = label[2];

		m_Script = ScriptContext::Get().GetDirector().GetGameScript(script->stringValue());
		m_Label = labelname->constStringValue();

		if (!m_Script->GetStateScript().FindLabel(m_Label))
		{
			m_Script = NULL;
			m_Label = STRING_EMPTY;

			ScriptError("^~^~^ Could not find label '%s' in '%s'", labelname->stringValue().c_str(), script->stringValue().c_str());
		}
	}
	else
	{
		ScriptError("ScriptThreadLabel::SetThread bad label type '%s'", label.GetTypeName());
	}
}

bool ScriptThreadLabel::TrySet(const rawchar_t *label)
{
	try
	{
		Set(label);
	}
	catch (const rawchar_t *string)
	{
		printf("%s\n", string);
		return false;
	}

	return true;
}

bool ScriptThreadLabel::TrySet(const_str label)
{
	return TrySet(ScriptContext::Get().GetDirector().GetString(label));
}

bool ScriptThreadLabel::TrySetScript(const rawchar_t *label)
{
	try
	{
		SetScript(label);
	}
	catch (const rawchar_t *string)
	{
		printf("%s\n", string);
		return false;
	}

	return true;
}

bool ScriptThreadLabel::TrySetScript(const_str label)
{
	return TrySetScript(ScriptContext::Get().GetDirector().GetString(label));
}

bool ScriptThreadLabel::IsSet(void)
{
	return m_Script != NULL ? true : false;
}

void ScriptThreadLabel::Archive(Archiver& arc)
{
	//m_Script->Archive(arc);
	//arc.ArchiveConstString(m_Label);
}

