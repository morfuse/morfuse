#pragma once

#include "../Common/rawchar.h"

namespace mfuse
{
	class ModuleDef
	{
	public:
		ModuleDef(const rawchar_t* moduleNameValue);
		ModuleDef(const ModuleDef&) = delete;
		ModuleDef& operator=(const ModuleDef&) = delete;
		ModuleDef(ModuleDef&&) = delete;
		ModuleDef& operator=(ModuleDef&&) = delete;

		const rawchar_t* GetModuleName() const;
		
	private:
		const rawchar_t* moduleName;
	};
}
