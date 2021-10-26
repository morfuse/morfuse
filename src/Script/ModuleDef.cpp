#include <morfuse/Script/ModuleDef.h>

using namespace mfuse;

ModuleDef::List ModuleDef::head = ModuleDef::List();

ModuleDef::ModuleDef(const rawchar_t* moduleNameValue, const rawchar_t* descriptionValue)
	: moduleName(moduleNameValue)
	, description(descriptionValue)
{
	head.Add(this);
}

ModuleDef::~ModuleDef()
{
	head.Remove(this);
}

const rawchar_t* ModuleDef::GetModuleName() const
{
	return moduleName;
}

const rawchar_t* ModuleDef::GetDescription() const
{
	return description;
}
