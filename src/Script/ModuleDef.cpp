#include <morfuse/Script/ModuleDef.h>

using namespace mfuse;

ModuleDef::ModuleDef(const rawchar_t* moduleNameValue)
	: moduleName(moduleNameValue)
{
}

const mfuse::rawchar_t* ModuleDef::GetModuleName() const
{
	return moduleName;
}
