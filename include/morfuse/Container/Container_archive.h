#pragma once

#include "Container.h"
#include "../Script/Archiver.h"

namespace mfuse
{
	namespace con
	{
	template<class Type>
	using ContainerArchiveFunc = void(*)(Archiver& arc, Type& obj);

	template<class Type, class Allocator>
	void Archive(Archiver& arc, Container<Type, Allocator>& container, ContainerArchiveFunc<Type> ArchiveFunc)
	{
		uint32_t num;

		if (arc.Loading())
		{
			arc.ArchiveUInt32(num);
			container.SetNumObjects(num);
		}
		else
		{
			num = (uint32_t)container.NumObjects();
			arc.ArchiveUInt32(num);
		}

		for (uint32_t i = 1; i <= num; i++)
		{
			ArchiveFunc(arc, container.ObjectAt(i));
		}
	}
	}
}