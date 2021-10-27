#pragma once

#include <cstdint>

namespace mfuse
{
namespace con
{
	template<typename T, typename AllocatorT>
	class Container;

	/**
	 * A class used to refer to a contiguous sequence of objects.
	 */
	template<typename T>
	class ContainerView
	{
	public:
		/**
		 * Construct an empty array view.
		 */
		ContainerView();

		/**
		 * Construct with an array of objects and the number of elements.
		 *
		 * @param objlistValue The object list.
		 * @param arraysizeValue The number of elements in list.
		 */
		ContainerView(T* objlistValue, size_t arraysizeValue);
		/**
		 * Construct with an existing container
		 *
		 * @param container The container to use.
		 */
		ContainerView(const Container<T>& container);
		/**
		 * Construct with an existing container
		 *
		 * @param container The container to use.
		 */
		template<size_t N>
		ContainerView(T (&objlistValue)[N]);

		/** Return the number of objects in list. */
		size_t NumObjects() const;

		/**
		 * Return the object at the specified element number.
		 *
		 * @param elementNum An element number, starting from 1 like the Container class.
		 */
		T& ObjectAt(uintptr_t elementNum) const;

		/**
		 * Return the object at the specified index.
		 *
		 * @param index The index of the object in the array.
		 */
		T& operator[](uintptr_t index) const;
		
	private:
		T* objlist;
		size_t arraysize;
	};

	template<typename T>
	ContainerView<T>::ContainerView()
		: objlist(nullptr)
		, arraysize(0)
	{
	}

	template<typename T>
	ContainerView<T>::ContainerView(T* objlistValue, size_t arraysizeValue)
		: objlist(objlistValue)
		, arraysize(arraysizeValue)
	{
	}
	
	template<typename T>
	ContainerView<T>::ContainerView(const Container<T>& container)
		: objlist(container.objlist)
		, arraysize(container.numobjects)
	{
	}

	template<typename T>
	template<size_t N>
	mfuse::con::ContainerView<T>::ContainerView(T (&objlistValue)[N])
		: ContainerView(objlistValue, N)
	{
	}

	template<typename T>
	size_t ContainerView<T>::NumObjects() const
	{
		return arraysize;
	}

	template<typename T>
	T& ContainerView<T>::ObjectAt(uintptr_t elementNum) const
	{
		return objlist[elementNum - 1];
	}

	template<typename T>
	T& ContainerView<T>::operator[](uintptr_t index) const
	{
		return objlist[index];
	}
};
}