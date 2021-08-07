#pragma once

#include "../Global.h"

#include <cstddef>

namespace mfuse
{
	class SafePtrBase;

	/**
	 * The abstract class is the base abstraction for all classes
	 * that are intended to be publicly used.
	 */
	class mfuse_EXPORTS AbstractClass
	{
		friend class SafePtrBase;

	public:
		AbstractClass();
		virtual ~AbstractClass();

		static void* operator new(size_t size);
		static void* operator new[](size_t size);
		static void operator delete(void* ptr);
		static void operator delete[](void* ptr);

		template<typename T>
		static void* operator new(size_t size, T& placement)
		{
			return ::operator new(size, placement);
		}

		template<typename T>
		static void operator delete(void* ptr, T& placement)
		{}

		static void* operator new(size_t size, void* placement);
		static void operator delete(void* ptr, void* placement);

	private:
		SafePtrBase* SafePtrList;
	};
}