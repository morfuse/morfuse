#pragma once

#include "../Container/Container.h"
#include "../Common/str.h"
#include "../Container/arrayset.h"
#include "../Common/AbstractClass.h"
#include "../Common/ConstStr.h"

#include <vector>

namespace mfuse
{
	class Archiver;
	class Class;
	class Event;
	namespace MEM { class PreAllocator; }

	using eventNum_t = uintptr_t;

#define MFUS_CLASS_DECLARATION(parentclass, classname, classid)	\
	mfuse::ClassDef classname::ClassInfo						\
	(															\
		mfuse::ClassDef::GetClass<parentclass>(),				\
		#classname, classid,									\
		(mfuse::ResponseDefClass *)classname::Responses,		\
		(mfuse::NewInstanceHandler)&classname::_newInstance		\
	);															\
	classname *classname::_newInstance()						\
	{															\
		return new classname;									\
	}															\
	const mfuse::ClassDef *classname::classinfo() const			\
	{															\
		return &(classname::ClassInfo);							\
	}															\
	mfuse::ClassDef *classname::staticclass()					\
	{															\
		return &(classname::ClassInfo);							\
	}															\
	void classname::AddWaitTill(mfuse::const_str s)				\
	{															\
		classname::ClassInfo.AddWaitTill(s);					\
	}															\
	void classname::RemoveWaitTill(mfuse::const_str s)			\
	{															\
		classname::ClassInfo.RemoveWaitTill(s);					\
	}															\
	bool classname::WaitTillDefined(mfuse::const_str s)			\
	{															\
		return classname::ClassInfo.WaitTillDefined(s);			\
	}															\
	mfuse::ResponseDef<classname> classname::Responses[] =

#define MFUS_CLASS_ABSTRACT_PROTOTYPE(classname)			\
	private:												\
	static mfuse::ClassDef ClassInfo;						\
	static classname* _newInstance();						\
	public:													\
	static void AddWaitTill(mfuse::const_str s);			\
	static void RemoveWaitTill(mfuse::const_str s);			\
	static bool WaitTillDefined(mfuse::const_str s);		\
	static mfuse::ResponseDef<classname> Responses[];		\
	private:

#define MFUS_CLASS_PROTOTYPE(classname)						\
	MFUS_CLASS_ABSTRACT_PROTOTYPE(classname)				\
	public:													\
	static mfuse::ClassDef* staticclass();					\
	const mfuse::ClassDef* classinfo() const override;		\
	private:												\

#define MFUS_CLASS_PROTOTYPE_EXPORTS(classname)				\
	MFUS_CLASS_ABSTRACT_PROTOTYPE(classname)				\
	public:													\
	mfuse_EXPORTS static mfuse::ClassDef* staticclass();	\
	const mfuse::ClassDef* classinfo() const override;		\
	private:												\

	using Response = void (Class::*)(Event *ev);

	class EventDef;

	template<class Type>
	struct mfuse_EXPORTS ResponseDef
	{
		EventDef		*event;
		void			(Type::*response)(Event *ev);
	};

	using ResponseDefClass = ResponseDef<Class>;
	using ResponseLookup = const ResponseDefClass**;
	using NewInstanceHandler = Class* (*)();

	class ClassDef
	{
	public:
		/* Create-a-class function */
		mfuse_EXPORTS ClassDef(ClassDef* superclass, const rawchar_t* classname, const rawchar_t* classID, const ResponseDefClass* responses,
			NewInstanceHandler newInstance);

		mfuse_EXPORTS ~ClassDef();

		ClassDef(const ClassDef&) = delete;
		ClassDef& operator=(const ClassDef&) = delete;
		ClassDef(ClassDef&&) = delete;
		ClassDef& operator=(ClassDef&&) = delete;

		void BuildResponseList(MEM::PreAllocator& allocator);
		void ClearResponseList(MEM::PreAllocator& allocator);

		mfuse_EXPORTS Class* createInstance() const;
		mfuse_EXPORTS void AddWaitTill(const_str s);
		mfuse_EXPORTS void RemoveWaitTill(const_str s);
		mfuse_EXPORTS bool WaitTillDefined(const_str s) const;

