#include <morfuse/Common/ConstStr.h>
#include <morfuse/Script/ScriptVariable.h>
#include <morfuse/Script/ScriptException.h>
#include <morfuse/Script/Listener.h>
#include <morfuse/Script/SimpleEntity.h>
#include <morfuse/Script/Archiver.h>
#include <morfuse/Script/Context.h>
#include <morfuse/Script/ScriptMaster.h>
#include <morfuse/Script/PredefinedString.h>
#include <morfuse/Container/set_archive.h>
#include <morfuse/Container/Container_archive.h>

using namespace mfuse;

template<>
class con::Entry<const_str, ScriptVariable> : public con::EntryBase<const_str, ScriptVariable>
{
public:
	Entry()
	{
	}

	Entry(const_str key)
		: value(key, nullptr)
	{
	}

	Entry(const_str key, const ScriptVariable& varValue)
		: value(key, varValue)
	{
	}

	const_str Key() const noexcept { return value.GetKey(); }
	ScriptVariable& Value() noexcept { return value; }
	const ScriptVariable& Value() const noexcept { return value; }

private:
	ScriptVariable value;
};

template<>
void con::Archive(Archiver& arc, con::Entry<const_str, ScriptVariable>& entry)
{
	entry.Value().Archive(arc);
}

template<>
void con::Archive(Archiver& arc, con::Entry<ScriptVariable, ScriptVariable>& entry)
{
	entry.Key().ArchiveInternal(arc);
	entry.Value().ArchiveInternal(arc);
}

template<>
intptr_t Hash<ScriptVariable>::operator()(const ScriptVariable& key) const
{
	Listener *l;

	switch (key.GetType())
	{
	case variableType_e::String:
	case variableType_e::ConstString:
		return Hash<str>()(key.stringValue());

	case variableType_e::Integer:
		return (intptr_t)key.GetData().long64Value;

	case variableType_e::Listener:
		l = key.listenerValue();
		return (intptr_t)l;

	default:
		throw ScriptVariableErrors::BadHashCodeValue(key.stringValue());
	}
}

static ScriptVariable noneVar;

ScriptArrayHolder::ScriptArrayHolder()
	: refCount(0)
{

}

ScriptArrayHolder::~ScriptArrayHolder()
{

}

void ScriptArrayHolder::Archive(Archiver& arc)
{
	arc.ArchiveUInt32(refCount);
	arrayValue.Archive(arc);
}

void ScriptArrayHolder::Archive(Archiver& arc, ScriptArrayHolder*& arrayHolder)
{
	bool newRef;

	if (arc.Loading())
	{
		arc.ArchiveBoolean(newRef);
	}
	else
	{
		newRef = !arc.ObjectPositionExists(arrayHolder);
		arc.ArchiveBoolean(newRef);
	}

	if (newRef)
	{
		if (arc.Loading())
		{
			arrayHolder = new ScriptArrayHolder();
		}

		arc.ArchiveObjectPosition(arrayHolder);
		arrayHolder->Archive(arc);
		return;
	}
	else
	{
		arc.ArchiveObjectPointer(arrayHolder);
	}
}

void ScriptConstArrayHolder::Archive(Archiver& arc)
{
	arc.ArchiveUInt32(refCount);
	if (arc.Loading())
	{
		uint32_t sz32;
		arc.ArchiveUInt32(sz32);
		size = sz32;
	}
	else
	{
		uint32_t sz32 = (uint32_t)size;
		arc.ArchiveUInt32(sz32);
	}

	if (arc.Loading())
	{
		constArrayValue = new ScriptVariable[size + 1] - 1;
	}

	for (uintptr_t i = 1; i <= size; i++)
	{
		constArrayValue[i].ArchiveInternal(arc);
	}
}

void ScriptConstArrayHolder::Archive(Archiver& arc, ScriptConstArrayHolder *& arrayHolder)
{
	bool newRef;

	if (arc.Loading())
	{
		arc.ArchiveBoolean(newRef);
	}
	else
	{
		newRef = !arc.ObjectPositionExists(arrayHolder);
		arc.ArchiveBoolean(newRef);
	}

	if (newRef)
	{
		if (arc.Loading())
		{
			arrayHolder = new ScriptConstArrayHolder();
		}

		arc.ArchiveObjectPosition(arrayHolder);
		arrayHolder->Archive(arc);
		return;
	}
	else
	{
		arc.ArchiveObjectPointer(arrayHolder);
	}
}

ScriptConstArrayHolder::ScriptConstArrayHolder(ScriptVariable *pVar, size_t size)
{
	refCount = 0;
	this->size = size;

	constArrayValue = new ScriptVariable[size] - 1;

	for (size_t i = 1; i <= size; i++)
	{
		constArrayValue[i] = pVar[i];
	}
}

ScriptConstArrayHolder::ScriptConstArrayHolder(size_t size)
{
	refCount = 0;
	this->size = size;

	constArrayValue = new ScriptVariable[size] - 1;
}

ScriptConstArrayHolder::ScriptConstArrayHolder()
{
	refCount = 0;
	size = 0;
	constArrayValue = nullptr;
}

ScriptConstArrayHolder::~ScriptConstArrayHolder()
{
	if (constArrayValue)
	{
		delete[] (constArrayValue + 1);
	}
}

ScriptPointer::ScriptPointer()
{

}

ScriptPointer::ScriptPointer(size_t initialSize)
	: list(initialSize)
{
}

ScriptPointer::~ScriptPointer()
{

}

void ScriptPointer::Archive(Archiver& arc)
{
	con::Archive(arc, list, &ScriptVariable::Archive);
}

void ScriptPointer::Archive(Archiver& arc, ScriptPointer*& pointerHolder)
{
	bool newRef;

	if (arc.Loading())
	{
		arc.ArchiveBoolean(newRef);
	}
	else
	{
		newRef = !arc.ObjectPositionExists(pointerHolder);
		arc.ArchiveBoolean(newRef);
	}

	if (newRef)
	{
		if (arc.Loading())
		{
			pointerHolder = new ScriptPointer();
		}

		arc.ArchiveObjectPosition(pointerHolder);
		pointerHolder->Archive(arc);
		return;
	}
	else
	{
		arc.ArchiveObjectPointer(pointerHolder);
	}
}

void ScriptPointer::Clear()
{
	for (size_t i = 0; i < list.NumObjects(); i++)
	{
		ScriptVariable *variable = list[i];

		variable->type = variableType_e::None;
	}

	delete this;
}

void ScriptPointer::add(ScriptVariable *var)
{
	list.AddObject(var);
}

void ScriptPointer::remove(ScriptVariable *var)
{
	list.RemoveObject(var);

	if (list.NumObjects() == 0)
	{
		delete this;
	}
}

void ScriptPointer::setValue(const ScriptVariable& var)
{
	if (var.type == variableType_e::Pointer)
	{
		for (uintptr_t i = list.NumObjects(); i > 0; i--) {
			ScriptVariable* pVar = list.ObjectAt(i);

			pVar->m_data.pointerValue = var.m_data.pointerValue;
			var.m_data.pointerValue->add(pVar);
		}
	}
	else
	{
		for (uintptr_t i = list.NumObjects(); i > 0; i--) {
			ScriptVariable* pVar = list.ObjectAt(i);

			// since they're holding this script pointer, set it to none
			// because otherwise it would call ClearInternal()
			// which will delete this ScriptPointer
			pVar->type = variableType_e::None;
			*pVar = var;
		}
	}

	delete this;
}

void ScriptPointer::setValueRef(ScriptVariable& var, const ScriptVariable& ignoredVar)
{
	if (list.NumObjects() == 2)
	{
		ScriptVariable* const pVars[] =
		{
			list[0],
			list[1]
		};

		pVars[0]->type = pVars[1]->type = variableType_e::None;

		if (pVars[0] == &ignoredVar) {
			*pVars[1] = std::move(var);
		}
		else if (pVars[1] == &ignoredVar) {
			*pVars[0] = std::move(var);
		}
		else {
			*pVars[0] = *pVars[1] = var;
		}
	}
	else
	{
		for (uintptr_t i = list.NumObjects(); i > 0; i--)
		{
			ScriptVariable* const pVar = list.ObjectAt(i);

			// since they're holding this script pointer, set it to none
			// because otherwise it would call ClearInternal()
			// which will delete this ScriptPointer
			pVar->type = variableType_e::None;
			*pVar = const_cast<const ScriptVariable&>(var);
		}
	}

	delete this;
}

scriptData_u::scriptData_u()
{
}

scriptData_u::scriptData_u(const scriptData_u& other)
{
	long64Value = other.long64Value;
}

scriptData_u& scriptData_u::operator=(const scriptData_u& other)
{
	long64Value = other.long64Value;
	return *this;
}

ScriptVariable::ScriptVariable()
	: key(0)
	, type(variableType_e::None)
{
}

ScriptVariable::ScriptVariable(const_str stringValue)
	: key(0)
	, type(variableType_e::ConstString)
{
	m_data.constStringValue = stringValue;
}

ScriptVariable::ScriptVariable(const_str keyValue, std::nullptr_t)
	: key(keyValue)
	, type(variableType_e::None)
{
}

ScriptVariable::ScriptVariable(const_str keyValue, const ScriptVariable& variable)
	: key(keyValue)
{
	setDataInternal(variable);
}

ScriptVariable::ScriptVariable(const ScriptVariable& variable)
	: key(0)
{
	type = variableType_e::None;
	*this = variable;
}

ScriptVariable::ScriptVariable(ScriptVariable&& variable)
	: m_data(variable.m_data)
	, key(0)
	, type(variable.type)
{
	variable.type = variableType_e::None;
}

ScriptVariable::ScriptVariable(int32_t initialValue)
	: key(0)
	, type(variableType_e::Integer)
{
	m_data.long64Value = initialValue;
}

ScriptVariable::ScriptVariable(int64_t initialValue)
	: key(0)
	, type(variableType_e::Integer)
{
	m_data.long64Value = initialValue;
}

ScriptVariable::ScriptVariable(float initialValue)
	: key(0)
	, type(variableType_e::Float)
{
	m_data.floatValue = initialValue;
}

ScriptVariable::ScriptVariable(char initialValue)
	: key(0)
	, type(variableType_e::Char)
{
	m_data.charValue = initialValue;
}

ScriptVariable::ScriptVariable(Listener* initialValue)
	: key(0)
	, type(variableType_e::Listener)
{
	m_data.listenerValue = new ListenerPtr(initialValue);
}

ScriptVariable::ScriptVariable(const rawchar_t* initialValue)
	: key(0)
	, type(variableType_e::String)
{
	m_data.stringValue = new str(initialValue);
}

ScriptVariable::ScriptVariable(const str& initialValue)
	: key(0)
	, type(variableType_e::String)
{
	m_data.stringValue = new str(initialValue);
}

ScriptVariable::ScriptVariable(const con::Container<ListenerPtr>* initialValue)
	: key(0)
	, type(variableType_e::Container)
{
	m_data.containerValue = initialValue;
}

ScriptVariable::ScriptVariable(const Vector& initialValue)
	: key(0)
	, type(variableType_e::Vector)
{
	m_data.vectorValue = new float[3];
	m_data.vectorValue[0] = initialValue[0];
	m_data.vectorValue[1] = initialValue[1];
	m_data.vectorValue[2] = initialValue[2];
}

