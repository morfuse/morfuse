#include <morfuse/Script/Class.h>
#include <morfuse/Script/Event.h>
#include <morfuse/Script/EventSystem.h>
#include <morfuse/Script/Context.h>
#include <morfuse/Script/ScriptMaster.h>
#include <morfuse/Common/MEM/PreAllocator.h>
#include <cstdarg>

using namespace mfuse;

LinkedList<ClassDef*, &ClassDef::next, &ClassDef::prev> ClassDef::classlist;
ClassDefExt* ClassDefExt::list = nullptr;

size_t ClassDef::numclasses;

/*
size_t ClassDef::dump_numclasses;
size_t ClassDef::dump_numevents;
Container<intptr_t> ClassDef::sortedList;
Container<ClassDef *> ClassDef::sortedClassList;
*/

MFUS_CLASS_DECLARATION(nullptr, Class, nullptr)
{
	{ nullptr, nullptr }
};

Class::Class()
{
}

Class::~Class()
{
}

const rawchar_t* Class::GetClassID(void) const
{
	return classinfo()->GetClassID();
}

const rawchar_t* Class::GetClassname(void) const
{
	return classinfo()->GetClassName();
}

const rawchar_t* Class::getSuperclass(void) const
{
	const ClassDef* super = classinfo()->GetSuper();
	return super ? super->GetClassName() : nullptr;
}

bool Class::inheritsFrom(ClassDef* c) const
{
	return ClassDef::checkInheritance(c, classinfo());
}

bool Class::inheritsFrom(const rawchar_t* name) const
{
	const ClassDef* c = ClassDef::GetClass(name);

	if (c == nullptr)
	{
		// FIXME
		//CLASS_DPrintf("Unknown class: %s\n", name);
		return false;
	}

	return ClassDef::checkInheritance(c, classinfo());
}

bool Class::isInheritedBy(const rawchar_t* name) const
{
	const ClassDef* c = ClassDef::GetClass(name);

	if (c == nullptr)
	{
		// FIXME
		//CLASS_DPrintf("Unknown class: %s\n", name);
		return false;
	}

	return ClassDef::checkInheritance(classinfo(), c);
}

bool Class::isInheritedBy(ClassDef* c) const
{
	return ClassDef::checkInheritance(classinfo(), c);
}

void Class::Archive(Archiver& arc)
{

}

void Class::warning(const rawchar_t* function, const rawchar_t* format, ...)
{
	const rawchar_t* classname;
	va_list va;

	va_start(va, format);
	int len = vsprintf(nullptr, format, va);
	rawchar_t* buffer = new rawchar_t[len + 1];
	vsprintf(buffer, format, va);
	va_end(va);

	classname = classinfo()->GetClassName();

	printf("%s::%s : %s\n", classname, function, buffer);

	delete[] buffer;
}

void Class::error(const rawchar_t* function, const rawchar_t* format, ...)
{
	va_list	va;

	va_start(va, format);
	int len = vsprintf(nullptr, format, va);
	rawchar_t* buffer = new rawchar_t[len + 1];
	vsprintf(buffer, format, va);
	va_end(va);

	if (GetClassID())
	{
		// FIXME
		//CLASS_Error("%s::%s : %s\n", GetClassID(), function, buffer);
	}
	else
	{
		// FIXME
		//CLASS_Error("%s::%s : %s\n", GetClassname(), function, buffer);
	}

	delete[] buffer;
}

ClassDef::ClassDef(ClassDef* superclass, const rawchar_t* classname, const rawchar_t* classID, const ResponseDefClass* responses,
	void* (*newInstance)())
{
	this->classname = classname;
	this->classID = classID;
	this->responses = responses;
	this->responseLookup = nullptr;
	this->newInstance = newInstance;
	this->super = superclass;
	this->waitTillSet = nullptr;

	if (!classID) classID = "";

	// Fixup class definitions that was created before their parent
	/*
	for (node = classlist; node != nullptr; node = node->next)
	{
		if ((node->super == nullptr) && (!xstr::icmp(node->superclass, this->classname)) &&
			(xstr::icmp(node->classname, "Class")))
		{
			node->super = this;
		}
	}
	*/

	// Add to front of list
	//LL::SafeAddFront<ClassDef*, &ClassDef::next, &ClassDef::prev>(classlist, this);
	classlist.Add(this);


	/*
	if (classlist) classlist->prev = this;
	prev = nullptr;
	next = classlist;
	classlist = this;
	*/

	numclasses++;
}

