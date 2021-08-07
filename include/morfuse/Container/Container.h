#pragma once

#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <utility>
#include <exception>
#include "../Common/MEM/Memory.h"
#include "../Common/MEM/DefaultAlloc.h"

#undef new

namespace mfuse
{
	class Archiver;

	namespace con
	{
	class ContainerException : public std::exception
	{
	};

	class OutOfRangeContainerException : public ContainerException
	{
	private:
		size_t badIndex;

	public:
		OutOfRangeContainerException(size_t inBadIndex) noexcept
			: badIndex(inBadIndex)
		{}

		size_t getBadIndex() const noexcept { return badIndex; }
	};

	/**
	 * A container is a dynamically allocated array of elements.
	 * 
	 * @tparam Type The type of element.
	 * @tparam Allocator The allocator type.
	 *   The allocator must contain Alloc(size_t elemsize) and Free(void* ptr). They are self-explanatory.
	 */
	template<class Type, class Allocator = MEM::DefaultAlloc>
	class Container
	{
	public:
		Container() noexcept;
		Container(size_t initialSize);
		Container(const Container& container);
		Container(Container&& container) noexcept;
		~Container();

		template<typename...Args>
		uintptr_t			AddObject(Args&&...args);
		uintptr_t			AddObject();
		uintptr_t			AddObjectUninitialized();
		uintptr_t			AddUniqueObject(const Type& obj);
		void				AddObjectAt(size_t index, const Type& obj);
		Type				*AddressOfObjectAt(size_t index);
		Type				*Data() noexcept;
		const Type			*Data() const noexcept;
		void				ClearObjectList();
		void				FreeObjectList();
		uintptr_t			IndexOfObject(const Type& obj) noexcept;
		void				InsertObjectAt(size_t index, const Type& obj);
		uintptr_t			MaxObjects() const noexcept;
		uintptr_t			NumObjects() const noexcept;
		Type&				ObjectAt(const size_t index) const;
		bool				ObjectInList(const Type& obj);
		void				RemoveObjectAt(uintptr_t index);
		void				RemoveObject(const Type& obj);
		void				RemoveObject(const Type* obj);
		void				Resize(size_t maxelements);
		void				SetNumObjects(size_t numelements);
		void				SetNumObjectsUninitialized(size_t numelements);
		void				SetObjectAt(size_t index, const Type& obj);
		void				Shrink();
		void				Sort(int(*compare)(const void* elem1, const void* elem2));
		Allocator&			GetAllocator();
		Type&				operator[](const size_t index) const;
		Container&	operator=(const Container& container);
		Container&	operator=(Container&& container) noexcept;

		// STL functions
		Type* begin() noexcept { return objlist; }
		const Type* begin() const noexcept { return objlist; }
		Type* end() noexcept { return objlist + numobjects; }
		const Type* end() const noexcept { return objlist + numobjects; }
		Type* data()  noexcept { return objlist; }
		const Type* data() const noexcept { return objlist; }
		size_t size() const noexcept { return numobjects; }
		const Type& at(const size_t index) const { return ObjectAt(index + 1); }
		Type& at(const size_t index) { return ObjectAt(index + 1); }
		void push_back(const Type& obj) { AddObject(obj); }
		void clear() noexcept { ClearObjectList(); }
		void reserve(size_t newSize) { Resize(newSize); }
		void resize(size_t newSize) { SetNumObjects(newSize); }
		void shrink_to_fit() { Shrink(); }

	protected:
		Allocator Object_allocator;
		Type* objlist;
		size_t numobjects;
		size_t maxobjects;

	private:
		void Copy(const Container& container);
	};

	template<class Type, class Allocator>
	Container<Type, Allocator>::Container() noexcept
	{
		objlist = nullptr;
		numobjects = 0;
		maxobjects = 0;
	}

	template<class Type, class Allocator>
	Container<Type, Allocator>::Container(size_t initialSize)
		: Container<Type, Allocator>()
	{
		Resize(initialSize);
	}

	template<class Type, class Allocator>
	Container<Type, Allocator>::Container(const Container& container)
	{
		objlist = nullptr;

		Copy(container);
	}