ScriptVariable& ScriptVariable::operator=(const ScriptVariable& variable)
{
	switch(uint32_t(type + variable.type * variableType_e::Max))
	{
	case uint32_t(variableType_e::None + variableType_e::None * variableType_e::Max):
	case uint32_t(variableType_e::None + variableType_e::Integer * variableType_e::Max):
	case uint32_t(variableType_e::None + variableType_e::Float * variableType_e::Max):
	case uint32_t(variableType_e::None + variableType_e::Char * variableType_e::Max):
	case uint32_t(variableType_e::None + variableType_e::ConstString * variableType_e::Max):
	case uint32_t(variableType_e::String + variableType_e::None * variableType_e::Max):
	case uint32_t(variableType_e::String + variableType_e::Integer * variableType_e::Max):
	case uint32_t(variableType_e::String + variableType_e::Float * variableType_e::Max):
	case uint32_t(variableType_e::String + variableType_e::Char * variableType_e::Max):
	case uint32_t(variableType_e::String + variableType_e::ConstString * variableType_e::Max):
	case uint32_t(variableType_e::Integer + variableType_e::None * variableType_e::Max):
	case uint32_t(variableType_e::Integer + variableType_e::Integer * variableType_e::Max):
	case uint32_t(variableType_e::Integer + variableType_e::Float * variableType_e::Max):
	case uint32_t(variableType_e::Integer + variableType_e::Char * variableType_e::Max):
	case uint32_t(variableType_e::Integer + variableType_e::ConstString * variableType_e::Max):
	case uint32_t(variableType_e::Float + variableType_e::None * variableType_e::Max):
	case uint32_t(variableType_e::Float + variableType_e::Integer * variableType_e::Max):
	case uint32_t(variableType_e::Float + variableType_e::Float * variableType_e::Max):
	case uint32_t(variableType_e::Float + variableType_e::Char * variableType_e::Max):
	case uint32_t(variableType_e::Float + variableType_e::ConstString * variableType_e::Max):
	case uint32_t(variableType_e::Char + variableType_e::None * variableType_e::Max):
	case uint32_t(variableType_e::Char + variableType_e::Integer * variableType_e::Max):
	case uint32_t(variableType_e::Char + variableType_e::Float * variableType_e::Max):
	case uint32_t(variableType_e::Char + variableType_e::Char * variableType_e::Max):
	case uint32_t(variableType_e::Char + variableType_e::ConstString * variableType_e::Max):
	case uint32_t(variableType_e::ConstString + variableType_e::None * variableType_e::Max):
	case uint32_t(variableType_e::ConstString + variableType_e::Integer * variableType_e::Max):
	case uint32_t(variableType_e::ConstString + variableType_e::Float * variableType_e::Max):
	case uint32_t(variableType_e::ConstString + variableType_e::Char * variableType_e::Max):
	case uint32_t(variableType_e::ConstString + variableType_e::ConstString * variableType_e::Max):
		m_data = variable.m_data;
		type = variable.type;
		return *this;

	default:
		ClearInternal();
		break;
	}

	setDataInternal(variable);
	return *this;
}

ScriptVariable& ScriptVariable::operator=(ScriptVariable&& variable)
{
	ClearInternal();

	type = variable.type;
	variable.type = variableType_e::None;
	m_data = variable.m_data;

	if (type == variableType_e::Pointer)
	{
		// if it's a pointer, make sure to properly point
		m_data.pointerValue->add(this);
		m_data.pointerValue->remove(&variable);
	}

	return *this;
}

ScriptVariable::~ScriptVariable()
{
	ClearInternal();
}

void ScriptVariable::Archive(Archiver& arc)
{
	const_str s;

	if (arc.Loading())
	{
		ScriptContext::Get().GetDirector().GetDictionary().ArchiveString(arc, s);
		key = s;
	}
	else
	{
		s = key;
		ScriptContext::Get().GetDirector().GetDictionary().ArchiveString(arc, s);
	}

	ArchiveInternal(arc);
}

void ScriptVariable::Archive(Archiver& arc, ScriptVariable*& obj)
{
	arc.ArchiveObjectPointer((void*&)obj);
}

void ScriptVariable::ArchiveInternal(Archiver& arc)
{
	arc.ArchiveObjectPosition(this);

	arc.ArchiveEnum(type);
	switch (type)
	{
	case variableType_e::String:
		if (arc.Loading())
		{
			m_data.stringValue = new str(4);
		}

		::Archive(arc, *m_data.stringValue);
		break;

	case variableType_e::Integer:
		arc.ArchiveInt64(m_data.long64Value);
		break;

	case variableType_e::Float:
		arc.ArchiveFloat(m_data.floatValue);
		break;

	case variableType_e::Char:
		arc.ArchiveChar(m_data.charValue);
		break;

	case variableType_e::ConstString:
		ScriptContext::Get().GetDirector().GetDictionary().ArchiveString(arc, m_data.constStringValue);
		break;

	case variableType_e::Listener:
		if (arc.Loading())
		{
			m_data.listenerValue = new ListenerPtr;
		}

		arc.ArchiveSafePointer(*m_data.listenerValue);
		break;

	case variableType_e::Array:
		ScriptArrayHolder::Archive(arc, m_data.arrayValue);
		break;

	case variableType_e::ConstArray:
		ScriptConstArrayHolder::Archive(arc, m_data.constArrayValue);
		break;

	case variableType_e::Ref:
	case variableType_e::Container:
		arc.ArchiveObjectPointer((void*&)m_data.refValue);
		break;

	case variableType_e::SafeContainer:
		if (arc.Loading())
		{
			m_data.safeContainerValue = new ConListPtr;
		}

		arc.ArchiveSafePointer(*m_data.safeContainerValue);
		break;

	case variableType_e::Pointer:
		ScriptPointer::Archive(arc, m_data.pointerValue);
		break;

	case variableType_e::Vector:
		if (arc.Loading())
		{
			m_data.vectorValue = new float[3];
		}

		arc.ArchiveElements(m_data.vectorValue, 3);
		break;

	default:
		break;
	}
}

void ScriptVariable::CastBoolean()
{
	int newvalue = booleanValue();

	ClearInternal();

	type = variableType_e::Integer;
	m_data.long64Value = newvalue;
}

void ScriptVariable::CastConstArrayValue()
{
	ScriptConstArrayHolder* constArrayValue;

	switch (type)
	{
	case variableType_e::Pointer:
		ClearPointerInternal();
	case variableType_e::None:
		throw ScriptVariableErrors::CastError("NIL", "listener");

	case variableType_e::ConstArray:
		return;

	case variableType_e::Array:
	{
		constArrayValue = new ScriptConstArrayHolder(m_data.arrayValue->arrayValue.size());

		con::map_enum<ScriptVariable, ScriptVariable> en = m_data.arrayValue->arrayValue;

		uintptr_t i = 0;

		for (const ScriptVariable* value = en.NextValue(); value != nullptr; value = en.NextValue())
		{
			i++;
			constArrayValue->constArrayValue[i] = *value;
		}

		break;
	}

	case variableType_e::Container:
	{
		constArrayValue = new ScriptConstArrayHolder(m_data.containerValue->NumObjects());

		for (uintptr_t i = m_data.containerValue->NumObjects(); i > 0; i--)
		{
			constArrayValue->constArrayValue[i].setListenerValue(m_data.containerValue->ObjectAt(i));
		}
		break;
	}

	case variableType_e::SafeContainer:
	{
		const ConList* listeners = *m_data.safeContainerValue;

		if (listeners)
		{
			constArrayValue = new ScriptConstArrayHolder(listeners->NumObjects());

			for (intptr_t i = listeners->NumObjects(); i > 0; i--)
			{
				constArrayValue->constArrayValue[i].setListenerValue(listeners->ObjectAt(i));
			}
		}
		else
		{
			constArrayValue = new ScriptConstArrayHolder(0);
		}
		break;
	}

	default:
	{
		constArrayValue = new ScriptConstArrayHolder(1);
		constArrayValue->constArrayValue[1] = *this;

		break;
	}
	}

	ClearInternal();
	type = variableType_e::ConstArray;
	m_data.constArrayValue = constArrayValue;
}

void ScriptVariable::CastFloat()
{
	setFloatValue(floatValue());
}

void ScriptVariable::CastInteger()
{
	setIntValue(intValue());
}

void ScriptVariable::CastString()
{
	setStringValue(stringValue());
}

void ScriptVariable::Clear()
{
	ClearInternal();
	type = variableType_e::None;
}

void ScriptVariable::ClearInternal()
{
	switch (type)
	{
	case variableType_e::String:
		if (m_data.stringValue)
		{
			delete m_data.stringValue;
			m_data.stringValue = nullptr;
		}

		break;

	case variableType_e::Array:
		if (m_data.arrayValue->refCount)
		{
			m_data.arrayValue->refCount--;
		}
		else
		{
			delete m_data.arrayValue;
		}

		m_data.arrayValue = nullptr;
		break;

	case variableType_e::ConstArray:
		if (m_data.constArrayValue->refCount)
		{
			m_data.constArrayValue->refCount--;
		}
		else
		{
			delete m_data.constArrayValue;
		}

		m_data.constArrayValue = nullptr;
		break;

	case variableType_e::Listener:
		if (m_data.listenerValue)
		{
			delete m_data.listenerValue;
			m_data.listenerValue = nullptr;
		}

		break;

	case variableType_e::SafeContainer:
		if (m_data.safeContainerValue)
		{
			delete m_data.safeContainerValue;
			m_data.safeContainerValue = nullptr;
		}

		break;

	case variableType_e::Pointer:
		m_data.pointerValue->remove(this);
		m_data.pointerValue = nullptr;
		break;

	case variableType_e::Vector:
		delete[] m_data.vectorValue;
		m_data.vectorValue = nullptr;
		break;

	default:
		break;
	}
}

void ScriptVariable::ClearPointer()
{
	if (type == variableType_e::Pointer) {
		return ClearPointerInternal();
	}
}

void ScriptVariable::ClearPointerInternal() const
{
	m_data.pointerValue->Clear();
}

const rawchar_t *ScriptVariable::GetTypeName() const
{
	return typenames[(uintptr_t)type];
}

variableType_e ScriptVariable::GetType() const
{
	return (variableType_e)type;
}

scriptData_u ScriptVariable::GetData() const
{
	return m_data;
}

scriptData_u& ScriptVariable::GetData()
{
	return m_data;
}

bool ScriptVariable::IsEntity() const
{
	if (type == variableType_e::Listener)
	{
		if (!m_data.listenerValue->Pointer() ||
#if defined ( CGAME_DLL ) || defined ( GAME_DLL )
			checkInheritance(Entity::classinfostatic(), m_data.listenerValue->Pointer()->classinfo())
#else
			0
#endif
			)
		{
			return true;
		}
	}

	return false;
}

bool ScriptVariable::IsListener() const
{
	return type == variableType_e::Listener;
}

bool ScriptVariable::IsConstArray() const
{
	return type == variableType_e::ConstArray
		|| type == variableType_e::Container
		|| type == variableType_e::SafeContainer;
}

bool ScriptVariable::IsNumeric() const
{
	return type == variableType_e::Integer || type == variableType_e::Float;
}

