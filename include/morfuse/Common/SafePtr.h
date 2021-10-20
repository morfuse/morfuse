#pragma once

#include "../Global.h"
#include "Linklist.h"
#include <cassert>
#include <mutex>
#include <cstdint>

namespace mfuse
{
	class AbstractClass;

	class mfuse_PUBLIC SafePtrBase
	{
	public:
		mfuse_EXPORTS SafePtrBase() noexcept;
		mfuse_EXPORTS SafePtrBase(AbstractClass* initial) noexcept;
		mfuse_EXPORTS virtual ~SafePtrBase();
		mfuse_EXPORTS void InitSafePtr(AbstractClass* newptr) noexcept;
		mfuse_EXPORTS AbstractClass* Pointer() const noexcept;
		mfuse_EXPORTS void Clear() noexcept;
		mfuse_EXPORTS bool IsLastReference() const noexcept;
		mfuse_EXPORTS bool Valid() const noexcept;

	private:
		mfuse_LOCAL void AddReference(AbstractClass* ptr) noexcept;
		mfuse_LOCAL void RemoveReference(AbstractClass* ptr) noexcept;

	private:
		SafePtrBase* prev;
		SafePtrBase* next;
		AbstractClass* ptr;
	};

	template<class T>
	class SafePtr : public SafePtrBase
	{
	public:
		SafePtr() noexcept;
		SafePtr(T* objptr) noexcept;
		SafePtr(const SafePtr& obj) noexcept;

		SafePtr& operator=(const SafePtr& obj) noexcept;
		SafePtr& operator=(T* obj) noexcept;

		T* Pointer() const noexcept;

		template<class U> friend bool operator==(const SafePtr<U>& a, U* b) noexcept;
		template<class U> friend bool operator!=(const SafePtr<U>& a, U* b) noexcept;
		template<class U> friend bool operator==(U* a, const SafePtr<U>& b) noexcept;
		template<class U> friend bool operator!=(U* a, const SafePtr<U>& b) noexcept;
		template<class U> friend bool operator==(const SafePtr<U>& a, const SafePtr<U>& b) noexcept;
		template<class U> friend bool operator!=(const SafePtr<U>& a, const SafePtr<U>& b) noexcept;

		bool operator !() const noexcept;
		operator T* () const noexcept;
		T* operator->() const noexcept;
		T& operator*() const noexcept;
	};

	template<class T>
	SafePtr<T>::SafePtr() noexcept
		: SafePtrBase()
	{
	}

	template<class T>
	SafePtr<T>::SafePtr(T* objptr) noexcept
		: SafePtrBase(objptr)
	{
	}

	template<class T>
	SafePtr<T>::SafePtr(const SafePtr& obj) noexcept
		: SafePtrBase(obj.Pointer())
	{
	}

	template<class T>
	T* SafePtr<T>::Pointer() const noexcept
	{
		return (T*)SafePtrBase::Pointer();
	}

	template<class T>
	SafePtr<T>& SafePtr<T>::operator=(const SafePtr& obj) noexcept
	{
		InitSafePtr(obj.Pointer());
		return *this;
	}

	template<class T>
	SafePtr<T>& SafePtr<T>::operator=(T* obj) noexcept
	{
		InitSafePtr((AbstractClass*)obj);
		return *this;
	}

	template<class U>
	bool operator==(const SafePtr<U>& a, U* b) noexcept
	{
		return a.Pointer() == b;
	}

	template<class U>
	bool operator!=(const SafePtr<U>& a, U* b) noexcept
	{
		return a.Pointer() != b;
	}

	template<class U>
	bool operator==(U* a, const SafePtr<U>& b) noexcept
	{
		return a == b.Pointer();
	}

	template<class U>
	bool operator!=(U* a, const SafePtr<U>& b) noexcept
	{
		return a != b.Pointer();
	}

	template<class U>
	bool operator==(const SafePtr<U>& a, const SafePtr<U>& b) noexcept
	{
		return a.Pointer() == b.Pointer();
	}

	template<class U>
	bool operator!=(const SafePtr<U>& a, const SafePtr<U>& b) noexcept
	{
		return a.Pointer() != b.Pointer();
	}

	template<class T>
	bool SafePtr<T>::operator !() const noexcept
	{
		return !Pointer();
	}

	template<class T>
	SafePtr<T>::operator T* () const noexcept
	{
		return static_cast<T*>(Pointer());
	}

	template<class T>
	T* SafePtr<T>::operator->() const noexcept
	{
		assert(Pointer());
		return static_cast<T*>(Pointer());
	}

	template<class T>
	T& SafePtr<T>::operator*() const noexcept
	{
		assert(Pointer());
		return static_cast<T*>(Pointer());
	}
}