	template<class Type, class Allocator>
	Container<Type, Allocator>::Container(Container&& container) noexcept
	{
		objlist = container.objlist;
		numobjects = container.numobjects;
		maxobjects = container.maxobjects;

		// reset the old container
		container.objlist = nullptr;
		container.numobjects = 0;
		container.maxobjects = 0;
	}

	template<class Type, class Allocator>
	Container<Type, Allocator>::~Container()
	{
		FreeObjectList();
	}

	template<class Type, class Allocator>
	template<typename...Args>
	uintptr_t Container<Type, Allocator>::AddObject(Args&&... args)
	{
		if (numobjects >= maxobjects) {
			Resize((numobjects + 1) * 2);
		}

		new(&objlist[numobjects++]) Type(std::forward<Args>(args)...);

		return numobjects;
	}

	template<class Type, class Allocator>
	uintptr_t Container<Type, Allocator>::AddObject()
	{
		const uintptr_t index = AddObjectUninitialized() - 1;
		new (objlist + index) Type();
		return index;
	}

	template<class Type, class Allocator>
	uintptr_t Container<Type, Allocator>::AddObjectUninitialized()
	{
		if (!objlist) {
			Resize(10);
		}

		if (numobjects >= maxobjects) {
			Resize(numobjects * 2);
		}

		++numobjects;
		return numobjects;
	}

	template<class Type, class Allocator>
	uintptr_t Container<Type, Allocator>::AddUniqueObject(const Type& obj)
	{
		uintptr_t index = IndexOfObject(obj);

		if (!index)
		{
			index = AddObject(obj);
		}

		return index;
	}

	template<class Type, class Allocator>
	void Container<Type, Allocator>::AddObjectAt(size_t index, const Type& obj)
	{
		if (index > maxobjects)
			Resize(index);

		if (index > numobjects)
			numobjects = index;

		SetObjectAt(index, obj);
	}

	template<class Type, class Allocator>
	Type *Container<Type, Allocator>::AddressOfObjectAt(size_t index)
	{
		assert(index <= maxobjects);

		if (index > maxobjects) {
			//CONTAINER_Error(ERR_DROP, "Container::AddressOfObjectAt : index is greater than maxobjects");
		}

		if (index > numobjects) {
			numobjects = index;
		}

		return &objlist[index - 1];
	}

	template<class Type, class Allocator>
	void Container<Type, Allocator>::ClearObjectList()
	{
		if (objlist && numobjects)
		{

			for (size_t i = 0; i < numobjects; ++i) {
				objlist[i].~Type();
			}

			// don't free the object list, it can be reused later
			/*
			Object_allocator.Free(objlist);

			if (maxobjects == 0)
			{
				objlist = nullptr;
				return;
			}

			objlist = (Type*)Object_allocator.Alloc(sizeof(Type) * maxobjects);
			*/
			numobjects = 0;
		}
	}

	template<class Type, class Allocator>
	Type* Container<Type, Allocator>::Data() noexcept
	{
		return objlist;
	}

	template<class Type, class Allocator>
	const Type* Container<Type, Allocator>::Data() const noexcept
	{
		return objlist;
	}

	template<class Type, class Allocator>
	void Container<Type, Allocator>::FreeObjectList()
	{
		if (objlist)
		{
			for (size_t i = 0; i < numobjects; ++i) {
				objlist[i].~Type();
			}

			Object_allocator.Free(objlist);
		}

		objlist = nullptr;
		numobjects = 0;
		maxobjects = 0;
	}

	template<class Type, class Allocator>
	uintptr_t Container<Type, Allocator>::IndexOfObject(const Type& obj) noexcept
	{
		if (!objlist) {
			return 0;
		}

		const Type* start = objlist;
		const Type* end = objlist + numobjects;
		for (const Type* data = start; data != end; ++data)
		{
			if (*data == obj) {
				return static_cast<uintptr_t>(data - start) + 1;
			}
		}

		return 0;
	}