bool ScriptVariable::IsSimpleEntity() const
{
	if (type == variableType_e::Listener)
	{
		/*
		if (!m_data.listenerValue->Pointer() ||
			checkInheritance(&SimpleEntity::ClassInfo, m_data.listenerValue->Pointer()->classinfo())
			)
		{
			return true;
		}
		*/
	}

	return false;
}

bool ScriptVariable::IsString() const
{
	return (type == variableType_e::String || type == variableType_e::ConstString);
}

bool ScriptVariable::IsVector() const
{
	return type == variableType_e::Vector;
}

bool ScriptVariable::IsImmediate() const
{
	return type <= variableType_e::ConstString;
}

bool ScriptVariable::IsNone() const
{
	return type == variableType_e::None;
}

void ScriptVariable::PrintValue()
{
	const ScriptContext& context = ScriptContext::Get();
	std::ostream* out = context.GetOutputInfo().GetOutput(outputLevel_e::Debug);

	switch (type)
	{
	case variableType_e::None:
		*out << std::endl;
		break;

	case variableType_e::ConstString:
		*out << context.GetDirector().GetDictionary().Get(m_data.constStringValue).c_str();
		break;

	case variableType_e::String:
		*out << m_data.stringValue->c_str();
		break;

	case variableType_e::Integer:
		*out << m_data.long64Value;
		break;

	case variableType_e::Float:
		*out << m_data.floatValue;
		break;

	case variableType_e::Char:
		*out << m_data.charValue;
		break;

	case variableType_e::Listener:
		*out << "<Listener>" << m_data.listenerValue->Pointer();
		break;

	case variableType_e::Ref:
	case variableType_e::Array:
	case variableType_e::ConstArray:
	case variableType_e::Container:
	case variableType_e::SafeContainer:
	case variableType_e::Pointer:
		*out << "type: " << GetTypeName();
		break;

	case variableType_e::Vector:
		*out << "( " << m_data.vectorValue[0] << " " << m_data.vectorValue[1] << " " << m_data.vectorValue[2] << " )";
		break;
	default:
		break;
	}
}

void ScriptVariable::SetFalse()
{
	setIntValue(0);
}

void ScriptVariable::SetTrue()
{
	setIntValue(1);
}

size_t ScriptVariable::arraysize() const
{
	switch (type)
	{
	case variableType_e::None:
		return -1;

	case variableType_e::String:
	case variableType_e::Integer:
	case variableType_e::Float:
	case variableType_e::Char:
	case variableType_e::ConstString:
	case variableType_e::Listener:
	case variableType_e::Ref:
	case variableType_e::Vector:
		return 1;

	case variableType_e::Array:
		return m_data.arrayValue->arrayValue.size();

	case variableType_e::ConstArray:
		return m_data.constArrayValue->size;

	case variableType_e::Container:
		return m_data.containerValue->NumObjects();

	case variableType_e::SafeContainer:
		if (*m_data.safeContainerValue) {
			return (*m_data.safeContainerValue)->NumObjects();
		}
		else {
			return 0;
		}

	case variableType_e::Pointer:
		ClearPointerInternal();
		return -1;

	default:
		return -1;
	}

	return 0;
}

size_t ScriptVariable::size() const
{
	switch (type)
	{
	case variableType_e::None:
		return -1;

	case variableType_e::ConstString:
	case variableType_e::String:
		return stringValue().length();

	case variableType_e::Listener:
		return *m_data.listenerValue != nullptr;

	case variableType_e::Array:
		return m_data.arrayValue->arrayValue.size();

	case variableType_e::ConstArray:
		return m_data.constArrayValue->size;

	case variableType_e::Container:
		return m_data.containerValue->NumObjects();

	case variableType_e::SafeContainer:
		if (*m_data.safeContainerValue) {
			return (*m_data.safeContainerValue)->NumObjects();
		}
		else {
			return 0;
		}

	case variableType_e::Pointer:
		ClearPointerInternal();
		return -1;

	default:
		return 1;
	}

	return 0;
}

bool ScriptVariable::booleanNumericValue()
{
	str value;

	switch (type)
	{
	case variableType_e::String:
	case variableType_e::ConstString:
		value = stringValue();

		return atoi(value.c_str()) ? true : false;

	case variableType_e::Integer:
		return m_data.long64Value != 0;

	case variableType_e::Float:
		return fabs(m_data.floatValue) >= 0.00009999999747378752;

	case variableType_e::Listener:
		return (*m_data.listenerValue) != nullptr;

	default:
		throw ScriptVariableErrors::CastError(GetTypeName(), "boolean numeric");
	}

	return true;
}

bool ScriptVariable::booleanValue() const
{
	switch (type)
	{
	case variableType_e::None:
		return false;

	case variableType_e::String:
		if (m_data.stringValue)
		{
			return m_data.stringValue->length() != 0;
		}

		return false;

	case variableType_e::Integer:
		return m_data.long64Value != 0;

	case variableType_e::Float:
		return fabs(m_data.floatValue) >= 0.00009999999747378752;

	case variableType_e::ConstString:
		return m_data.constStringValue != ConstStrings::Empty.GetIndex();

	case variableType_e::Listener:
		return (*m_data.listenerValue) != nullptr;

	default:
		return true;
	}
}

rawchar_t ScriptVariable::charValue() const
{
	str value;

	switch (type)
	{
	case variableType_e::Char:
		return m_data.charValue;

	case variableType_e::ConstString:
	case variableType_e::String:
		value = stringValue();

		if (value.length() != 1)
		{
			throw ScriptVariableErrors::CastError("string not of length 1", "listener");
		}

		return value[0];

	default:
		throw ScriptVariableErrors::CastError(GetTypeName(), "char");
	}

	return 0;
}

ScriptVariable *ScriptVariable::constArrayValue()
{
	return m_data.constArrayValue->constArrayValue;
}

const_str ScriptVariable::GetKey() const
{
	return key;
}

void ScriptVariable::SetKey(const_str key)
{
	this->key = key;
}

void ScriptVariable::evalArrayAt(const ScriptVariable &var)
{
	size_t index;
	str string;
	ScriptVariable *array;

	switch (type)
	{
	case variableType_e::Vector:
		index = (size_t)var.longValue();

		if (index > 2)
		{
			Clear();
			throw ScriptVariableErrors::TypeIndexOutOfRange("Vector", index);
		}

		return setFloatValue(m_data.vectorValue[index]);

	case variableType_e::None:
		break;

	case variableType_e::ConstString:
	case variableType_e::String:
		index = (size_t)var.longValue();
		string = stringValue();

		if (index >= string.length())
		{
			Clear();
			throw ScriptVariableErrors::TypeIndexOutOfRange("String", index);
		}

		return setCharValue(string[index]);

	case variableType_e::Listener:
		index = (size_t)var.longValue();

		if (index != 1)
		{
			Clear();
			throw ScriptVariableErrors::TypeIndexOutOfRange("array", index);
		}

		break;

	case variableType_e::Array:
		array = m_data.arrayValue->arrayValue.find(var);

		if (array)
		{
			*this = *array;
		}
		else
		{
			Clear();
		}

		break;

	case variableType_e::ConstArray:
		index = (size_t)var.longValue();

		if (!index || index > m_data.constArrayValue->size)
		{
			throw ScriptVariableErrors::TypeIndexOutOfRange("array", index);
		}

		*this = m_data.constArrayValue->constArrayValue[index];
		break;

	case variableType_e::Container:
		index = (size_t)var.longValue();

		if (!index || index > m_data.containerValue->NumObjects())
		{
			throw ScriptVariableErrors::TypeIndexOutOfRange("array", index);
		}

		setListenerValue(m_data.containerValue->ObjectAt(index));
		break;

	case variableType_e::SafeContainer:
		index = (size_t)var.longValue();

		if (!*m_data.safeContainerValue || !index || index > m_data.constArrayValue->size) {
			throw ScriptVariableErrors::TypeIndexOutOfRange("array", index);
		}

		setListenerValue((*m_data.safeContainerValue)->ObjectAt(index));
		break;

	default:
		Clear();
		throw ScriptVariableErrors::InvalidAppliedType("[]", GetTypeName());
	}
}

float ScriptVariable::floatValue() const
{
	const rawchar_t *string;
	float val;
	rawchar_t* p;

	switch (type)
	{
	case variableType_e::Float:
		return m_data.floatValue;

	case variableType_e::Integer:
		return (float)m_data.long64Value;

		/* Transform the string into an integer if possible */
	case variableType_e::String:
	case variableType_e::ConstString:
		string = stringValue().c_str();
		val = std::strtof(string, &p);

		return val;

	default:
		throw ScriptVariableErrors::CastError(GetTypeName(), "float");
	}
}

uint32_t ScriptVariable::intValue() const
{
	switch (type)
	{
	case variableType_e::Integer:
		return m_data.int32Value;

	case variableType_e::Float:
		return (uint32_t)m_data.floatValue;

	case variableType_e::String:
	case variableType_e::ConstString:
	{
		str string = stringValue();
		rawchar_t* p;
		const uint32_t val = std::strtol(string.c_str(), &p, 10);

		return val;
	}

	default:
		throw ScriptVariableErrors::CastError(GetTypeName(), "int");
	}
}

uint64_t ScriptVariable::longValue() const
{
	switch (type)
	{
	case variableType_e::Integer:
		return m_data.long64Value;

	case variableType_e::Float:
		return (uint64_t)m_data.floatValue;

	case variableType_e::String:
	case variableType_e::ConstString:
	{
		str string = stringValue();
		rawchar_t* p;
		const uint64_t val = std::strtoll(string.c_str(), &p, 10);

		return val;
	}

	default:
		throw ScriptVariableErrors::CastError(GetTypeName(), "int64");
	}
}

Listener *ScriptVariable::listenerValue() const
{
	Listener* l = nullptr;

	switch (type)
	{
	case variableType_e::ConstString:
		l = ScriptContext::Get().GetTargetList().GetTarget(m_data.constStringValue);
		break;

	case variableType_e::String:
		l = ScriptContext::Get().GetTargetList().GetTarget(constStringValue());
		break;

	case variableType_e::Listener:
		return (Listener *)m_data.listenerValue->Pointer();

	default:
		throw ScriptVariableErrors::CastError(GetTypeName(), "listener");
	}

	return l;
}

Listener* ScriptVariable::listenerAt(uintptr_t index) const
{
	switch (type)
	{
	case variableType_e::ConstArray:
		return m_data.constArrayValue->constArrayValue[index].listenerValue();

	case variableType_e::Container:
		return m_data.containerValue->ObjectAt(index);

	case variableType_e::SafeContainer:
		assert(*m_data.safeContainerValue);
		return (*m_data.safeContainerValue)->ObjectAt(index);

	default:
		throw ScriptVariableErrors::CastError(GetTypeName(), "listener");
	}
}

void ScriptVariable::newPointer()
{
	type = variableType_e::Pointer;

	m_data.pointerValue = new ScriptPointer();
	m_data.pointerValue->add(this);
}

void ScriptVariable::newPointer(size_t initialSize)
{
	type = variableType_e::Pointer;

	m_data.pointerValue = new ScriptPointer(initialSize);
	m_data.pointerValue->add(this);
}

