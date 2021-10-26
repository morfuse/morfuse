#pragma once

#include "../Common/rawchar.h"
#include "../Common/Linklist.h"

namespace mfuse
{

#define MFUS_MODULE_DECLARATION(moduleName, description)

	class ModuleDef
	{
	public:
		mfuse_EXPORTS ModuleDef(const rawchar_t* moduleNameValue, const rawchar_t* descriptionValue);
		mfuse_EXPORTS ~ModuleDef();

		// non-copyable
		ModuleDef(const ModuleDef&) = delete;
		ModuleDef& operator=(const ModuleDef&) = delete;
		
		// non-movable
		ModuleDef(ModuleDef&&) = delete;
		ModuleDef& operator=(ModuleDef&&) = delete;

		mfuse_EXPORTS const rawchar_t* GetModuleName() const;
		mfuse_EXPORTS const rawchar_t* GetDescription() const;

	private:
		const rawchar_t* moduleName;
		const rawchar_t* description;
		ModuleDef* prev;
		ModuleDef* next;

		using List = LinkedList<ModuleDef*, &ModuleDef::prev, &ModuleDef::next>;
		static List head;
	};
}