	template<class Type, class Allocator>
	void Container<Type, Allocator>::InsertObjectAt(size_t index, const Type& obj)
	{
		if ((index <= 0) || (index > numobjects + 1))
		{
			//CONTAINER_Error(ERR_DROP, "Container::InsertObjectAt : index out of range");
			return;
		}

		numobjects++;
		intptr_t arrayIndex = index - 1;

		if (numobjects > maxobjects)
		{
			maxobjects = numobjects;
			if (!objlist)
			{
				objlist = Object_allocator.Alloc(sizeof(Type) * maxobjects);
				for (size_t i = 0; i < arrayIndex; ++i) {
					new(objlist + i) Type();
				}

				objlist[arrayIndex] = obj;
				return;
			}
			else
			{
				Type *temp = objlist;
				if (maxobjects < numobjects) {
					maxobjects = numobjects;
				}

				objlist = Object_allocator.Alloc(sizeof(Type) * maxobjects);

				for (intptr_t i = 0; i < arrayIndex; ++i) {
					new(objlist + i) Type(std::move_if_noexcept(temp[i]));
				}

				new(objlist + arrayIndex) Type(obj);
				for (intptr_t i = arrayIndex + 1; i < numobjects; ++i) {
					new(objlist + i) Type(std::move_if_noexcept(temp[i]));
				}

				Object_allocator.Free(temp);
			}
		}
		else
		{
			for (int i = numobjects - 1; i > arrayIndex; i--) {
				objlist[i] = objlist[i - 1];
			}
			objlist[arrayIndex] = obj;
		}
	}

	template<class Type, class Allocator>
	uintptr_t Container<Type, Allocator>::MaxObjects() const noexcept
	{
		return maxobjects;
	}

	template<class Type, class Allocator>
	uintptr_t Container<Type, Allocator>::NumObjects() const noexcept
	{
		return numobjects;
	}

	template<class Type, class Allocator>
	Type& Container<Type, Allocator>::ObjectAt(const size_t index) const
	{
		assert(index > 0 && index <= numobjects);
		/*
		if ((index <= 0) || (index > numobjects)) {
			throw OutOfRangeContainerException(index);
		}
		*/

		return objlist[index - 1];
	}

	template<class Type, class Allocator>
	bool Container<Type, Allocator>::ObjectInList(const Type& obj)
	{
		if (!IndexOfObject(obj)) {
			return false;
		}

		return true;
	}

	template<class Type, class Allocator>
	void Container<Type, Allocator>::RemoveObjectAt(uintptr_t index)
	{
		uintptr_t i;

		if ((index <= 0) || (index > numobjects)) {
			throw OutOfRangeContainerException(index);
		}

		i = index - 1;
		numobjects--;

		for (i = index - 1; i < numobjects; i++) {
			objlist[i] = std::move_if_noexcept(objlist[i + 1]);
		}

		// Destroy the last object as it's now useless
		objlist[numobjects].~Type();
	}

	template<class Type, class Allocator>
	void Container<Type, Allocator>::RemoveObject(const Type& obj)
	{
		uintptr_t index = IndexOfObject(obj);
		if (!index) {
			return;
		}

		RemoveObjectAt(index);
	}

	template<class Type, class Allocator>
	void Container<Type, Allocator>::RemoveObject(const Type* obj)
	{
		uintptr_t index = obj - objlist;
		if (index > numobjects) {
			return;
		}

		RemoveObjectAt(index + 1);
	}

	template<class Type, class Allocator>
	void Container<Type, Allocator>::Resize(size_t maxelements)
	{
		if (maxelements <= 0)
		{
			FreeObjectList();
			return;
		}

		if (!objlist)
		{
			maxobjects = maxelements;
			// allocate without initializing
			objlist = (Type*)Object_allocator.Alloc(sizeof(Type) * maxobjects);
		}
		else
		{
			Type* temp = objlist;

			maxobjects = maxelements;

			if (maxobjects < numobjects) {
				maxobjects = numobjects;
			}

			// allocate without initializing
			objlist = (Type*)Object_allocator.Alloc(maxobjects * sizeof(Type));

			for (size_t i = 0; i < numobjects; i++)
			{
				// move the older type
				new(objlist + i) Type(std::move_if_noexcept(temp[i]));

				// destruct the older type
				temp[i].~Type();
			}

			Object_allocator.Free(temp);
		}
	}

	template<class Type, class Allocator>
	void Container<Type, Allocator>::SetNumObjects(size_t numelements)
	{
		if (numelements > maxobjects) Resize(numelements);

		const size_t startNum = numobjects;
		numobjects = numelements;
		for (size_t i = startNum; i < numobjects; ++i) {
			new(objlist + i) Type();
		}
	}