str ScriptVariable::stringValue() const
{
	str string;

	switch (type)
	{
	case variableType_e::None:
		return "NIL";

	case variableType_e::ConstString:
		return ScriptContext::Get().GetDirector().GetDictionary().Get(m_data.constStringValue);

	case variableType_e::String:
		return *m_data.stringValue;

	case variableType_e::Integer:
		return str(m_data.long64Value);

	case variableType_e::Float:
		return str(m_data.floatValue);

	case variableType_e::Char:
		return str(m_data.charValue);

	case variableType_e::Listener:
		if (m_data.listenerValue->Pointer())
		{
			// FIXME: return listener name (for entities with targetname?)

			string = "class '" + str(m_data.listenerValue->Pointer()->Class::GetClassname()) + "'";
			return string;
		}
		else
		{
			return "NULL";
		}

	case variableType_e::Vector:
		return str("( ") + str(m_data.vectorValue[0]) + str(" ") + str(m_data.vectorValue[1]) + str(" ") + str(m_data.vectorValue[2]) + str(" )");

	default:
		return "Type: '" + str(GetTypeName()) + "'";
	}

	return "";
}

Vector ScriptVariable::vectorValue() const
{
	const rawchar_t *string;
	float x = 0.f, y = 0.f, z = 0.f;

	switch (type)
	{
	case variableType_e::Vector:
		return Vector(m_data.vectorValue);

	case variableType_e::ConstString:
	case variableType_e::String:
		string = stringValue().c_str();

		if (str::cmp(string, "") == 0) {
			throw ScriptVariableErrors::CastError("empty string", "vector");
		}

		if (*string == '(')
		{
			if (sscanf(string, "(%f %f %f)", &x, &y, &z) != 3)
			{
				if (sscanf(string, "(%f, %f, %f)", &x, &y, &z) != 3) {
					throw ScriptException("Couldn't convert string to vector - malformed string '" + str(string) + "'");
				}
			}
		}
		else
		{
			if (sscanf(string, "%f %f %f", &x, &y, &z) != 3)
			{
				if (sscanf(string, "%f, %f, %f", &x, &y, &z) != 3) {
					throw ScriptException("Couldn't convert string to vector - malformed string '" + str(string) + "'");
				}
			}
		}

		return Vector(x, y, z);
	case variableType_e::Listener:
	{
		if (!m_data.listenerValue->Pointer()) {
			throw ScriptVariableErrors::CastError("NULL", "vector");
		}

		if (!ClassDef::checkInheritance(&SimpleEntity::staticclass(), &m_data.listenerValue->Pointer()->classinfo()))
		{
			throw ScriptVariableErrors::CastError(GetTypeName(), "vector");
		}

		SimpleEntity *ent = (SimpleEntity *)m_data.listenerValue->Pointer();

		const Vector& origin = ent->getOrigin();
		return Vector(origin[0], origin[1], origin[2]);
	}

	default:
		throw ScriptVariableErrors::CastError(GetTypeName(), "vector");
	}
}

void ScriptVariable::setArrayAt(const ScriptVariable& index, const ScriptVariable& value)
{
	return m_data.refValue->setArrayAtRef(index, value);
}

void ScriptVariable::setArrayAtRef(const ScriptVariable& index, const ScriptVariable& value)
{
	int intValue;
	unsigned int uintValue;
	str string;

	switch (type)
	{
	case variableType_e::Vector:
		intValue = index.intValue();

		if (intValue > 2) {
			throw ScriptVariableErrors::TypeIndexOutOfRange("Vector", intValue);
		}

		m_data.vectorValue[intValue] = value.floatValue();
		break;

	case variableType_e::Ref:
		return;

	case variableType_e::None:
		type = variableType_e::Array;

		m_data.arrayValue = new ScriptArrayHolder();

		if (value.type != variableType_e::None)
		{
			m_data.arrayValue->arrayValue[index] = value;
		}

		break;

	case variableType_e::Array:
		if (value.type == variableType_e::None)
		{
			m_data.arrayValue->arrayValue.remove(index);
		}
		else
		{
			m_data.arrayValue->arrayValue[index] = value;
		}
		break;

	case variableType_e::String:
	case variableType_e::ConstString:
		intValue = index.intValue();
		string = stringValue();

		if (intValue >= (intptr_t)string.length()) {
			throw ScriptVariableErrors::TypeIndexOutOfRange("String", intValue);
		}

		string[intValue] = value.charValue();

		setStringValue(string);

		break;

	case variableType_e::ConstArray:
		uintValue = index.intValue();

		if (!uintValue || uintValue > m_data.constArrayValue->size)
		{
			throw ScriptVariableErrors::TypeIndexOutOfRange("array", uintValue);
		}

		if (value.type != variableType_e::None)
		{
			m_data.constArrayValue->constArrayValue[uintValue] = value;
		}
		else
		{
			m_data.constArrayValue->constArrayValue[uintValue].Clear();
		}

		break;

	default:
		throw ScriptVariableErrors::InvalidAppliedType("[]", GetTypeName());
	}
}

void ScriptVariable::setArrayRefValue(ScriptVariable &var)
{
	setRefValue(&(*m_data.refValue)[var]);
}

void ScriptVariable::setCharValue(rawchar_t newvalue)
{
	ClearInternal();

	type = variableType_e::Char;
	m_data.charValue = newvalue;
}

void ScriptVariable::setContainerValue(const con::Container<SafePtr<Listener>>* newvalue)
{
	ClearInternal();

	type = variableType_e::Container;
	m_data.containerValue = newvalue;
}

void ScriptVariable::setSafeContainerValue(ConList *newvalue)
{
	ClearInternal();

	if (newvalue)
	{
		type = variableType_e::SafeContainer;
		m_data.safeContainerValue = new ConListPtr(newvalue);
	}
	else
	{
		type = variableType_e::None;
	}
}

ScriptVariable* ScriptVariable::createConstArrayValue(size_t size)
{
	ScriptConstArrayHolder* constArray = new ScriptConstArrayHolder(size);

	ClearInternal();
	type = variableType_e::ConstArray;

	m_data.constArrayValue = constArray;

	return constArray->constArrayValue + 1;
}

void ScriptVariable::setConstArrayValue(ScriptVariable *pVar, size_t size)
{
	ScriptConstArrayHolder *constArray = new ScriptConstArrayHolder(pVar - 1, size);

	ClearInternal();
	type = variableType_e::ConstArray;

	m_data.constArrayValue = constArray;
}

const_str ScriptVariable::constStringValue() const
{
	if (type == variableType_e::ConstString)
	{
		return m_data.constStringValue;
	}
	else
	{
		return ScriptContext::Get().GetDirector().GetDictionary().Add(stringValue());
	}
}

void ScriptVariable::setConstStringValue(const_str s)
{
	ClearInternal();
	type = variableType_e::ConstString;
	m_data.constStringValue = s;
}

void ScriptVariable::setFloatValue(float newvalue)
{
	ClearInternal();
	type = variableType_e::Float;
	m_data.floatValue = newvalue;
}

void ScriptVariable::setIntValue(uint32_t newvalue)
{
	ClearInternal();
	type = variableType_e::Integer;
	m_data.long64Value = newvalue;
}

void ScriptVariable::setLongValue(uint64_t newvalue)
{
	ClearInternal();
	type = variableType_e::Integer;
	m_data.long64Value = newvalue;
}

void ScriptVariable::setListenerValue(Listener *newvalue)
{
	ClearInternal();

	type = variableType_e::Listener;

	m_data.listenerValue = new ListenerPtr(newvalue);
}

void ScriptVariable::setPointer(const ScriptVariable& newvalue)
{
	if (type == variableType_e::Pointer) {
		m_data.pointerValue->setValue(newvalue);
	}
}

void ScriptVariable::setPointerRef(ScriptVariable& newvalue)
{
	if (type == variableType_e::Pointer) {
		m_data.pointerValue->setValueRef(newvalue, *this);
	}
}

void ScriptVariable::setRefValue(ScriptVariable* ref)
{
	ClearInternal();

	type = variableType_e::Ref;
	m_data.refValue = ref;
}

void ScriptVariable::setStringValue(const str& newvalue)
{
	ClearInternal();
	type = variableType_e::String;
	m_data.stringValue = new str(newvalue);
}

void ScriptVariable::setStringValue(const rawchar_t* newvalue)
{
	ClearInternal();
	type = variableType_e::String;
	m_data.stringValue = new str(newvalue);
}

void ScriptVariable::setVectorValue(const Vector &newvector)
{
	ClearInternal();

	type = variableType_e::Vector;
	m_data.vectorValue = new float[3];
	VecCopy(newvector, m_data.vectorValue);
}

void ScriptVariable::operator+=(const ScriptVariable& value)
{
	const rawchar_t *type1, *type2;

    switch (uint32_t(type + value.type * variableType_e::Max))
    {
    default:
		type1 = GetTypeName();
		type2 = value.GetTypeName();

        Clear();
        throw ScriptVariableErrors::IncompatibleOperator("+", type1, type2);

	case uint32_t(variableType_e::Integer + variableType_e::Integer * variableType_e::Max): // ( int ) + ( int )
		m_data.long64Value = m_data.long64Value + value.m_data.long64Value;
		break;

	case uint32_t(variableType_e::Integer + variableType_e::Float * variableType_e::Max): // ( int ) + ( float )
		setFloatValue((float)m_data.long64Value + value.m_data.floatValue);
		break;

	case uint32_t(variableType_e::Float + variableType_e::Float * variableType_e::Max): // ( float ) + ( float )
		m_data.floatValue = m_data.floatValue + value.m_data.floatValue;
		break;

	case uint32_t(variableType_e::Float + variableType_e::Integer * variableType_e::Max): // ( float ) + ( int )
		m_data.floatValue = m_data.floatValue + value.m_data.long64Value;
		break;

	case uint32_t(variableType_e::String + variableType_e::String * variableType_e::Max):				// ( string )			+		( string )
	case uint32_t(variableType_e::Integer + variableType_e::String * variableType_e::Max):			// ( int )				+		( string )
	case uint32_t(variableType_e::Float + variableType_e::String * variableType_e::Max):				// ( float )			+		( string )
	case uint32_t(variableType_e::Char + variableType_e::String * variableType_e::Max):				// ( rawchar_t )		+		( string )
	case uint32_t(variableType_e::ConstString + variableType_e::String * variableType_e::Max):		// ( const string )		+		( string )
	case uint32_t(variableType_e::Listener + variableType_e::String * variableType_e::Max):			// ( listener )			+		( string )
	case uint32_t(variableType_e::Vector + variableType_e::String * variableType_e::Max):				// ( vector )			+		( string )
	case uint32_t(variableType_e::String + variableType_e::Integer * variableType_e::Max):			// ( string )			+		( int )
	case uint32_t(variableType_e::ConstString + variableType_e::Integer * variableType_e::Max):		// ( const string )		+		( int )
	case uint32_t(variableType_e::String + variableType_e::Float * variableType_e::Max):				// ( string )			+		( float )
	case uint32_t(variableType_e::ConstString + variableType_e::Float * variableType_e::Max):			// ( const string )		+		( float )
	case uint32_t(variableType_e::String + variableType_e::Char * variableType_e::Max):				// ( string )			+		( rawchar_t )
	case uint32_t(variableType_e::ConstString + variableType_e::Char * variableType_e::Max):			// ( const string )		+		( rawchar_t )
	case uint32_t(variableType_e::String + variableType_e::ConstString * variableType_e::Max):		// ( string )			+		( const string )
	case uint32_t(variableType_e::Integer + variableType_e::ConstString * variableType_e::Max):		// ( int )				+		( const string )
	case uint32_t(variableType_e::Float + variableType_e::ConstString * variableType_e::Max):			// ( float )			+		( const string )
	case uint32_t(variableType_e::Char + variableType_e::ConstString * variableType_e::Max):			// ( rawchar_t )		+		( const string )
	case uint32_t(variableType_e::ConstString + variableType_e::ConstString * variableType_e::Max):	// ( const string )		+		( const string )
	case uint32_t(variableType_e::Listener + variableType_e::ConstString * variableType_e::Max):		// ( listener )			+		( const string )
	case uint32_t(variableType_e::Vector + variableType_e::ConstString * variableType_e::Max):		// ( vector )			+		( const string )
	case uint32_t(variableType_e::String + variableType_e::Listener * variableType_e::Max):			// ( string )			+		( listener )
	case uint32_t(variableType_e::ConstString + variableType_e::Listener * variableType_e::Max):		// ( const string )		+		( listener )
	case uint32_t(variableType_e::String + variableType_e::Vector * variableType_e::Max):				// ( string )			+		( vector )
	case uint32_t(variableType_e::ConstString + variableType_e::Vector * variableType_e::Max):		// ( const string )		+		( vector )
		setStringValue(stringValue() + value.stringValue());
		break;

	case uint32_t(variableType_e::Vector + variableType_e::Vector * variableType_e::Max):
		VecAdd(m_data.vectorValue, value.m_data.vectorValue, m_data.vectorValue);
		break;
	}
}

