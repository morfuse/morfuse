#include <morfuse/Common/ConstStr.h>
#include <morfuse/Script/ScriptThreadLabel.h>
#include <morfuse/Script/ScriptThread.h>
#include <morfuse/Script/ScriptClass.h>
#include <morfuse/Script/ScriptException.h>
#include <morfuse/Script/ScriptVariable.h>
#include <morfuse/Script/Level.h>
#include <morfuse/Script/ProgramScript.h>
#include <morfuse/Script/StateScript.h>
#include <morfuse/Script/Context.h>
#include <morfuse/Script/ScriptMaster.h>
#include <morfuse/Script/PredefinedString.h>

using namespace mfuse;

ScriptThreadLabel::ScriptThreadLabel()
{
	m_Script = nullptr;
	m_Label = ConstStrings::Empty;
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
	if (!m_Script) {
		return;
	}

	ScriptThread* thread = Create(listener);

	if (thread)
	{
		thread->Execute(ev);
	}
}

void ScriptThreadLabel::Set(const rawchar_t *label)
{
	str script;
	rawchar_t buffer[1023];
	rawchar_t *p = buffer;
	bool foundLabel = false;

	if (!label || !*label)
	{
		m_Script = NULL;
		m_Label = ConstStrings::Empty;
		return;
	}

	strcpy(buffer, label);

	while (*p != '\0')
	{
		if (p[0] == ':' && p[1] == ':')
		{
			*p = '\0';

			script = buffer;
			m_Label = ScriptContext::Get().GetDirector().GetDictionary().Add(&p[2]);
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
		m_Label = director.GetDictionary().Add(buffer);
	}

	m_Script = director.GetProgramScript(script.c_str());

	if (!m_Script->GetStateScript().FindLabel(m_Label))
	{
		const str& l = director.GetDictionary().Get(m_Label);

		m_Script = NULL;
		m_Label = ConstStrings::Empty;

		throw ScriptException("^~^~^ Could not find label '" + l + "' in '" + script + "'");
	}
}

void ScriptThreadLabel::SetScript(const ScriptVariable& label)
{
	if (label.GetType() == variableType_e::String || label.GetType() == variableType_e::ConstString)
	{
		m_Script = ScriptContext::Get().GetDirector().GetProgramScript(label.stringValue());
		m_Label = ConstStrings::Empty;
	}
	else if (label.GetType() == variableType_e::ConstArray && label.arraysize() > 1)
	{
		const ScriptVariable& script = label[1];
		const ScriptVariable& labelname = label[2];

		m_Script = ScriptContext::Get().GetDirector().GetProgramScript(script.constStringValue());
		m_Label = labelname.constStringValue();

		if (!m_Script->GetStateScript().FindLabel(m_Label))
		{
			m_Script = nullptr;
			m_Label = ConstStrings::Empty;

			throw ScriptException("^~^~^ Could not find label '" + labelname.stringValue() + "' in '" + script.stringValue() + "'");
		}
	}
	else
	{
		throw ScriptException("bad label type '" + str(label.GetTypeName()) + "'");
	}
}

void ScriptThreadLabel::SetScript(const rawchar_t *label)
{
	Set(label);
}

void ScriptThreadLabel::SetThread(const ScriptVariable& label)
{
	if (label.GetType() == variableType_e::String || label.GetType() == variableType_e::ConstString)
	{
		m_Script = ScriptContext::Get().GetDirector().CurrentScriptClass()->GetScript();
		m_Label = label.constStringValue();
	}
	else if (label.GetType() == variableType_e::ConstArray && label.arraysize() > 1)
	{
		const ScriptVariable& script = label[1];
		const ScriptVariable& labelname = label[2];

		m_Script = ScriptContext::Get().GetDirector().GetProgramScript(script.constStringValue());
		m_Label = labelname.constStringValue();

		if (!m_Script->GetStateScript().FindLabel(m_Label))
		{
			m_Script = nullptr;
			m_Label = ConstStrings::Empty;

			throw ScriptException("^~^~^ Could not find label '" + labelname.stringValue() + "' in '" + script.stringValue() + "'");
		}
	}
	else
	{
		throw ScriptException("bad label type '" + str(label.GetTypeName()) + "'");
	}
}

bool ScriptThreadLabel::TrySet(const rawchar_t *label)
{
	try
	{
		Set(label);
	}
	catch (const std::exception& e)
	{
		const ScriptContext& context = ScriptContext::Get();
		std::ostream* out = context.GetOutputInfo().GetOutput(outputLevel_e::Error);
		*out << e.what();
		return false;
	}

	return true;
}

bool ScriptThreadLabel::TrySet(const_str label)
{
	return TrySet(ScriptContext::Get().GetDirector().GetDictionary().Get(label).c_str());
}

bool ScriptThreadLabel::TrySetScript(const rawchar_t *label)
{
	try
	{
		SetScript(label);
	}
	catch (const std::exception& e)
	{
		const ScriptContext& context = ScriptContext::Get();
		std::ostream* out = context.GetOutputInfo().GetOutput(outputLevel_e::Error);
		*out << e.what();
		return false;
	}

	return true;
}

bool ScriptThreadLabel::TrySetScript(const_str label)
{
	return TrySetScript(ScriptContext::Get().GetDirector().GetDictionary().Get(label).c_str());
}

bool ScriptThreadLabel::IsSet(void)
{
	return m_Script != NULL ? true : false;
}

void ScriptThreadLabel::Archive(Archiver&)
{
	/*
	m_Script->Archive(arc);
	arc.ArchiveConstString(m_Label);
	*/
}