ClassDef::~ClassDef()
{
	// If the head of the list is deleted before the list is cleared, then we may have problems
	assert(!classlist.IsEmpty());

	//LL::SafeRemoveRoot<ClassDef*, &ClassDef::next, &ClassDef::prev>(classlist, this);
	classlist.Remove(this);

	// Check if any subclasses were initialized before their superclass
	for (auto node = classlist.CreateIterator(); node; node = node.Next())
	{
		if (node->super == this)
		{
			node->super = nullptr;
		}
	}

	//ClearResponseList();
}

void ClassDef::AddWaitTill(const xstr& s)
{
	//return AddWaitTill(Director.AddString(s));
}

void ClassDef::AddWaitTill(const_str s)
{
	if (!waitTillSet)
	{
		waitTillSet = new con::arrayset<const_str, const_str>;
	}

	waitTillSet->addKeyIndex(s);
}

void ClassDef::RemoveWaitTill(const xstr& s)
{
	//return RemoveWaitTill(Director.AddString(s));
}

void ClassDef::RemoveWaitTill(const_str s)
{
	if (waitTillSet)
	{
		waitTillSet->remove(s);
	}
}

bool ClassDef::WaitTillDefined(const xstr& s) const
{
	return false; // return WaitTillDefined(Director.AddString(s));
}

bool ClassDef::WaitTillDefined(const_str s) const
{
	if (!waitTillSet)
	{
		return false;
	}

	return waitTillSet->findKeyIndex(s) != 0;
}

const EventDef* ClassDef::GetDef(eventNum_t eventnum) const
{
	const ResponseDefClass* r = responseLookup[eventnum];
	return r ? r->event : nullptr;
}

const ResponseDefClass* ClassDef::GetResponse(eventNum_t eventNum) const
{
	return responseLookup[eventNum];
}

uint32_t ClassDef::GetFlags(Event* event) const
{
	const EventDef* def = GetDef(event->Num());
	return def ? def->flags : 0;
}

void ClassDef::BuildResponseList(MEM::PreAllocator& allocator)
{
	if (responseLookup)
	{
		// no need to rebuild the response again
		return;
	}

	const size_t num = EventSystem::NumEventCommands();
	responseLookup = new (allocator) const ResponseDefClass * [num];
	// event number start with 1
	--responseLookup;

	if (super)
	{
		// when a parent is present
		// copy the parent's responseLookup first
		super->BuildResponseList(allocator);
		std::copy(super->responseLookup, super->responseLookup + num, responseLookup);
	}
	else
	{
		// no parent so clear the response lookup
		std::fill(responseLookup, responseLookup + num, nullptr);
	}

	for (const ResponseDefClass* r = responses; r->event != nullptr; r++)
	{
		const eventNum_t ev = r->event->GetEventNum();
		assert(ev > 0);

		if (r->response)
		{
			// the event has a corresponding response method
			responseLookup[ev] = r;
		}
		else
		{
			// no response method
			responseLookup[ev] = nullptr;
		}
	}

	// Construct the response event lookup
	/*
	for (const ClassDef* c = this; c != nullptr; c = c->GetSuper())
	{
		const ResponseDefClass* r = c->responses;

		if (r)
		{
			for (size_t i = 0; r[i].event != nullptr; i++)
			{
				const uintptr_t ev = r[i].event->GetEventNum();

				if (!set[ev])
				{
					set[ev] = true;

					if (r[i].response)
					{
						responseLookup[ev] = &r[i];
					}
					else
					{
						responseLookup[ev] = nullptr;
					}
				}
			}
		}
	}
	*/
}

void ClassDef::ClearResponseList(MEM::PreAllocator& allocator)
{
	if (responseLookup)
	{
		//delete[] responseLookup;
		allocator.Free(responseLookup + 1);
		responseLookup = nullptr;
	}
}

size_t ClassDef::GetNumEvents() const
{
	size_t num = 0;
	for (size_t i = 0; responses[i].event != nullptr; i++, ++num);
	return num;
}

const ResponseDefClass* ClassDef::GetResponseList() const
{
	return responses;
}

ResponseLookup ClassDef::GetResponseLookupList() const
{
	return responseLookup;
}

const ClassDef* ClassDef::GetSuper() const
{
	return super;
}

const rawchar_t* ClassDef::GetClassName() const
{
	return classname;
}

const rawchar_t* ClassDef::GetClassID() const
{
	return classID;
}

ClassDef::List::iterator ClassDef::GetList()
{
	return classlist.CreateIterator();
}