void ScriptVariable::operator-=(const ScriptVariable& value)
{
	const rawchar_t *type1, *type2;

    switch (uint32_t(type + value.type * variableType_e::Max))
    {
    default:
		type1 = GetTypeName();
		type2 = value.GetTypeName();

        Clear();
        throw ScriptVariableErrors::IncompatibleOperator("-", type1, type2);

	case uint32_t(variableType_e::Integer + variableType_e::Integer * variableType_e::Max): // ( int ) - ( int )
		m_data.long64Value = m_data.long64Value - value.m_data.long64Value;
		break;

	case uint32_t(variableType_e::Integer + variableType_e::Float * variableType_e::Max): // ( int ) - ( float )
		setFloatValue((float)m_data.long64Value - value.m_data.floatValue);
		break;

	case uint32_t(variableType_e::Float + variableType_e::Float * variableType_e::Max): // ( float ) - ( float )
		m_data.floatValue = m_data.floatValue - value.m_data.floatValue;
		break;

	case uint32_t(variableType_e::Float + variableType_e::Integer * variableType_e::Max): // ( float ) - ( int )
		m_data.floatValue = m_data.floatValue - value.m_data.long64Value;
		break;

	case uint32_t(variableType_e::Vector + variableType_e::Vector * variableType_e::Max): // ( vector ) - ( vector )
		VecSubtract(m_data.vectorValue, value.m_data.vectorValue, m_data.vectorValue);
		break;
	}
}

void ScriptVariable::operator*=(const ScriptVariable& value)
{
	const rawchar_t *type1, *type2;

    switch (uint32_t(type + value.type * variableType_e::Max))
    {
    default:
		type1 = GetTypeName();
		type2 = value.GetTypeName();

        Clear();
        throw ScriptVariableErrors::IncompatibleOperator("*", type1, type2);

	case uint32_t(variableType_e::Integer + variableType_e::Integer * variableType_e::Max): // ( int ) * ( int )
		m_data.long64Value = m_data.long64Value * value.m_data.long64Value;
		break;

	case uint32_t(variableType_e::Vector + variableType_e::Integer * variableType_e::Max): // ( vector ) * ( int )
		VectorScale(m_data.vectorValue, (float)value.m_data.long64Value, m_data.vectorValue);
		break;

	case uint32_t(variableType_e::Vector + variableType_e::Float * variableType_e::Max): // ( vector ) * ( float )
		VectorScale(m_data.vectorValue, value.m_data.floatValue, m_data.vectorValue);
		break;

	case uint32_t(variableType_e::Integer + variableType_e::Float * variableType_e::Max): // ( int ) * ( float )
		setFloatValue((float)m_data.long64Value * value.m_data.floatValue);
		break;

	case uint32_t(variableType_e::Float + variableType_e::Float * variableType_e::Max): // ( float ) * ( float )
		m_data.floatValue = m_data.floatValue * value.m_data.floatValue;
		break;

	case uint32_t(variableType_e::Float + variableType_e::Integer * variableType_e::Max): // ( float ) * ( int )
		m_data.floatValue = m_data.floatValue * value.m_data.long64Value;
		break;

	case uint32_t(variableType_e::Integer + variableType_e::Vector * variableType_e::Max): // ( int ) * ( vector )
		setVectorValue((float)m_data.long64Value * Vector(value.m_data.vectorValue));
		break;

	case uint32_t(variableType_e::Float + variableType_e::Vector * variableType_e::Max): // ( float ) * ( vector )
		setVectorValue(m_data.floatValue * Vector(value.m_data.vectorValue));
		break;

	case uint32_t(variableType_e::Vector + variableType_e::Vector * variableType_e::Max): // ( vector ) * ( vector )
		m_data.vectorValue[0] = m_data.vectorValue[0] * value.m_data.vectorValue[0];
		m_data.vectorValue[1] = m_data.vectorValue[1] * value.m_data.vectorValue[1];
		m_data.vectorValue[2] = m_data.vectorValue[2] * value.m_data.vectorValue[2];
		break;
	}
}

void ScriptVariable::operator/=(const ScriptVariable& value)
{
	const rawchar_t *type1, *type2;

    switch (uint32_t(type + value.type * variableType_e::Max))
    {
    default:
		type1 = GetTypeName();
		type2 = value.GetTypeName();

        Clear();
        throw ScriptVariableErrors::IncompatibleOperator("/", type1, type2);

	case uint32_t(variableType_e::Integer + variableType_e::Integer * variableType_e::Max): // ( int ) / ( int )
		if (value.m_data.long64Value == 0) {
			throw ScriptVariableErrors::DivideByZero();
		}

		m_data.long64Value = m_data.long64Value / value.m_data.long64Value;
		break;

	case uint32_t(variableType_e::Vector + variableType_e::Integer * variableType_e::Max): // ( vector ) / ( int )
		if (value.m_data.long64Value == 0) {
			throw ScriptVariableErrors::DivideByZero();
		}

		(Vector)m_data.vectorValue = (Vector)m_data.vectorValue / (float)value.m_data.long64Value;
		break;

	case uint32_t(variableType_e::Vector + variableType_e::Float * variableType_e::Max): // ( vector ) / ( float )
		if (value.m_data.floatValue == 0) {
			throw ScriptVariableErrors::DivideByZero();
		}

		m_data.vectorValue[0] = m_data.vectorValue[0] / value.m_data.floatValue;
		m_data.vectorValue[1] = m_data.vectorValue[1] / value.m_data.floatValue;
		m_data.vectorValue[2] = m_data.vectorValue[2] / value.m_data.floatValue;
		break;

	case uint32_t(variableType_e::Integer + variableType_e::Float * variableType_e::Max): // ( int ) / ( float )
		if (value.m_data.floatValue == 0) {
			throw ScriptVariableErrors::DivideByZero();
		}

		setFloatValue((float)m_data.long64Value / value.m_data.floatValue);
		break;

	case uint32_t(variableType_e::Float + variableType_e::Float * variableType_e::Max): // ( float ) / ( float )
		if (value.m_data.floatValue == 0) {
			throw ScriptVariableErrors::DivideByZero();
		}

		m_data.floatValue = m_data.floatValue / value.m_data.floatValue;
		break;

	case uint32_t(variableType_e::Float + variableType_e::Integer * variableType_e::Max): // ( float ) / ( int )
		if (value.m_data.long64Value == 0) {
			throw ScriptVariableErrors::DivideByZero();
		}

		m_data.floatValue = m_data.floatValue / value.m_data.long64Value;
		break;

	case uint32_t(variableType_e::Integer + variableType_e::Vector * variableType_e::Max): // ( int ) / ( vector )
		if (m_data.long64Value == 0) {
			throw ScriptVariableErrors::DivideByZero();
		}

		setVectorValue((float)m_data.long64Value / Vector(value.m_data.vectorValue));
		break;

	case uint32_t(variableType_e::Float + variableType_e::Vector * variableType_e::Max): // ( float ) / ( vector )
		if (m_data.floatValue == 0) {
			throw ScriptVariableErrors::DivideByZero();
		}

		setVectorValue(m_data.floatValue / Vector(value.m_data.vectorValue));
		break;

	case uint32_t(variableType_e::Vector + variableType_e::Vector * variableType_e::Max): // ( vector ) / ( vector )
		m_data.vectorValue = vec_zero;

		if (value.m_data.vectorValue[0] != 0) {
			m_data.vectorValue[0] = m_data.vectorValue[0] / value.m_data.vectorValue[0];
		}

		if (value.m_data.vectorValue[1] != 0) {
			m_data.vectorValue[1] = m_data.vectorValue[1] / value.m_data.vectorValue[1];
		}

		if (value.m_data.vectorValue[2] != 0) {
			m_data.vectorValue[2] = m_data.vectorValue[2] / value.m_data.vectorValue[2];
		}
		break;
	}
}

