#pragma once

#include "../Global.h"
#include "Linklist.h"
#include <cassert>
#include <mutex>
#include <cstdint>

namespace mfuse
{
	class AbstractClass;

	class SafePtrBase
	{
	public:
		mfuse_EXPORTS SafePtrBase();
		mfuse_EXPORTS SafePtrBase(AbstractClass* initial);
		mfuse_EXPORTS virtual ~SafePtrBase();
		mfuse_EXPORTS void InitSafePtr(AbstractClass* newptr);
		mfuse_EXPORTS AbstractClass* Pointer()  const;
		mfuse_EXPORTS void Clear(void);
		mfuse_EXPORTS int32_t GetRefCount() const;

	private:
		void AddReference(AbstractClass* ptr);
		void RemoveReference(AbstractClass* ptr);

	private:
		SafePtrBase* prev;
		SafePtrBase* next;
		AbstractClass* ptr;
	};

	template<class T>
	class SafePtr : public SafePtrBase
	{
	public:
		SafePtr() = default;
		SafePtr(T* objptr);
		SafePtr(const SafePtr& obj);

		SafePtr& operator=(const SafePtr& obj);
		SafePtr& operator=(T* obj);

		T* Pointer() const;

		template<class U> friend bool operator==(const SafePtr<U>& a, U* b);
		template<class U> friend bool operator!=(const SafePtr<U>& a, U* b);
		template<class U> friend bool operator==(U* a, const SafePtr<U>& b);
		template<class U> friend bool operator!=(U* a, const SafePtr<U>& b);
		template<class U> friend bool operator==(const SafePtr<U>& a, const SafePtr<U>& b);
		template<class U> friend bool operator!=(const SafePtr<U>& a, const SafePtr<U>& b);

		bool operator !() const;
		operator T* () const;
		T* operator->() const;
		T& operator*() const;
	};

	template<class T>
	SafePtr<T>::SafePtr(T* objptr)
		: SafePtrBase(objptr)
	{
	}

	template<class T>
	SafePtr<T>::SafePtr(const SafePtr& obj)
		: SafePtrBase(obj.Pointer())
	{
	}

	template<class T>
	T* SafePtr<T>::Pointer() const
	{
		return (T*)SafePtrBase::Pointer();
	}

	template<class T>
	SafePtr<T>& SafePtr<T>::operator=(const SafePtr& obj)
	{
		InitSafePtr(obj.Pointer());
		return *this;
	}

	template<class T>
	SafePtr<T>& SafePtr<T>::operator=(T* obj)
	{
		InitSafePtr((AbstractClass*)obj);
		return *this;
	}

	template<class U>
	bool operator==(const SafePtr<U>& a, U* b)
	{
		return a.Pointer() == b;
	}

	template<class U>
	bool operator!=(const SafePtr<U>& a, U* b)
	{
		return a.Pointer() != b;
	}

	template<class U>
	bool operator==(U* a, const SafePtr<U>& b)
	{
		return a == b.Pointer();
	}

	template<class U>
	bool operator!=(U* a, const SafePtr<U>& b)
	{
		return a != b.Pointer();
	}

	template<class U>
	bool operator==(const SafePtr<U>& a, const SafePtr<U>& b)
	{
		return a.Pointer() == b.Pointer();
	}

	template<class U>
	bool operator!=(const SafePtr<U>& a, const SafePtr<U>& b)
	{
		return a.Pointer() != b.Pointer();
	}

	template<class T>
	bool SafePtr<T>::operator !() const
	{
		return !Pointer();
	}

	template<class T>
	SafePtr<T>::operator T* () const
	{
		return static_cast<T*>(Pointer());
	}

	template<class T>
	T* SafePtr<T>::operator->() const
	{
		assert(Pointer());
		return static_cast<T*>(Pointer());
	}

	template<class T>
	T& SafePtr<T>::operator*() const
	{
		assert(Pointer());
		return static_cast<T*>(Pointer());
	}
}
