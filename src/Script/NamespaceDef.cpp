#include <morfuse/Script/NamespaceDef.h>

using namespace mfuse;

NamespaceDef::List NamespaceDef::head = NamespaceDef::List();
mfuse::namespaceNum_t mfuse::NamespaceDef::lastNum = 0;

NamespaceDef::NamespaceDef(const rawchar_t* moduleNameValue, const rawchar_t* descriptionValue)
	: namespaceName(moduleNameValue)
	, description(descriptionValue)
{
	// look for already-existing namespaces
	for (auto it = head.CreateConstIterator(); it; it = it.Next())
	{
		if (!str::icmp(it->GetName(), namespaceName)) {
			throw NamespaceDefErrors::AlreadyExists(*it.Node());
		}
	}

	head.Add(this);
	num = ++lastNum;
}

NamespaceDef::~NamespaceDef()
{
	head.Remove(this);
}

const rawchar_t* NamespaceDef::GetName() const
{
	return namespaceName;
}

const rawchar_t* NamespaceDef::GetDescription() const
{
	return description;
}

namespaceNum_t NamespaceDef::GetId() const
{
	return num;
}

NamespaceDefErrors::AlreadyExists::AlreadyExists(const NamespaceDef& otherRef)
	: other(otherRef)
{
}

const mfuse::rawchar_t* NamespaceDefErrors::AlreadyExists::GetName() const
{
	return other.GetName();
}

const mfuse::rawchar_t* NamespaceDefErrors::AlreadyExists::GetDescription() const
{
	return other.GetDescription();
}


const char* NamespaceDefErrors::AlreadyExists::what() const noexcept
{
	if (!filled()) {
		fill("A namespace of name '" + str(GetName()) + "' already exists");
	}

	return Messageable::what();
}

ObjectInNamespace::ObjectInNamespace()
	: namespaceDef(nullptr)
{

}

ObjectInNamespace::ObjectInNamespace(const NamespaceDef& def)
	: namespaceDef(&def)
{

}

ObjectInNamespace::~ObjectInNamespace()
{
}

const NamespaceDef* ObjectInNamespace::GetNamespace() const
{
	return namespaceDef;
}