void ScriptVariable::operator%=(const ScriptVariable& value)
{
	float mult = 0.0f;
	const rawchar_t *type1, *type2;

    switch (uint32_t(type + value.type * variableType_e::Max))
    {
    default:
		type1 = GetTypeName();
		type2 = value.GetTypeName();

        Clear();
        throw ScriptVariableErrors::IncompatibleOperator("%", type1, type2);

	case uint32_t(variableType_e::Integer + variableType_e::Integer * variableType_e::Max): // ( int ) % ( int )
		if (value.m_data.long64Value == 0) {
			throw ScriptVariableErrors::DivideByZero();
		}

		m_data.long64Value = m_data.long64Value % value.m_data.long64Value;
		break;

	case uint32_t(variableType_e::Vector + variableType_e::Integer * variableType_e::Max): // ( vector ) % ( int )
		if (value.m_data.long64Value == 0) {
			throw ScriptVariableErrors::DivideByZero();
		}

		m_data.vectorValue[0] = (float)fmodf(m_data.vectorValue[0], (float)value.m_data.long64Value);
		m_data.vectorValue[1] = (float)fmodf(m_data.vectorValue[1], (float)value.m_data.long64Value);
		m_data.vectorValue[2] = (float)fmodf(m_data.vectorValue[2], (float)value.m_data.long64Value);
		break;

	case uint32_t(variableType_e::Vector + variableType_e::Float * variableType_e::Max): // ( vector ) % ( float )
		if (value.m_data.floatValue == 0) {
			throw ScriptVariableErrors::DivideByZero();
		}

		m_data.vectorValue[0] = (float)fmodf(m_data.vectorValue[0], value.m_data.floatValue);
		m_data.vectorValue[1] = (float)fmodf(m_data.vectorValue[1], value.m_data.floatValue);
		m_data.vectorValue[2] = (float)fmodf(m_data.vectorValue[2], value.m_data.floatValue);
		break;

	case uint32_t(variableType_e::Integer + variableType_e::Float * variableType_e::Max): // ( int ) % ( float )
		if (value.m_data.floatValue == 0) {
			throw ScriptVariableErrors::DivideByZero();
		}

		setFloatValue(fmodf((float)m_data.long64Value, value.m_data.floatValue));
		break;

	case uint32_t(variableType_e::Float + variableType_e::Float * variableType_e::Max): // ( float ) % ( float )
		if (value.m_data.floatValue == 0) {
			throw ScriptVariableErrors::DivideByZero();
		}

		m_data.floatValue = fmodf(m_data.floatValue, value.m_data.floatValue);
		break;

	case uint32_t(variableType_e::Float + variableType_e::Integer * variableType_e::Max): // ( float ) % ( int )
		if (value.m_data.long64Value == 0) {
			throw ScriptVariableErrors::DivideByZero();
		}

		m_data.floatValue = fmodf(m_data.floatValue, (float)value.m_data.long64Value);
		break;

	case uint32_t(variableType_e::Integer + variableType_e::Vector * variableType_e::Max): // ( int ) % ( vector )
		mult = (float)m_data.long64Value;

		if (mult == 0) {
			throw ScriptVariableErrors::DivideByZero();
		}

		setVectorValue(vec_zero);

		m_data.vectorValue[0] = fmodf(value.m_data.vectorValue[0], mult);
		m_data.vectorValue[1] = fmodf(value.m_data.vectorValue[1], mult);
		m_data.vectorValue[2] = fmodf(value.m_data.vectorValue[2], mult);
		break;

	case uint32_t(variableType_e::Float + variableType_e::Vector * variableType_e::Max): // ( float ) % ( vector )
		mult = m_data.floatValue;

		if (mult == 0) {
			throw ScriptVariableErrors::DivideByZero();
		}

		setVectorValue(vec_zero);

		m_data.vectorValue[0] = fmodf(m_data.vectorValue[0], mult);
		m_data.vectorValue[1] = fmodf(m_data.vectorValue[1], mult);
		m_data.vectorValue[2] = fmodf(m_data.vectorValue[2], mult);
		break;

	case uint32_t(variableType_e::Vector + variableType_e::Vector * variableType_e::Max): // ( vector ) % ( vector )
		m_data.vectorValue = vec_zero;

		if (value.m_data.vectorValue[0] != 0) {
			m_data.vectorValue[0] = fmodf(m_data.vectorValue[0], value.m_data.vectorValue[0]);
		}

		if (value.m_data.vectorValue[1] != 0) {
			m_data.vectorValue[1] = fmodf(m_data.vectorValue[1], value.m_data.vectorValue[1]);
		}

		if (value.m_data.vectorValue[2] != 0) {
			m_data.vectorValue[2] = fmodf(m_data.vectorValue[2], value.m_data.vectorValue[2]);
		}

		break;
	}
}

void ScriptVariable::operator&=(const ScriptVariable& value)
{
	const rawchar_t *type1, *type2;

    switch (uint32_t(type + value.type * variableType_e::Max))
    {
    default:
		type1 = GetTypeName();
		type2 = value.GetTypeName();

        Clear();
        throw ScriptVariableErrors::IncompatibleOperator("&", type1, type2);

	case uint32_t(variableType_e::Integer + variableType_e::Integer * variableType_e::Max): // ( int ) &= ( int )
		m_data.long64Value &= value.m_data.long64Value;
		break;
	}
}

void ScriptVariable::operator^=(const ScriptVariable& value)
{
	const rawchar_t *type1, *type2;

    switch (uint32_t(type + value.type * variableType_e::Max))
    {
    default:
		type1 = GetTypeName();
		type2 = value.GetTypeName();

        Clear();
        throw ScriptVariableErrors::IncompatibleOperator("^", type1, type2);

	case uint32_t(variableType_e::Integer + variableType_e::Integer * variableType_e::Max): // ( int ) ^= ( int )
		m_data.long64Value ^= value.m_data.long64Value;
		break;
	}
}

void ScriptVariable::operator|=(const ScriptVariable& value)
{
	const rawchar_t *type1, *type2;

    switch (uint32_t(type + value.type * variableType_e::Max))
    {
    default:
		type1 = GetTypeName();
		type2 = value.GetTypeName();

        Clear();
        throw ScriptVariableErrors::IncompatibleOperator("|", type1, type2);

	case uint32_t(variableType_e::Integer + variableType_e::Integer * variableType_e::Max): // ( int ) |= ( int )
		m_data.long64Value |= value.m_data.long64Value;
		break;
	}
}

void ScriptVariable::operator<<=(const ScriptVariable& value)
{
	const rawchar_t *type1, *type2;

    switch (uint32_t(type + value.type * variableType_e::Max))
    {
    default:
		type1 = GetTypeName();
		type2 = value.GetTypeName();

        Clear();
        throw ScriptVariableErrors::IncompatibleOperator("<<", type1, type2);

	case uint32_t(variableType_e::Integer + variableType_e::Integer * variableType_e::Max): // ( int ) <<= ( int )
		m_data.long64Value <<= value.m_data.long64Value;
		break;
	}
}

void ScriptVariable::operator>>=(const ScriptVariable& value)
{
	const rawchar_t *type1, *type2;

    switch (uint32_t(type + value.type * variableType_e::Max))
    {
    default:
		type1 = GetTypeName();
		type2 = value.GetTypeName();

        Clear();
        throw ScriptVariableErrors::IncompatibleOperator(">>", type1, type2);

	// ( int ) >>= ( int )
	case uint32_t(variableType_e::Integer + variableType_e::Integer * variableType_e::Max):
		m_data.long64Value >>= value.m_data.long64Value;
		break;
	}
}

bool ScriptVariable::operator!=(const ScriptVariable &value) const
{
	return !(*this == value);
}

bool ScriptVariable::operator==(const ScriptVariable &value) const
{
	switch (uint32_t(type + value.type * variableType_e::Max))
	{
	// ( lval )	==	( nil )
	// ( nil )	==	( rval )
	default:
		return false;

	// ( nil ) == ( nil )
	case uint32_t(variableType_e::None + variableType_e::None * variableType_e::Max):
		return true;

	// ( listener ) == ( listener )
	case uint32_t(variableType_e::Listener + variableType_e::Listener * variableType_e::Max):
	{
		AbstractClass *lval = nullptr;
		AbstractClass *rval = nullptr;

		if (m_data.listenerValue)
		{
			lval = m_data.listenerValue->Pointer();
		}

		if (value.m_data.listenerValue)
		{
			rval = value.m_data.listenerValue->Pointer();
		}

		return lval == rval;
	}

	// ( int ) == ( int )
	case uint32_t(variableType_e::Integer + variableType_e::Integer * variableType_e::Max):
		return m_data.long64Value == value.m_data.long64Value;

	// ( int ) == ( char )
	case uint32_t(variableType_e::Integer + variableType_e::Char * variableType_e::Max):
		return m_data.long64Value == value.m_data.charValue;

	// ( int ) == ( float )
    case uint32_t(variableType_e::Integer + variableType_e::Float * variableType_e::Max):
        return fabs(m_data.long64Value - value.m_data.floatValue) < 0.0001;

	// ( float ) == ( float )
    case uint32_t(variableType_e::Float + variableType_e::Float * variableType_e::Max):
        return fabs(m_data.floatValue - value.m_data.floatValue) < 0.0001;

	// ( float ) == ( int )
	case uint32_t(variableType_e::Float + variableType_e::Integer * variableType_e::Max):
        return fabs(m_data.floatValue - value.m_data.long64Value) < 0.0001;

	// ( char ) == ( int )
	case uint32_t(variableType_e::Char + variableType_e::Integer * variableType_e::Max):
		return m_data.charValue == value.m_data.long64Value;

	// ( char ) == ( char )
	case uint32_t(variableType_e::Char + variableType_e::Char * variableType_e::Max):
		return m_data.charValue == value.m_data.charValue;

	case uint32_t(variableType_e::ConstString + variableType_e::ConstString * variableType_e::Max):	// ( const string )		==		( const string )
		return m_data.long64Value == value.m_data.long64Value;

	case uint32_t(variableType_e::String + variableType_e::String * variableType_e::Max):				// ( string )			==		( string )
	case uint32_t(variableType_e::Integer + variableType_e::String * variableType_e::Max):				// ( int )				==		( string )
	case uint32_t(variableType_e::Float + variableType_e::String * variableType_e::Max):				// ( float )			==		( string )
	case uint32_t(variableType_e::Char + variableType_e::String * variableType_e::Max):				// ( rawchar_t )				==		( string )
	case uint32_t(variableType_e::ConstString + variableType_e::String * variableType_e::Max):			// ( const string )		==		( string )
	case uint32_t(variableType_e::Listener + variableType_e::String * variableType_e::Max):			// ( listener )			==		( string )
	case uint32_t(variableType_e::Vector + variableType_e::String * variableType_e::Max):				// ( vector )			==		( string )
	case uint32_t(variableType_e::String + variableType_e::ConstString * variableType_e::Max):			// ( string )			==		( const string )
	case uint32_t(variableType_e::Integer + variableType_e::ConstString * variableType_e::Max):		// ( int )				==		( const string )
	case uint32_t(variableType_e::Float + variableType_e::ConstString * variableType_e::Max):			// ( float )			==		( const string )
	case uint32_t(variableType_e::Char + variableType_e::ConstString * variableType_e::Max):			// ( rawchar_t )				==		( const string )
	case uint32_t(variableType_e::Listener + variableType_e::ConstString * variableType_e::Max):		// ( listener )			==		( const string )
	case uint32_t(variableType_e::Vector + variableType_e::ConstString * variableType_e::Max):			// ( vector )			==		( const string )
	case uint32_t(variableType_e::String + variableType_e::Integer * variableType_e::Max):				// ( string )			==		( int )
	case uint32_t(variableType_e::ConstString + variableType_e::Integer * variableType_e::Max):		// ( const string )		==		( int )
	case uint32_t(variableType_e::String + variableType_e::Float * variableType_e::Max):				// ( string )			==		( float )
	case uint32_t(variableType_e::ConstString + variableType_e::Float * variableType_e::Max):			// ( const string )		==		( float )
	case uint32_t(variableType_e::String + variableType_e::Char * variableType_e::Max):				// ( string )			==		( rawchar_t )
	case uint32_t(variableType_e::ConstString + variableType_e::Char * variableType_e::Max):			// ( const string )		==		( rawchar_t )
	case uint32_t(variableType_e::String + variableType_e::Listener * variableType_e::Max):			// ( string )			==		( listener )
	case uint32_t(variableType_e::ConstString + variableType_e::Listener * variableType_e::Max):		// ( const string )		==		( listener )
	case uint32_t(variableType_e::String + variableType_e::Vector * variableType_e::Max):				// ( string )			==		( vector )
	case uint32_t(variableType_e::ConstString + variableType_e::Vector * variableType_e::Max):			// ( const string )		==		( vector )
	{
		str lval = stringValue();
		str rval = value.stringValue();

		return (!lval.length() && !rval.length()) || (lval == rval);
	}

	case uint32_t(variableType_e::Vector + variableType_e::Vector * variableType_e::Max): // ( vector ) == ( vector )
		return VecCompare(m_data.vectorValue, value.m_data.vectorValue, 0.0001) ? true : false;
	}
}