	template<class Type, class Allocator>
	void Container<Type, Allocator>::SetNumObjectsUninitialized(size_t numelements)
	{
		if (numelements > maxobjects) Resize(numelements);
		numobjects = numelements;
	}

	template<class Type, class Allocator>
	void Container<Type, Allocator>::SetObjectAt(size_t index, const Type& obj)
	{
		assert(index > 0 && index <= numobjects);
		/*
		if ((index <= 0) || (index > numobjects)) {
			throw OutOfRangeContainerException(index);
		}
		*/

		objlist[index - 1] = obj;
	}

	template<class Type, class Allocator>
	void Container<Type, Allocator>::Shrink()
	{
		if (!objlist || !numobjects) {
			return;
		}

		Type* newlist = (Type*)Object_allocator.Alloc(sizeof(Type) * numobjects);

		for (size_t i = 0; i < numobjects; i++)
		{
			new(newlist + i) Type(std::move_if_noexcept(objlist[i]));
			objlist[i].~Type();
		}

		Object_allocator.Free(objlist);
		objlist = newlist;
		maxobjects = numobjects;
	}

	template<class Type, class Allocator>
	void Container<Type, Allocator>::Sort(int(*compare)(const void *elem1, const void *elem2))

	{
		if (!objlist) {
			return;
		}

		qsort((void *)objlist, (size_t)numobjects, sizeof(Type), compare);
	}

	template<class Type, class Allocator>
	Allocator& Container<Type, Allocator>::GetAllocator()
	{
		return Object_allocator;
	}

	template<class Type, class Allocator>
	Type& Container<Type, Allocator>::operator[](const size_t index) const
	{
		return ObjectAt(index + 1);
	}

	template<class Type, class Allocator>
	void Container<Type, Allocator>::Copy(const Container& container)
	{
		if (&container == this) {
			return;
		}

		FreeObjectList();

		numobjects = container.numobjects;
		maxobjects = container.maxobjects;
		objlist = nullptr;

		if (container.objlist == nullptr || !container.maxobjects) {
			return;
		}

		Resize(maxobjects);

		if (!container.numobjects) {
			return;
		}

		for (size_t i = 0; i < container.numobjects; i++) {
			new(objlist + i) Type(container.objlist[i]);
		}

		return;
	}

	template<class Type, class Allocator>
	Container<Type, Allocator>& Container<Type, Allocator>::operator=(const Container& container)
	{
		Copy(container);

		return *this;
	}

	template<class Type, class Allocator>
	Container<Type, Allocator>& Container<Type, Allocator>::operator=(Container&& container) noexcept
	{
		// free the object list before moving
		FreeObjectList();
		objlist = container.objlist;
		numobjects = container.numobjects;
		maxobjects = container.maxobjects;

		// reset the old container
		container.objlist = nullptr;
		container.numobjects = 0;
		container.maxobjects = 0;
		return *this;
	}

	template<typename Archive, typename T, typename Allocator>
	Archive& operator<<(Archive& ar, const Container<T, Allocator>& container)
	{
		const size_t numObjects = container.NumObjects();
		ar << numObjects;
		
		for (size_t i = 0; i < numObjects; i++)
		{
			T& obj = container[i];
			ar << obj;
		}

		return ar;
	}

	template<typename Archive, typename T, typename Allocator>
	Archive& operator>>(Archive& ar, Container<T, Allocator>& container)
	{
		size_t numObjects;
		ar >> numObjects;

		container.SetNumObjectsUninitialized(numObjects);

		for (size_t i = 0; i < numObjects; i++)
		{
			T* obj = new(container) T();
			ar >> *obj;
		}

		return ar;
	}
	}
};

template<typename T, typename Allocator>
void* operator new(size_t count, mfuse::con::Container<T, Allocator>& container)
{
	assert(count == sizeof(T));
	return &container.ObjectAt(
		container.AddObjectUninitialized()
	);
}

template<typename T, typename Allocator>
void operator delete(void* ptr, mfuse::con::Container<T, Allocator>& container)
{
	container.RemoveObject((T*)ptr);
}

