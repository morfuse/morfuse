#pragma once

#include "ConstStr.h"
#include "../Common/SafePtr.h"
#include "../Common/rawchar.h"
#include "../Container/Container.h"
#include "../Container/set.h"
#include <cstdint>

namespace mfuse
{
	class SimpleEntity;
	using ConSimple = con::Container<SafePtr<SimpleEntity>>;

	class TargetList
	{
	public:
		void AddTargetEntity(SimpleEntity* ent);
		void AddTargetEntityAt(SimpleEntity* ent, int index);
		void RemoveTargetEntity(SimpleEntity* ent);

		void FreeTargetList();

		SimpleEntity* GetNextEntity(const rawchar_t* targetname, SimpleEntity* ent);
		SimpleEntity* GetNextEntity(const_str targetname, SimpleEntity* ent);
		SimpleEntity* GetScriptTarget(const rawchar_t* targetname);
		SimpleEntity* GetScriptTarget(const_str targetname);
		SimpleEntity* GetTarget(const rawchar_t* targetname, bool quiet);
		SimpleEntity* GetTarget(const_str targetname, bool quiet);
		uintptr_t GetTargetnameIndex(SimpleEntity* ent);

		ConSimple* GetExistingTargetList(const rawchar_t* targetname);
		ConSimple* GetExistingTargetList(const_str targetname);
		ConSimple* GetTargetList(const rawchar_t* targetname);
		ConSimple* GetTargetList(const_str targetname);

	private:
		con::set<const_str, ConSimple> m_targetList;
	};
}