		mfuse_EXPORTS const rawchar_t* GetClassName() const;
		mfuse_EXPORTS const rawchar_t* GetClassID() const;
		mfuse_EXPORTS const ResponseDefClass* GetResponseList() const;
		mfuse_EXPORTS const ResponseDefClass* GetResponse(eventNum_t eventNum) const;
		mfuse_EXPORTS ResponseLookup GetResponseLookupList() const;
		mfuse_EXPORTS size_t GetNumEvents() const;

		mfuse_EXPORTS const EventDef *GetDef(eventNum_t eventnum) const;
		mfuse_EXPORTS uint32_t GetFlags(Event *event) const;

		mfuse_EXPORTS const ClassDef* GetSuper() const;

		mfuse_EXPORTS static ClassDef* GetClassForID(const rawchar_t* name);
		mfuse_EXPORTS static const ClassDef* GetClass(const rawchar_t* name);

		template<typename T>
		static ClassDef* GetClass();

		template<std::nullptr_t>
		static std::nullptr_t GetClass();

		mfuse_EXPORTS bool inheritsFrom(const rawchar_t* name) const;
		mfuse_EXPORTS bool inheritsFrom(const ClassDef* other) const;
		mfuse_EXPORTS bool isInheritedBy(const rawchar_t* name) const;
		mfuse_EXPORTS bool isInheritedBy(const ClassDef* other) const;
		mfuse_EXPORTS static bool checkInheritance(const ClassDef* superclass, const ClassDef* subclass);
		mfuse_EXPORTS static bool checkInheritance(ClassDef* superclass, const rawchar_t* subclass);
		mfuse_EXPORTS static bool checkInheritance(const rawchar_t* superclass, const rawchar_t* subclass);
		mfuse_EXPORTS static size_t GetNumClasses();

		//static int compareClasses(const void* arg1, const void* arg2);
		//static void SortClassList(Container<ClassDef*>* sortedList);

	private:
		const rawchar_t* classname;
		const rawchar_t* classID;
		NewInstanceHandler newInstance;
		const ResponseDefClass* responses;
		ResponseLookup responseLookup;
		ClassDef* super;
		ClassDef* next;
		ClassDef* prev;

		con::arrayset<const_str, const_str>* waitTillSet;

	public:
		using List = LinkedList<ClassDef*, &ClassDef::next, &ClassDef::prev>;
		static List::iterator GetList();

	private:
		static List classlist;
		//static ClassDef* classlist;
		static size_t numclasses;

		/*
		static size_t dump_numclasses;
		static size_t dump_numevents;
		static Container<intptr_t> sortedList;
		static Container<ClassDef*> sortedClassList;
		*/
	};

	template<typename T> ClassDef* ClassDef::GetClass() { return T::staticclass(); }
	template<std::nullptr_t> std::nullptr_t ClassDef::GetClass() { return nullptr; }

	/**
	 * Extend an existing class definition.
	 */
	class ClassDefExt
	{
	public:
		mfuse_EXPORTS ClassDefExt(ClassDef* existingClass, const ResponseDefClass* responses);
		mfuse_EXPORTS ~ClassDefExt();

		static void InitClassDef();

	private:
		ClassDef* classDef;
		const ResponseDefClass* responses;
		ClassDefExt* next;
		ClassDefExt* prev;

		static ClassDefExt* list;
	};

	class Class : public AbstractClass
	{
		MFUS_CLASS_ABSTRACT_PROTOTYPE(Class);

	public:
		mfuse_EXPORTS static mfuse::ClassDef* staticclass();
		virtual const mfuse::ClassDef* classinfo() const;

	public:
		mfuse_EXPORTS Class();
		mfuse_EXPORTS ~Class();

		mfuse_EXPORTS virtual void Archive(Archiver& arc);

		mfuse_EXPORTS const rawchar_t* GetClassID() const;
		mfuse_EXPORTS const rawchar_t* GetClassname() const;
		mfuse_EXPORTS const rawchar_t* getSuperclass() const;

		mfuse_EXPORTS bool inheritsFrom(ClassDef* c) const;
		mfuse_EXPORTS bool inheritsFrom(const rawchar_t* name) const;
		mfuse_EXPORTS bool isInheritedBy(const rawchar_t* name) const;
		mfuse_EXPORTS bool isInheritedBy(ClassDef* c) const;
	};
};
