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

void StateScript::Archive(Archiver&)
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

const script_label_t*StateScript::FindLabel(const_str label) const
{
	const script_label_t* s = label_list.findKeyValue(label);

	if (s)
	{
		return s;
	}
	else
	{
		return nullptr;
	}
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

StateScriptErrors::LabelNotFound::LabelNotFound(const StringResolvable& labelValue, const StringResolvable& fileNameValue)
	: fileName(fileNameValue)
	, label(labelValue)
{
}

StateScriptErrors::LabelNotFound::~LabelNotFound()
{
}

const StringResolvable& StateScriptErrors::LabelNotFound::GetFileName() const noexcept
{
	return fileName;
}

const StringResolvable& StateScriptErrors::LabelNotFound::GetLabel() const noexcept
{
	return label;
}

const char* StateScriptErrors::LabelNotFound::what() const noexcept
{
	if (!filled()) {
		fill("label '" + label.GetString() + "' does not exist in '" + fileName.GetString() + "'");
	}

	return Messageable::what();
}
