#pragma once

#include "../Container/Container.h"

namespace mfuse
{
	class Archiver;

	namespace con
	{
	template<class Type>
	class ContainerClass : public Container<Type>, public Class {
	public:
		using Container<Type>::Container;

		virtual void Archive(Archiver& arc);
		void Archive(Archiver& arc, void(*ArchiveFunc)(Archiver& arc, Type& obj));
	};

	template<class Type>
	void ContainerClass<Type>::Archive(Archiver& arc, void(*ArchiveFunc)(Archiver& arc, Type& obj))
	{
		for(size_t i = 0; i < this->numobjects; ++i)
		{
			ArchiveFunc(arc, this->objlist[i]);
		}
	}
	}
};
