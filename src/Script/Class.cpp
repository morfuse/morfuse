#include <morfuse/Script/Class.h>

using namespace mfuse;

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
	return classinfo().GetClassID();
}

const rawchar_t* Class::GetClassname(void) const
{
	return classinfo().GetClassName();
}

const rawchar_t* Class::getSuperclass(void) const
{
	const ClassDef* super = classinfo().GetSuper();
	return super ? super->GetClassName() : nullptr;
}

bool Class::inheritsFrom(ClassDef* c) const
{
	return ClassDef::checkInheritance(c, &classinfo());
}

bool Class::inheritsFrom(const rawchar_t* name) const
{
	const ClassDef* c = ClassDef::GetClass(name);

	if (!c) {
		return false;
	}

	return ClassDef::checkInheritance(c, &classinfo());
}

bool Class::isInheritedBy(const rawchar_t* name) const
{
	const ClassDef* c = ClassDef::GetClass(name);

	if (!c) {
		return false;
	}

	return ClassDef::checkInheritance(&classinfo(), c);
}

bool Class::isInheritedBy(ClassDef* c) const
{
	return ClassDef::checkInheritance(&classinfo(), c);
}

void Class::Archive(Archiver&)
{

}