const mfuse::ScriptVariable& ScriptVariable::operator[](const ScriptVariable& index) const
{
	size_t i;

	switch (type)
	{
	case variableType_e::None:
		return noneVar;

	case variableType_e::Array:
		return m_data.arrayValue->arrayValue[index];

	case variableType_e::ConstArray:
		i = index.intValue();

		if (i < 1 || i > m_data.constArrayValue->size)
		{
			throw ScriptVariableErrors::TypeIndexOutOfRange("array", i);
		}

		return m_data.constArrayValue->constArrayValue[i];

	default:
		throw ScriptVariableErrors::InvalidAppliedType("[]", GetTypeName());
	}
}

ScriptVariable &ScriptVariable::operator[](const ScriptVariable& index)
{
	size_t i;

	switch (type)
	{
	case variableType_e::None:
		type = variableType_e::Array;

		m_data.arrayValue = new ScriptArrayHolder();
		return m_data.arrayValue->arrayValue[index];

	case variableType_e::Array:
		return m_data.arrayValue->arrayValue[index];

	case variableType_e::ConstArray:
		i = index.intValue();

		if (i == 0 || i > m_data.constArrayValue->size)
		{
			throw ScriptVariableErrors::TypeIndexOutOfRange("array", i);
		}

		return m_data.constArrayValue->constArrayValue[i];

	default:
		throw ScriptVariableErrors::InvalidAppliedType("[]", GetTypeName());
	}
}

ScriptVariable& ScriptVariable::operator[](uintptr_t index) const
{
	return constArrayElement(index);
}

ScriptVariable *ScriptVariable::operator*()
{
	return m_data.refValue;
}

ScriptVariable& ScriptVariable::constArrayElement(uintptr_t index) const
{
	assert(GetType() == variableType_e::ConstArray);
	assert(m_data.constArrayValue);
	assert(index > 0 && index <= m_data.constArrayValue->size);
	return m_data.constArrayValue->constArrayValue[index];
}

void ScriptVariable::complement()
{
	if (type == variableType_e::Integer)
	{
		m_data.long64Value = ~m_data.long64Value;
	}
	else {
		setIntValue(~intValue());
	}
}

void ScriptVariable::greaterthan(const ScriptVariable &variable)
{
	const rawchar_t *type1, *type2;

	switch (uint32_t(type + variable.type * variableType_e::Max))
	{
	default:
		type1 = GetTypeName();
		type2 = variable.GetTypeName();

		Clear();
		throw ScriptVariableErrors::IncompatibleOperator(">", type1, type2);

	// ( int ) > ( int )
	case uint32_t(variableType_e::Integer + variableType_e::Integer * variableType_e::Max):
		m_data.long64Value = m_data.long64Value > variable.m_data.long64Value;
		break;

	// ( int ) > ( float )
	case uint32_t(variableType_e::Integer + variableType_e::Float * variableType_e::Max):
		m_data.long64Value = m_data.long64Value - variable.m_data.floatValue >= 0.0001;
		break;

	// ( float ) > ( float )
	case uint32_t(variableType_e::Float + variableType_e::Float * variableType_e::Max):
		type = variableType_e::Integer;
		m_data.long64Value = m_data.floatValue - variable.m_data.floatValue >= 0.0001;
		break;

	// ( float ) > ( int )
	case uint32_t(variableType_e::Float + variableType_e::Integer * variableType_e::Max):
		type = variableType_e::Integer;
		m_data.long64Value = m_data.floatValue - variable.m_data.long64Value >= 0.0001;
		break;

	// ( char ) <= ( char )
	case uint32_t(variableType_e::Char + variableType_e::Char * variableType_e::Max):
		type = variableType_e::Integer;
		m_data.long64Value = m_data.charValue > variable.m_data.charValue;
		break;
	}
}

void ScriptVariable::greaterthanorequal(const ScriptVariable &variable)
{
	const rawchar_t *type1, *type2;

	switch (uint32_t(type + variable.type * variableType_e::Max))
	{
	default:
		type1 = GetTypeName();
		type2 = variable.GetTypeName();

		Clear();
		throw ScriptVariableErrors::IncompatibleOperator(">=", type1, type2);

	// ( int ) >= ( int )
	case uint32_t(variableType_e::Integer + variableType_e::Integer * variableType_e::Max):
		m_data.long64Value = m_data.long64Value >= variable.m_data.long64Value;
		break;

	// ( int ) >= ( float )
	case uint32_t(variableType_e::Integer + variableType_e::Float * variableType_e::Max):
		m_data.long64Value = m_data.long64Value - variable.m_data.floatValue > -0.0001;
		break;

	// ( float ) >= ( float )
	case uint32_t(variableType_e::Float + variableType_e::Float * variableType_e::Max):
		type = variableType_e::Integer;
		m_data.long64Value = m_data.floatValue - variable.m_data.floatValue > -0.0001;
		break;

	// ( float ) >= ( int )
	case uint32_t(variableType_e::Float + variableType_e::Integer * variableType_e::Max):
		type = variableType_e::Integer;
		m_data.long64Value = m_data.floatValue - variable.m_data.long64Value > -0.0001;
		break;

	// ( char ) >= ( char )
	case uint32_t(variableType_e::Char + variableType_e::Char * variableType_e::Max):
		type = variableType_e::Integer;
		m_data.long64Value = m_data.charValue >= variable.m_data.charValue;
		break;
	}
}

void ScriptVariable::lessthan(const ScriptVariable &variable)
{
	const rawchar_t *type1, *type2;

	switch (uint32_t(type + variable.type * variableType_e::Max))
	{
	default:
		type1 = GetTypeName();
		type2 = variable.GetTypeName();

		Clear();
		throw ScriptVariableErrors::IncompatibleOperator("<", type1, type2);

	// ( int ) < ( int )
	case uint32_t(variableType_e::Integer + variableType_e::Integer * variableType_e::Max):
		m_data.long64Value = m_data.long64Value < variable.m_data.long64Value;
		break;

	// ( int ) < ( float )
	case uint32_t(variableType_e::Integer + variableType_e::Float * variableType_e::Max):
		m_data.long64Value = m_data.long64Value - variable.m_data.floatValue <= -0.0001;
		break;

	// ( float ) < ( float )
	case uint32_t(variableType_e::Float + variableType_e::Float * variableType_e::Max):
		type = variableType_e::Integer;
		m_data.long64Value = m_data.floatValue - variable.m_data.floatValue <= -0.0001;
		break;

	// ( float ) < ( int )
	case uint32_t(variableType_e::Float + variableType_e::Integer * variableType_e::Max):
		type = variableType_e::Integer;
		m_data.long64Value = m_data.floatValue - variable.m_data.long64Value <= -0.0001;
		break;

	// ( char ) < ( char )
	case uint32_t(variableType_e::Char + variableType_e::Char * variableType_e::Max):
		type = variableType_e::Integer;
		m_data.long64Value = m_data.charValue < variable.m_data.charValue;
		break;
	}
}

void ScriptVariable::lessthanorequal(const ScriptVariable &variable)
{
	const rawchar_t *type1, *type2;

	switch (uint32_t(type + variable.type * variableType_e::Max))
	{
	default:
		type1 = GetTypeName();
		type2 = variable.GetTypeName();

		Clear();
		throw ScriptVariableErrors::IncompatibleOperator("<=", type1, type2);

	// ( int ) <= ( int )
	case uint32_t(variableType_e::Integer + variableType_e::Integer * variableType_e::Max):
		m_data.long64Value = m_data.long64Value <= variable.m_data.long64Value;
		break;

	// ( int ) <= ( float )
	case uint32_t(variableType_e::Integer + variableType_e::Float * variableType_e::Max):
		m_data.long64Value = m_data.long64Value - variable.m_data.floatValue < 0.0001;
		break;

	// ( float ) <= ( float )
	case uint32_t(variableType_e::Float + variableType_e::Float * variableType_e::Max):
		type = variableType_e::Integer;
		m_data.long64Value = m_data.floatValue - variable.m_data.floatValue < 0.0001;
		break;

	// ( float ) <= ( int )
	case uint32_t(variableType_e::Float + variableType_e::Integer * variableType_e::Max):
		type = variableType_e::Integer;
		m_data.long64Value = m_data.floatValue - variable.m_data.long64Value < 0.0001;
		break;

	// ( char ) <= ( char )
	case uint32_t(variableType_e::Char + variableType_e::Char * variableType_e::Max):
		type = variableType_e::Integer;
		m_data.long64Value = m_data.charValue <= variable.m_data.charValue;
		break;
	}
}

void ScriptVariable::minus()
{
	switch(type)
	{
	case variableType_e::Integer:
		m_data.long64Value = -(int64_t)m_data.long64Value;
		break;
	case variableType_e::Float:
		m_data.floatValue = -m_data.floatValue;
		break;
	default:
		setLongValue(-(int64_t)longValue());
	}
}

ScriptVariable& ScriptVariable::operator++(int)
{
	switch (type)
	{
	case variableType_e::None:
		return *this;

	case variableType_e::Integer:
		m_data.long64Value++;
		break;

	case variableType_e::Pointer:
		ClearPointerInternal();
		break;

	case variableType_e::Float:
		m_data.floatValue += 1.f;
		break;

	default:
		setIntValue(intValue() + 1);
		break;
	}

	return *this;
}

ScriptVariable& ScriptVariable::operator--(int)
{
	switch (type)
	{
	case variableType_e::None:
		return *this;

	case variableType_e::Integer:
		m_data.long64Value--;
		break;

	case variableType_e::Pointer:
		ClearPointerInternal();
		break;

	case variableType_e::Float:
		m_data.floatValue -= 1.f;
		break;

	default:
		setIntValue(intValue() - 1);
		break;
	}

	return *this;
}

void ScriptVariable::setDataInternal(const ScriptVariable& variable)
{
	switch (variable.type)
	{
	default:
		break;

	case variableType_e::ConstString:
	case variableType_e::Float:
	case variableType_e::Char:
	case variableType_e::Integer:
		m_data = variable.m_data;
		break;

	case variableType_e::String:
		m_data.stringValue = new str(*variable.m_data.stringValue);
		break;

	case variableType_e::Listener:
		m_data.listenerValue = new ListenerPtr(*variable.m_data.listenerValue);
		break;

	case variableType_e::Array:
		m_data.arrayValue = variable.m_data.arrayValue;
		m_data.arrayValue->refCount++;
		break;

	case variableType_e::ConstArray:
		m_data.constArrayValue = variable.m_data.constArrayValue;
		m_data.constArrayValue->refCount++;
		break;

	case variableType_e::Container:
		// it mustn't be deleted afterwards
		m_data.containerValue = variable.m_data.containerValue;
		break;

	case variableType_e::SafeContainer:
		m_data.safeContainerValue = new ConListPtr(*variable.m_data.safeContainerValue);
		break;

	case variableType_e::Pointer:
		m_data.pointerValue = variable.m_data.pointerValue;
		m_data.pointerValue->add(this);
		break;

	case variableType_e::Vector:
		m_data.vectorValue = (float*)new float[3];
		VecCopy(variable.m_data.vectorValue, m_data.vectorValue);
		break;
	}

	type = variable.type;
}

