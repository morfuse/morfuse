#include <morfuse/Script/Class.h>
#include <morfuse/Script/Event.h>
#include <morfuse/Script/EventSystem.h>
#include <morfuse/Common/MEM/PreAllocator.h>

using namespace mfuse;

ClassDef::List ClassDef::classlist = ClassDef::List();
ClassDefExt* ClassDefExt::list = nullptr;

size_t ClassDef::numclasses;

ClassDef::ClassDef(ClassDef* superclass, const rawchar_t* classname, const rawchar_t* classID, const ResponseDefClass* responses,
	NewInstanceHandler newInstance)
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
		if ((node->super == nullptr) && (!str::icmp(node->superclass, this->classname)) &&
			(str::icmp(node->classname, "Class")))
		{
			node->super = this;
		}
	}
	*/

	// Add to front of list
	classlist.Add(this);

	numclasses++;
}

ClassDef::~ClassDef()
{
	// If the head of the list is deleted before the list is cleared, then we may have problems
	assert(!classlist.IsEmpty());

	classlist.Remove(this);

	// Check if any subclasses were initialized before their superclass
	for (auto node = classlist.CreateIterator(); node; node = node.Next())
	{
		if (node->super == this)
		{
			node->super = nullptr;
		}
	}
}

void ClassDef::AddWaitTill(const_str s)
{
	if (!waitTillSet)
	{
		waitTillSet = new con::arrayset<const_str, const_str>;
	}

	waitTillSet->addKeyIndex(s);
}

void ClassDef::RemoveWaitTill(const_str s)
{
	if (waitTillSet)
	{
		waitTillSet->remove(s);
	}
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
		std::copy(super->responseLookup + 1, super->responseLookup + num + 1, responseLookup + 1);
	}
	else
	{
		// no parent so clear the response lookup
		std::fill(responseLookup + 1, responseLookup + num + 1, nullptr);
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
	for (const ClassDefExt* ext = list; list; list = list->next)
	{
		ResponseLookup lookup = ext->classDef->GetResponseLookupList();
		if (lookup)
		{
			for (const ResponseDefClass* r = ext->responses; r->event; ++r)
			{
				const uintptr_t ev = r->event->GetEventNum();

				if (r->response)
				{
					lookup[ev] = r;
				}
			}
		}
	}
}

ClassDef* ClassDef::GetClassForID(const rawchar_t* name)
{
	for (auto c = ClassDef::GetList(); c; c = c.Next())
	{
		if (c->GetClassID() && !str::icmp(c->GetClassID(), name))
		{
			return c;
		}
	}

	return nullptr;
}

const ClassDef* ClassDef::GetClass(const rawchar_t* name)
{
	if (name == nullptr || !*name) {
		return nullptr;
	}

	for (auto c = ClassDef::GetList(); c; c = c.Next())
	{
		if (str::icmp(c->GetClassName(), name) == 0) {
			return c.Node();
		}
	}

	return nullptr;
}

bool ClassDef::checkInheritance(const ClassDef* superclass, const ClassDef* subclass)
{
	const ClassDef* c;

	for (c = subclass; c != nullptr; c = c->GetSuper())
	{
		if (c == superclass)
		{
			return true;
		}
	}
	return false;
}

bool ClassDef::checkInheritance(ClassDef* superclass, const rawchar_t* subclass)
{
	const ClassDef* c = ClassDef::GetClass(subclass);

	if (!c) {
		return false;
	}

	return checkInheritance(superclass, c);
}

bool ClassDef::checkInheritance(const rawchar_t* superclass, const rawchar_t* subclass)
{
	const ClassDef* c1 = GetClass(superclass);
	const ClassDef* c2 = GetClass(subclass);

	if (!c1 || !c2) {
		return false;
	}

	return checkInheritance(c1, c2);
}

Class* ClassDef::createInstance() const
{
	return newInstance();
}

bool ClassDef::inheritsFrom(const rawchar_t* name) const
{
	const ClassDef* const other = GetClass(name);
	return inheritsFrom(other);
}

bool ClassDef::inheritsFrom(const ClassDef* other) const
{
	for (const ClassDef* c = this; c; c = c->GetSuper())
	{
		if (c == other)
		{
			return true;
		}
	}
	return false;
}

bool ClassDef::isInheritedBy(const rawchar_t* name) const
{
	const ClassDef* const other = GetClass(name);
	return isInheritedBy(other);
}

bool ClassDef::isInheritedBy(const ClassDef* other) const
{
	for (const ClassDef* c = other; c; c = c->GetSuper())
	{
		if (c == this)
		{
			return true;
		}
	}
	return false;
}