size_t ClassDef::GetNumClasses()
{
	return numclasses;
}

ClassDefExt::ClassDefExt(ClassDef* existingClass, const ResponseDefClass* inResponses)
	: classDef(existingClass)
	, responses(inResponses)
{
	LL::SafeAddFront<ClassDefExt*, &ClassDefExt::next, &ClassDefExt::prev>(list, this);
}

ClassDefExt::~ClassDefExt()
{
	LL::SafeRemoveRoot<ClassDefExt*, &ClassDefExt::next, &ClassDefExt::prev>(list, this);
}

void ClassDefExt::InitClassDef()
{
	const size_t numEvents = EventSystem::NumEventCommands();

	for(const ClassDefExt* ext = list; list; list = list->next)
	{
		ResponseLookup lookup = ext->classDef->GetResponseLookupList();
		if (lookup)
		{
			for(const ResponseDefClass* r = ext->responses; r->event; ++r)
			{
				const uintptr_t ev = r->event->GetEventNum();

				if(r->response)
				{
					lookup[ev] = r;
				}
			}
		}
	}
}

static void CLASS_Printf(const rawchar_t *format, ...)
{
	va_list va;
	va_start(va, format);
	vprintf(format, va);
	va_end(va);
}

static void CLASS_DPrintf(const rawchar_t *format, ...)
{
#ifdef _DEBUG
	va_list va;
	va_start(va, format);
	vprintf(format, va);
	va_end(va);
#endif
}

static void CLASS_Error(const rawchar_t *format, ...)
{
	va_list va;
	va_start(va, format);
	vprintf(format, va);
	va_end(va);
}

ClassDef *ClassDef::GetClassForID(const rawchar_t *name)
{
	for (auto c = ClassDef::GetList(); c; c = c.Next())
	{
		if (c->GetClassID() && !xstr::icmp(c->GetClassID(), name))
		{
			return c;
		}
	}

	return nullptr;
}

const ClassDef *ClassDef::GetClass(const rawchar_t *name)
{
	if (name == nullptr || !*name) {
		return nullptr;
	}

	for (auto c = ClassDef::GetList(); c; c = c.Next())
	{
		if (xstr::icmp(c->GetClassName(), name) == 0) {
			return c.Node();
		}
	}

	return nullptr;
}

bool ClassDef::checkInheritance(const ClassDef *superclass, const ClassDef *subclass)
{
	const ClassDef *c;

	for (c = subclass; c != nullptr; c = c->GetSuper())
	{
		if (c == superclass)
		{
			return true;
		}
	}
	return false;
}

bool ClassDef::checkInheritance(ClassDef *superclass, const rawchar_t *subclass)
{
	const ClassDef* c = ClassDef::GetClass(subclass);

	if (c == nullptr)
	{
		CLASS_DPrintf("Unknown class: %s\n", subclass);
		return false;
	}

	return checkInheritance(superclass, c);
}

bool ClassDef::checkInheritance(const rawchar_t *superclass, const rawchar_t *subclass)
{
	const ClassDef* c1 = GetClass(superclass);
	const ClassDef* c2 = GetClass(subclass);

	if (c1 == nullptr)
	{
		CLASS_DPrintf("Unknown class: %s\n", superclass);
		return false;
	}

	if (c2 == nullptr)
	{
		CLASS_DPrintf("Unknown class: %s\n", subclass);
		return false;
	}

	return checkInheritance(c1, c2);
}

void CLASS_Print(FILE *class_file, const rawchar_t *fmt, ...)
{
	va_list	argptr;
	rawchar_t		text[1024];

	va_start(argptr, fmt);
	vsprintf(text, fmt, argptr);
	va_end(argptr);

	if (class_file)
		fprintf(class_file, "%s", text);
	else
		CLASS_DPrintf(text);
}

/*
void listAllClasses(void)
{
	ClassDef* c;
	for (c = GetClassList(); c != nullptr; c = c->GetNext())
	{
		CLASS_DPrintf("%s\n", c->GetClassName());
	}
}

void listInheritanceOrder(const rawchar_t* classname)
{
	ClassDef* cls;
	ClassDef* c;

	cls = GetClass(classname);
	if (!cls)
	{
		CLASS_DPrintf("Unknown class: %s\n", classname);
		return;
	}
	for (c = cls; c != nullptr; c = c->GetSuper())
	{
		CLASS_DPrintf("%s\n", c->GetClassName());
	}
}
*/