ScriptVariableList::ScriptVariableList()
{

}

ScriptVariableList::~ScriptVariableList()
{

}

void ScriptVariableList::ClearList()
{
	list.clear();
}

ScriptVariable* ScriptVariableList::GetOrCreateVariable(const str& name)
{
	return GetOrCreateVariable(ScriptContext::Get().GetDirector().GetDictionary().Add(name));
}

ScriptVariable* ScriptVariableList::GetOrCreateVariable(const_str name)
{
	ScriptVariable& var = list.addKeyValue(name);
	assert(var.GetKey());
	return &var;
}

const ScriptVariable *ScriptVariableList::GetVariable(const str& name) const
{
	return GetVariable(ScriptContext::Get().GetDirector().GetDictionary().Add(name));
}

ScriptVariable* ScriptVariableList::GetVariable(const str& name)
{
	return GetVariable(ScriptContext::Get().GetDirector().GetDictionary().Add(name));
}

const ScriptVariable *ScriptVariableList::GetVariable(const_str name) const
{
	return list.findKeyValue(name);
}

ScriptVariable* ScriptVariableList::GetVariable(const_str name)
{
	return list.findKeyValue(name);
}

ScriptVariable * ScriptVariableList::SetVariable(const rawchar_t *name, int value)
{
	ScriptVariable* variable = GetOrCreateVariable(name);

	variable->setIntValue(value);

	return variable;
}

ScriptVariable* ScriptVariableList::SetVariable(const rawchar_t* name, float value)
{
	ScriptVariable* variable = GetOrCreateVariable(name);

	variable->setFloatValue(value);

	return variable;
}

ScriptVariable* ScriptVariableList::SetVariable(const rawchar_t* name, const rawchar_t* value)
{
	ScriptVariable* variable = GetOrCreateVariable(name);

	variable->setStringValue(value);

	return variable;
}

ScriptVariable* ScriptVariableList::SetVariable(const rawchar_t* name, Entity * value)
{
	ScriptVariable* variable = GetOrCreateVariable(name);

	variable->setListenerValue((Listener*)value);

	return variable;
}

ScriptVariable* ScriptVariableList::SetVariable(const rawchar_t* name, Listener * value)
{
	ScriptVariable* variable = GetOrCreateVariable(name);

	variable->setListenerValue(value);

	return variable;
}

ScriptVariable* ScriptVariableList::SetVariable(const rawchar_t* name, Vector & value)
{
	ScriptVariable* variable = GetOrCreateVariable(name);

	variable->setVectorValue(value);

	return variable;
}

ScriptVariable* ScriptVariableList::SetVariable(const rawchar_t* name, const ScriptVariable& value)
{
	ScriptVariable* variable = GetOrCreateVariable(name);

	*variable = value;

	return variable;
}

ScriptVariable* ScriptVariableList::SetVariable(const_str name, const ScriptVariable& value)
{
	ScriptVariable* variable = GetOrCreateVariable(name);

	*variable = value;

	return variable;
}

ScriptVariable* ScriptVariableList::SetVariable(const_str name, ScriptVariable&& value)
{
	ScriptVariable* variable = GetOrCreateVariable(name);

	*variable = std::move(value);

	return variable;
}

void ScriptVariableList::Archive(Archiver& arc)
{
	Class::Archive(arc);
	list.Archive(arc);
}

ScriptVariableErrors::CastError::CastError(const rawchar_t* sourceVal, const rawchar_t* targetVal)
	: source(sourceVal)
	, target(targetVal)
{
}

const rawchar_t* ScriptVariableErrors::CastError::getSource() const noexcept
{
	return source;
}

const rawchar_t* ScriptVariableErrors::CastError::getTarget() const noexcept
{
	return target;
}

const char* ScriptVariableErrors::CastError::what() const noexcept
{
	if (!filled()) {
		fill("Cannot cast '" + str(source) + "' to '" + target + "'");
	}
	
	return Messageable::what();
}

ScriptVariableErrors::IncompatibleOperator::IncompatibleOperator(const rawchar_t* opVal, const rawchar_t* leftTypeVal, const rawchar_t* rightTypeVal)
	: op(opVal)
	, leftType(leftTypeVal)
	, rightType(rightTypeVal)
{

}

const rawchar_t* ScriptVariableErrors::IncompatibleOperator::getOperator() const noexcept
{
	return op;
}

const rawchar_t* ScriptVariableErrors::IncompatibleOperator::getLeftType() const noexcept
{
	return leftType;
}

const rawchar_t* ScriptVariableErrors::IncompatibleOperator::getRightType() const noexcept
{
	return rightType;
}

const char* ScriptVariableErrors::IncompatibleOperator::what() const noexcept
{
	if (!filled()) {
		fill("binary '" + str(op) + "' applied to incompatible types '" + str(leftType) + "' and '" + str(rightType) + "'");
	}

	return Messageable::what();
}

const char* ScriptVariableErrors::DivideByZero::what() const noexcept
{
	return "Division by zero error";
}

ScriptVariableErrors::InvalidAppliedType::InvalidAppliedType(const rawchar_t* opValue, const rawchar_t* typeValue)
	: op(opValue)
	, typeName(typeValue)
{
}

const rawchar_t* ScriptVariableErrors::InvalidAppliedType::getOperator() const noexcept
{
	return op;
}

const rawchar_t* ScriptVariableErrors::InvalidAppliedType::getTypeName() const noexcept
{
	return typeName;
}

const char* ScriptVariableErrors::InvalidAppliedType::what() const noexcept
{
	if (!filled()) {
		fill(str(op) + " applied to invalid type: '" + str(typeName) + "'");
	}

	return Messageable::what();
}

ScriptVariableErrors::IndexOutOfRange::IndexOutOfRange(uintptr_t indexValue)
	: index(indexValue)
{
}

uintptr_t ScriptVariableErrors::IndexOutOfRange::getIndex() const noexcept
{
	return index;
}

const char* ScriptVariableErrors::IndexOutOfRange::what() const noexcept
{
	if (!filled()) {
		fill("Index '" + str(index) + "' out of range");
	}

	return Messageable::what();
}

ScriptVariableErrors::TypeIndexOutOfRange::TypeIndexOutOfRange(const rawchar_t* typeValue, uintptr_t indexValue)
	: IndexOutOfRange(indexValue)
	, typeName(typeValue)
{
}

const rawchar_t* ScriptVariableErrors::TypeIndexOutOfRange::getType() const noexcept
{
	return typeName;
}

const char* ScriptVariableErrors::TypeIndexOutOfRange::what() const noexcept
{
	if (!filled()) {
		fill(str(typeName) + " index '" + str(getIndex()) + "' out of range");
	}

	return Messageable::what();
}

ScriptVariableErrors::BadHashCodeValue::BadHashCodeValue(str&& hashCodeRef)
	: hashCode(hashCodeRef)
{
}

const mfuse::str& ScriptVariableErrors::BadHashCodeValue::getHashCode()
{
	return hashCode;
}

const char* ScriptVariableErrors::BadHashCodeValue::what() const noexcept
{
	if (!filled()) {
		fill("Bad hash code value: " + hashCode);
	}

	return Messageable::what();
}

ScriptVariableIterator::ScriptVariableIterator(const ScriptVariable& var)
	: owningVar(var)
	, data(initData(var))
	, count(0)
{
}

ScriptVariableIterator::~ScriptVariableIterator()
{
}

ScriptVariableIterator::operator bool() const
{
	return isValid();
}

ScriptVariableIterator ScriptVariableIterator::operator++(int)
{
	ScriptVariableIterator old(*this);
	++*this;
	return old;
}

ScriptVariableIterator& ScriptVariableIterator::operator++()
{
	const scriptData_u varData = owningVar.GetData();

	switch (owningVar.GetType())
	{
	case variableType_e::Container:
		++count;
		++data.index;
		done = data.index > varData.containerValue->NumObjects();
		break;
	case variableType_e::SafeContainer:
		++count;
		++data.index;
		done = !varData.safeContainerValue->Pointer() || data.index > varData.safeContainerValue->Pointer()->NumObjects();
		break;
	case variableType_e::ConstArray:
		++count;
		++data.index;
		done = data.index > varData.constArrayValue->size;
		break;

	case variableType_e::Array:
		++count;
		done = !data.en.NextKey();
		break;

	case variableType_e::String:
		++count;
		++data.index;
		done = data.index >= varData.stringValue->length();
		break;
	case variableType_e::Vector:
		++count;
		++data.index;
		done = data.index > 2;
		break;

	default:
		break;
	}

	return *this;
}

ScriptVariableIterator::u ScriptVariableIterator::initData(const ScriptVariable& var)
{
	const scriptData_u varData = var.GetData();

	switch (var.GetType())
	{
	case variableType_e::Container:
		done = !varData.containerValue->NumObjects();
		return u(1);
	case variableType_e::SafeContainer:
		done = !varData.safeContainerValue->Pointer() || !varData.safeContainerValue->Pointer()->NumObjects();
		return u(1);
	case variableType_e::ConstArray:
		done = !varData.constArrayValue->size;
		return u(1);

	case variableType_e::Array:
	{
		u newData(varData.arrayValue->arrayValue);
		done = !newData.en.NextKey();
		return newData;
	}

	case variableType_e::String:
		done = !varData.stringValue->length();
		return u(0);
	case variableType_e::Vector:
		done = false;
		return u(0);

	default:
		throw ScriptVariableErrors::CastError(var.GetTypeName(), "array");
	}
}

bool ScriptVariableIterator::isValid() const
{
	return !done;
}

ScriptVariable ScriptVariableIterator::GetKey() const
{
	if (!isValid()) {
		return ScriptVariable();
	}

	switch (owningVar.GetType())
	{
	case variableType_e::Container:
	case variableType_e::SafeContainer:
	case variableType_e::ConstArray:
	case variableType_e::String:
	case variableType_e::Vector:
		return ScriptVariable(intptr_t(data.index));

	case variableType_e::Array:
		return *data.en.CurrentKey();

	default:
		return ScriptVariable();
	}
}

ScriptVariable ScriptVariableIterator::GetValue() const
{
	if (!isValid()) {
		return ScriptVariable();
	}

	const scriptData_u varData = owningVar.GetData();

	switch (owningVar.GetType())
	{
	case variableType_e::Container:
		return ScriptVariable(varData.containerValue->ObjectAt(data.index));
	case variableType_e::SafeContainer:
		if (!varData.safeContainerValue->Pointer()) {
			return ScriptVariable();
		}

		return ScriptVariable(varData.safeContainerValue->Pointer()->ObjectAt(data.index));
	case variableType_e::ConstArray:
		return ScriptVariable(varData.constArrayValue->constArrayValue[data.index]);

	case variableType_e::Array:
		return *data.en.CurrentValue();

	case variableType_e::String:
		return ScriptVariable(char((*varData.stringValue)[data.index]));
	case variableType_e::Vector:
		return ScriptVariable(float(varData.vectorValue[data.index]));

	default:
		return ScriptVariable();
	}
}

uintptr_t ScriptVariableIterator::Count() const
{
	return count;
}
