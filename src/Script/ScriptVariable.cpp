#include <morfuse/Script/ScriptVariable.h>
#include <morfuse/Script/ScriptException.h>
#include <morfuse/Script/Listener.h>
#include <morfuse/Script/SimpleEntity.h>
#include <morfuse/Script/ConstStr.h>
#include <morfuse/Script/Archiver.h>
#include <morfuse/Script/Context.h>
#include <morfuse/Script/ScriptMaster.h>

using namespace mfuse;

template<>
class con::Entry<const_str, ScriptVariable>
{
public:
	Entry(const_str key)
		: value(key)
	{
	}

	Entry(const_str key, const ScriptVariable& varValue)
		: value(key, varValue)
	{
	}

	const_str Key() { return value.GetKey(); }
	ScriptVariable& Value() { return value; }
	Entry* Next() const { return next; }
	void SetNext(Entry* nextValue) { next = nextValue; }

private:
	ScriptVariable value;
	Entry* next;
};

template<>
intptr_t Hash<ScriptVariable>::operator()(const ScriptVariable& key) const
{
	Listener *l;

	switch (key.GetType())
	{
	case variableType_e::String:
	case variableType_e::ConstString:
		return Hash<xstr>()(key.stringValue());

	case variableType_e::Integer:
		return key.GetData().long64Value;

	case variableType_e::Listener:
		l = key.listenerValue();
		return (intptr_t)l;

	default:
		ScriptError("Bad hash code value: %s", key.stringValue().c_str());
	}
}

ScriptArrayHolder::ScriptArrayHolder()
{

}

ScriptArrayHolder::~ScriptArrayHolder()
{

}

void ScriptArrayHolder::Archive(Archiver& arc)
{
	/*
	arc.ArchiveUnsigned(&refCount);
	arrayValue.Archive(arc);
	*/
}

void ScriptArrayHolder::Archive(Archiver& arc, ScriptArrayHolder *& arrayValue)
{
	/*
	bool newRef;

	if (arc.Loading())
	{
		arc.ArchiveBoolean(&newRef);
	}
	else
	{
		newRef = !arc.ObjectPositionExists(arrayValue);
		arc.ArchiveBoolean(&newRef);
	}

	if (newRef)
	{
		if (arc.Loading())
		{
			arrayValue = GetAssetManager()->NewObject<ScriptArrayHolder>();
		}

		arc.ArchiveObjectPosition(arrayValue);
		arrayValue->Archive(arc);
		return;
	}
	else
	{
		arc.ArchiveObjectPointer((Class **)&arrayValue);
	}
	*/
}

void ScriptConstArrayHolder::Archive(Archiver& arc)
{
	/*
	arc.ArchiveUnsigned(&refCount);
	arc.ArchiveUnsigned(&size);

	if (arc.Loading())
	{
		constArrayValue = new ScriptVariable[size + 1] - 1;
	}

	for (int i = 1; i <= size; i++)
	{
		constArrayValue[i].ArchiveInternal(arc);
	}
	*/
}

void ScriptConstArrayHolder::Archive(Archiver& arc, ScriptConstArrayHolder *& constArrayValue)
{
	/*
	bool newRef;

	if (arc.Loading())
	{
		arc.ArchiveBoolean(&newRef);
	}
	else
	{
		newRef = !arc.ObjectPositionExists(constArrayValue);
		arc.ArchiveBoolean(&newRef);
	}

	if (newRef)
	{
		if (arc.Loading())
		{
			constArrayValue = GetAssetManager()->NewObject<ScriptConstArrayHolder>();
		}

		arc.ArchiveObjectPosition(constArrayValue);
		constArrayValue->Archive(arc);
		return;
	}
	else
	{
		arc.ArchiveObjectPointer((Class **)&constArrayValue);
	}
	*/
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


void ScriptPointer::Archive(Archiver& arc)
{
	//list.Archive(arc, ScriptVariable::Archive);
}

void ScriptPointer::Archive(Archiver& arc, ScriptPointer *& pointerValue)
{
	/*
	bool newRef;

	if (arc.Loading())
	{
		arc.ArchiveBoolean(&newRef);
	}
	else
	{
		newRef = !arc.ObjectPositionExists(pointerValue);
		arc.ArchiveBoolean(&newRef);
	}

	if (newRef)
	{
		if (arc.Loading())
		{
			pointerValue = GetAssetManager()->NewObject<ScriptPointer>();
		}

		arc.ArchiveObjectPosition(pointerValue);
		pointerValue->Archive(arc);
		return;
	}
	else
	{
		arc.ArchiveObjectPointer((Class **)&pointerValue);
	}
	*/
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

	if (!list.NumObjects())
	{
		delete this;
	}
}

void ScriptPointer::setValue(const ScriptVariable& var)
{
	for (uintptr_t i = list.NumObjects(); i > 0; i--)
	{
		ScriptVariable* pVar = list.ObjectAt(i);

		pVar->type = variableType_e::None;
		*pVar = var;
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

ScriptVariable::ScriptVariable(const_str keyValue)
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
	: key(0)
	, type(variable.type)
	, m_data(variable.m_data)
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
	m_data.stringValue = new xstr(initialValue);
}

ScriptVariable::ScriptVariable(const xstr& initialValue)
	: key(0)
	, type(variableType_e::String)
{
	m_data.stringValue = new xstr(initialValue);
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
	switch(type + variable.type * variableType_e::Max)
	{
	case variableType_e::None + variableType_e::None * variableType_e::Max:
	case variableType_e::None + variableType_e::Integer * variableType_e::Max:
	case variableType_e::None + variableType_e::Float * variableType_e::Max:
	case variableType_e::None + variableType_e::Char * variableType_e::Max:
	case variableType_e::None + variableType_e::ConstString * variableType_e::Max:
	case variableType_e::String + variableType_e::None * variableType_e::Max:
	case variableType_e::String + variableType_e::Integer * variableType_e::Max:
	case variableType_e::String + variableType_e::Float * variableType_e::Max:
	case variableType_e::String + variableType_e::Char * variableType_e::Max:
	case variableType_e::String + variableType_e::ConstString * variableType_e::Max:
	case variableType_e::Integer + variableType_e::None * variableType_e::Max:
	case variableType_e::Integer + variableType_e::Integer * variableType_e::Max:
	case variableType_e::Integer + variableType_e::Float * variableType_e::Max:
	case variableType_e::Integer + variableType_e::Char * variableType_e::Max:
	case variableType_e::Integer + variableType_e::ConstString * variableType_e::Max:
	case variableType_e::Float + variableType_e::None * variableType_e::Max:
	case variableType_e::Float + variableType_e::Integer * variableType_e::Max:
	case variableType_e::Float + variableType_e::Float * variableType_e::Max:
	case variableType_e::Float + variableType_e::Char * variableType_e::Max:
	case variableType_e::Float + variableType_e::ConstString * variableType_e::Max:
	case variableType_e::Char + variableType_e::None * variableType_e::Max:
	case variableType_e::Char + variableType_e::Integer * variableType_e::Max:
	case variableType_e::Char + variableType_e::Float * variableType_e::Max:
	case variableType_e::Char + variableType_e::Char * variableType_e::Max:
	case variableType_e::Char + variableType_e::ConstString * variableType_e::Max:
	case variableType_e::ConstString + variableType_e::None * variableType_e::Max:
	case variableType_e::ConstString + variableType_e::Integer * variableType_e::Max:
	case variableType_e::ConstString + variableType_e::Float * variableType_e::Max:
	case variableType_e::ConstString + variableType_e::Char * variableType_e::Max:
	case variableType_e::ConstString + variableType_e::ConstString * variableType_e::Max:
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
	return *this;
}

ScriptVariable::~ScriptVariable()
{
	ClearInternal();
}

void ScriptVariable::Archive(Archiver& arc)
{
	/*
	const_str s;

	if (arc.Loading())
	{
		ScriptContext::Get().GetDirector().ArchiveString(arc, s);
		key = s;
	}
	else
	{
		s = key;
		ScriptContext::Get().GetDirector().ArchiveString(arc, s);
	}
	*/

	ArchiveInternal(arc);
}

void ScriptVariable::Archive(Archiver& arc, ScriptVariable **obj)
{
	//arc.ArchiveObjectPointer((Class **)obj);
}

void ScriptVariable::ArchiveInternal(Archiver& arc)
{
	/*
	arc.ArchiveObjectPosition(this);

	arc.ArchiveByte(&type);
	switch (type)
	{
	case variableType_e::String:
		if (arc.Loading())
		{
			m_data.stringValue = new xstr(4);
		}

		arc.ArchiveString(m_data.stringValue);
		break;

	case variableType_e::Integer:
		arc.ArchiveInteger(&m_data.long64Value);
		break;

	case variableType_e::Float:
		arc.ArchiveFloat(&m_data.floatValue);
		break;

	case variableType_e::Char:
		arc.ArchiveChar(&m_data.charValue);
		break;

	case variableType_e::ConstString:
		if (arc.Loading())
		{
			xstr s;
			arc.ArchiveString(&s);
			m_data.long64Value = ScriptContext::Get().GetDirector().AddString(s);
		}
		else
		{
			xstr s = ScriptContext::Get().GetDirector().GetString(m_data.long64Value);
			arc.ArchiveString(&s);
		}
		break;

	case variableType_e::Listener:
		if (arc.Loading())
		{
			m_data.listenerValue = new ListenerPtr;
		}

		arc.ArchiveSafePointer(m_data.listenerValue);
		break;

	case variableType_e::Array:
		ScriptArrayHolder::Archive(arc, m_data.arrayValue);
		break;

	case variableType_e::ConstArray:
		ScriptConstArrayHolder::Archive(arc, m_data.constArrayValue);
		break;

	case variableType_e::Ref:
	case variableType_e::Container:
		arc.ArchiveObjectPointer((Class **)&m_data.refValue);
		break;

	case variableType_e::SafeContainer:
		if (arc.Loading())
		{
			m_data.safeContainerValue = new ListenerPtr;
		}

		arc.ArchiveSafePointer(m_data.safeContainerValue);
		break;

	case variableType_e::Pointer:
		ScriptPointer::Archive(arc, m_data.pointerValue);
		break;

	case variableType_e::Vector:
		if (arc.Loading())
		{
			m_data.vectorValue = new float[3];
		}

		arc.ArchiveVec3(m_data.vectorValue);
		break;

	default:
		break;
	}
	*/
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
		ScriptError("cannot cast NIL to an array");

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
	case variableType_e::SafeContainer:
		if (m_data.listenerValue)
		{
			delete m_data.listenerValue;
			m_data.listenerValue = nullptr;
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

void ScriptVariable::ClearPointerInternal()
{
	type = variableType_e::None;

	m_data.pointerValue->Clear();
	m_data.pointerValue = nullptr;
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
	switch (type)
	{
	case variableType_e::None:
		printf("");
		break;

	case variableType_e::ConstString:
		printf("%s", ScriptContext::Get().GetDirector().GetString(m_data.constStringValue).c_str());
		break;

	case variableType_e::String:
		printf("%s", m_data.stringValue->c_str());
		break;

	case variableType_e::Integer:
		printf("%lld", m_data.long64Value);
		break;

	case variableType_e::Float:
		printf("%f", m_data.floatValue);
		break;

	case variableType_e::Char:
		printf("%c", m_data.charValue);
		break;

	case variableType_e::Listener:
		printf("<Listener>%p", m_data.listenerValue->Pointer());
		break;

	case variableType_e::Ref:
	case variableType_e::Array:
	case variableType_e::ConstArray:
	case variableType_e::Container:
	case variableType_e::SafeContainer:
	case variableType_e::Pointer:
		printf("type: %s", GetTypeName());
		break;

	case variableType_e::Vector:
		printf("( %f %f %f )", m_data.vectorValue[0], m_data.vectorValue[1], m_data.vectorValue[2]);
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
	case variableType_e::String:
	case variableType_e::Integer:
	case variableType_e::Float:
	case variableType_e::Char:
	case variableType_e::ConstString:
	case variableType_e::Listener:
	case variableType_e::Ref:
	case variableType_e::Vector:
		return 1;

	case variableType_e::None:
		return -1;

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
		m_data.pointerValue->Clear();
		delete m_data.pointerValue;

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
	case variableType_e::Pointer:
		m_data.pointerValue->Clear();
		delete m_data.pointerValue;

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

	default:
		return 1;
	}

	return 0;
}

bool ScriptVariable::booleanNumericValue()
{
	xstr value;

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
		ScriptError("Cannot cast '%s' to boolean numeric", GetTypeName());
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
		return m_data.long64Value != STRING_EMPTY;

	case variableType_e::Listener:
		return (*m_data.listenerValue) != nullptr;

	default:
		return true;
	}
}

rawchar_t ScriptVariable::charValue() const
{
	xstr value;

	switch (type)
	{
	case variableType_e::Char:
		return m_data.charValue;

	case variableType_e::ConstString:
	case variableType_e::String:
		value = stringValue();

		if (value.length() != 1)
		{
			ScriptError("Cannot cast string not of length 1 to rawchar_t");
		}

		return *value;

	default:
		ScriptError("Cannot cast '%s' to rawchar_t", GetTypeName());
	}

	return 0;
}

ScriptVariable *ScriptVariable::constArrayValue()
{
	return m_data.constArrayValue->constArrayValue;
}

#ifndef NO_SCRIPTENGINE

xstr getname_null = "";

const xstr& ScriptVariable::getName()
{
	return ScriptContext::Get().GetDirector().GetString(GetKey());
}

const_str ScriptVariable::GetKey()
{
	return key;
}

void ScriptVariable::SetKey(const_str key)
{
	this->key = key;
}

#endif

void ScriptVariable::evalArrayAt(ScriptVariable &var)
{
	size_t index;
	xstr string;
	ScriptVariable *array;

	switch (type)
	{
	case variableType_e::Vector:
		index = var.intValue();

		if (index > 2)
		{
			Clear();
			ScriptError("Vector index '%d' out of range", index);
		}

		return setFloatValue(m_data.vectorValue[index]);

	case variableType_e::None:
		break;

	case variableType_e::ConstString:
	case variableType_e::String:
		index = var.intValue();
		string = stringValue();

		if (index >= string.length())
		{
			Clear();
			ScriptError("String index %d out of range", index);
		}

		return setCharValue(string[index]);

	case variableType_e::Listener:
		index = var.intValue();

		if (index != 1)
		{
			Clear();
			ScriptError("array index %d out of range", index);
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
		index = var.intValue();

		if (!index || index > m_data.constArrayValue->size)
		{
			ScriptError("array index %d out of range", index);
		}

		*this = m_data.constArrayValue->constArrayValue[index];
		break;

	case variableType_e::Container:
		index = var.intValue();

		if (!index || index > m_data.constArrayValue->size)
		{
			ScriptError("array index %d out of range", index);
		}

		setListenerValue(m_data.containerValue->ObjectAt(index));
		break;

	case variableType_e::SafeContainer:
		index = var.intValue();

		if (!*m_data.safeContainerValue || !index || index > m_data.constArrayValue->size) {
			ScriptError("array index %d out of range", index);
		}

		setListenerValue((*m_data.safeContainerValue)->ObjectAt(index));
		break;

	default:
		Clear();
		ScriptError("[] applied to invalid type '%s'", GetTypeName());
		break;
	}
}

float ScriptVariable::floatValue() const
{
	const rawchar_t *string;
	float val;

	switch (type)
	{
	case variableType_e::Float:
		return m_data.floatValue;

	case variableType_e::Integer:
		return (float)m_data.long64Value;

		/* Transform the string into an integer if possible */
	case variableType_e::String:
	case variableType_e::ConstString:
		string = stringValue();
		val = (float)atof((const rawchar_t *)string);

		return val;

	default:
		ScriptError("Cannot cast '%s' to float", GetTypeName());
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
		xstr string = stringValue();
		uint32_t val = atoi(string);

		return val;
	}

	default:
		ScriptError("Cannot cast '%s' to int", GetTypeName());
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
		xstr string = stringValue();
		uint64_t val = atoll(string);

		return val;
	}

	default:
		ScriptError("Cannot cast '%s' to int64", GetTypeName());
	}
}

Listener *ScriptVariable::listenerValue() const
{
	switch (type)
	{
	case variableType_e::ConstString:
		return ScriptContext::Get().GetTargetList().GetScriptTarget(ScriptContext::Get().GetDirector().GetString(m_data.constStringValue));

	case variableType_e::String:
		return ScriptContext::Get().GetTargetList().GetScriptTarget(stringValue());

	case variableType_e::Listener:
		return (Listener *)m_data.listenerValue->Pointer();

	default:
		ScriptError("Cannot cast '%s' to listener", GetTypeName());
	}

	return nullptr;
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
		ScriptError("Cannot cast '%s' to listener", GetTypeName());
	}
}

void ScriptVariable::newPointer()
{
	type = variableType_e::Pointer;

	m_data.pointerValue = new ScriptPointer();
	m_data.pointerValue->add(this);
}

xstr ScriptVariable::stringValue() const
{
	xstr string;

	switch (type)
	{
	case variableType_e::None:
		return "NIL";

	case variableType_e::ConstString:
		return ScriptContext::Get().GetDirector().GetString(m_data.constStringValue);

	case variableType_e::String:
		return *m_data.stringValue;

	case variableType_e::Integer:
		return xstr(m_data.long64Value);

	case variableType_e::Float:
		return xstr(m_data.floatValue);

	case variableType_e::Char:
		return xstr(m_data.charValue);

	case variableType_e::Listener:
		if (m_data.listenerValue->Pointer())
		{
			/*
			if (m_data.listenerValue->Pointer()->isSubclassOf(SimpleEntity))
			{
				SimpleEntity *s = (SimpleEntity *)m_data.listenerValue->Pointer();
				return s->targetname;
			}
			else
			*/
			{
				string = "class '" + xstr(m_data.listenerValue->Pointer()->Class::GetClassname()) + "'";
				return string;
			}
		}
		else
		{
			return "nullptr";
		}

	case variableType_e::Vector:
		return xstr("( ") + xstr(m_data.vectorValue[0]) + xstr(" ") + xstr(m_data.vectorValue[1]) + xstr(" ") + xstr(m_data.vectorValue[2]) + xstr(" )");

	default:
		return "Type: '" + xstr(GetTypeName()) + "'";
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
		string = stringValue();

		if (strcmp(string, "") == 0) {
			ScriptError("cannot cast empty string to vector");
		}

		if (*string == '(')
		{
			if (sscanf(string, "(%f %f %f)", &x, &y, &z) != 3)
			{
				if (sscanf(string, "(%f, %f, %f)", &x, &y, &z) != 3) {
					ScriptError("Couldn't convert string to vector - malformed string '%s'", string);
				}
			}
		}
		else
		{
			if (sscanf(string, "%f %f %f", &x, &y, &z) != 3)
			{
				if (sscanf(string, "%f, %f, %f", &x, &y, &z) != 3) {
					ScriptError("Couldn't convert string to vector - malformed string '%s'", string);
				}
			}
		}

		return Vector(x, y, z);
	case variableType_e::Listener:
	{
		if (!m_data.listenerValue->Pointer()) {
			ScriptError("Cannot cast nullptr to vector");
		}

		if (!ClassDef::checkInheritance(SimpleEntity::staticclass(), m_data.listenerValue->Pointer()->classinfo()))
		{
			ScriptError("Cannot cast '%s' to vector", GetTypeName());
		}

		SimpleEntity *ent = (SimpleEntity *)m_data.listenerValue->Pointer();

		const Vector& origin = ent->getOrigin();
		return Vector(origin[0], origin[1], origin[2]);
	}

	default:
		ScriptError("Cannot cast '%s' to vector", GetTypeName());
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
	xstr string;

	switch (type)
	{
	case variableType_e::Vector:
		intValue = index.intValue();

		if (intValue > 2) {
			ScriptError("Vector index '%d' out of range", intValue);
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

		if (intValue >= (intptr_t)strlen(string)) {
			ScriptError("String index '%d' out of range", intValue);
		}

		string[intValue] = value.charValue();

		setStringValue(string);

		break;

	case variableType_e::ConstArray:
		uintValue = index.intValue();

		if (!uintValue || uintValue > m_data.constArrayValue->size)
		{
			ScriptError("array index %d out of range", uintValue);
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
		ScriptError("[] applied to invalid type '%s'\n", GetTypeName());
		break;
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

void ScriptVariable::setConstArrayValue(ScriptVariable *pVar, unsigned int size)
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
		return ScriptContext::Get().GetDirector().AddString(stringValue());
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

void ScriptVariable::setRefValue(ScriptVariable* ref)
{
	ClearInternal();

	type = variableType_e::Ref;
	m_data.refValue = ref;
}

void ScriptVariable::setStringValue(const xstr& newvalue)
{
	ClearInternal();
	type = variableType_e::String;
	m_data.stringValue = new xstr(newvalue);
	StringConvert(*m_data.stringValue, newvalue.c_str());
}

void ScriptVariable::setStringValue(const rawchar_t* newvalue)
{
	ClearInternal();
	type = variableType_e::String;
	m_data.stringValue = new xstr(newvalue);
	StringConvert(*m_data.stringValue, newvalue);
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
	switch (type + value.type * variableType_e::Max)
	{
	default:
		Clear();

		ScriptError("binary '+' applied to incompatible types '%s' and '%s'", GetTypeName(), value.GetTypeName());

		break;

	case variableType_e::Integer + variableType_e::Integer * variableType_e::Max: // ( int ) + ( int )
		m_data.long64Value = m_data.long64Value + value.m_data.long64Value;
		break;

	case variableType_e::Integer + variableType_e::Float * variableType_e::Max: // ( int ) + ( float )
		setFloatValue((float)m_data.long64Value + value.m_data.floatValue);
		break;

	case variableType_e::Float + variableType_e::Float * variableType_e::Max: // ( float ) + ( float )
		m_data.floatValue = m_data.floatValue + value.m_data.floatValue;
		break;

	case variableType_e::Float + variableType_e::Integer * variableType_e::Max: // ( float ) + ( int )
		m_data.floatValue = m_data.floatValue + value.m_data.long64Value;
		break;

	case variableType_e::String + variableType_e::String * variableType_e::Max:				// ( string )			+		( string )
	case variableType_e::Integer + variableType_e::String * variableType_e::Max:			// ( int )				+		( string )
	case variableType_e::Float + variableType_e::String * variableType_e::Max:				// ( float )			+		( string )
	case variableType_e::Char + variableType_e::String * variableType_e::Max:				// ( rawchar_t )		+		( string )
	case variableType_e::ConstString + variableType_e::String * variableType_e::Max:		// ( const string )		+		( string )
	case variableType_e::Listener + variableType_e::String * variableType_e::Max:			// ( listener )			+		( string )
	case variableType_e::Vector + variableType_e::String * variableType_e::Max:				// ( vector )			+		( string )
	case variableType_e::String + variableType_e::Integer * variableType_e::Max:			// ( string )			+		( int )
	case variableType_e::ConstString + variableType_e::Integer * variableType_e::Max:		// ( const string )		+		( int )
	case variableType_e::String + variableType_e::Float * variableType_e::Max:				// ( string )			+		( float )
	case variableType_e::ConstString + variableType_e::Float * variableType_e::Max:			// ( const string )		+		( float )
	case variableType_e::String + variableType_e::Char * variableType_e::Max:				// ( string )			+		( rawchar_t )
	case variableType_e::ConstString + variableType_e::Char * variableType_e::Max:			// ( const string )		+		( rawchar_t )
	case variableType_e::String + variableType_e::ConstString * variableType_e::Max:		// ( string )			+		( const string )
	case variableType_e::Integer + variableType_e::ConstString * variableType_e::Max:		// ( int )				+		( const string )
	case variableType_e::Float + variableType_e::ConstString * variableType_e::Max:			// ( float )			+		( const string )
	case variableType_e::Char + variableType_e::ConstString * variableType_e::Max:			// ( rawchar_t )		+		( const string )
	case variableType_e::ConstString + variableType_e::ConstString * variableType_e::Max:	// ( const string )		+		( const string )
	case variableType_e::Listener + variableType_e::ConstString * variableType_e::Max:		// ( listener )			+		( const string )
	case variableType_e::Vector + variableType_e::ConstString * variableType_e::Max:		// ( vector )			+		( const string )
	case variableType_e::String + variableType_e::Listener * variableType_e::Max:			// ( string )			+		( listener )
	case variableType_e::ConstString + variableType_e::Listener * variableType_e::Max:		// ( const string )		+		( listener )
	case variableType_e::String + variableType_e::Vector * variableType_e::Max:				// ( string )			+		( vector )
	case variableType_e::ConstString + variableType_e::Vector * variableType_e::Max:		// ( const string )		+		( vector )
		setStringValue(stringValue() + value.stringValue());
		break;

	case variableType_e::Vector + variableType_e::Vector * variableType_e::Max:
		VecAdd(m_data.vectorValue, value.m_data.vectorValue, m_data.vectorValue);
		break;
	}
}

void ScriptVariable::operator-=(const ScriptVariable& value)
{
	switch (type + value.type * variableType_e::Max)
	{
	default:
		Clear();

		ScriptError("binary '-' applied to incompatible types '%s' and '%s'", GetTypeName(), value.GetTypeName());

		break;

	case variableType_e::Integer + variableType_e::Integer * variableType_e::Max: // ( int ) - ( int )
		m_data.long64Value = m_data.long64Value - value.m_data.long64Value;
		break;

	case variableType_e::Integer + variableType_e::Float * variableType_e::Max: // ( int ) - ( float )
		setFloatValue((float)m_data.long64Value - value.m_data.floatValue);
		break;

	case variableType_e::Float + variableType_e::Float * variableType_e::Max: // ( float ) - ( float )
		m_data.floatValue = m_data.floatValue - value.m_data.floatValue;
		break;

	case variableType_e::Float + variableType_e::Integer * variableType_e::Max: // ( float ) - ( int )
		m_data.floatValue = m_data.floatValue - value.m_data.long64Value;
		break;

	case variableType_e::Vector + variableType_e::Vector * variableType_e::Max: // ( vector ) - ( vector )
		VecSubtract(m_data.vectorValue, value.m_data.vectorValue, m_data.vectorValue);
		break;
	}
}

void ScriptVariable::operator*=(const ScriptVariable& value)
{
	switch (type + value.type * variableType_e::Max)
	{
	default:
		Clear();

		ScriptError("binary '*' applied to incompatible types '%s' and '%s'", GetTypeName(), value.GetTypeName());

		break;

	case variableType_e::Integer + variableType_e::Integer * variableType_e::Max: // ( int ) * ( int )
		m_data.long64Value = m_data.long64Value * value.m_data.long64Value;
		break;

	case variableType_e::Vector + variableType_e::Integer * variableType_e::Max: // ( vector ) * ( int )
		VectorScale(m_data.vectorValue, (float)value.m_data.long64Value, m_data.vectorValue);
		break;

	case variableType_e::Vector + variableType_e::Float * variableType_e::Max: // ( vector ) * ( float )
		VectorScale(m_data.vectorValue, value.m_data.floatValue, m_data.vectorValue);
		break;

	case variableType_e::Integer + variableType_e::Float * variableType_e::Max: // ( int ) * ( float )
		setFloatValue((float)m_data.long64Value * value.m_data.floatValue);
		break;

	case variableType_e::Float + variableType_e::Float * variableType_e::Max: // ( float ) * ( float )
		m_data.floatValue = m_data.floatValue * value.m_data.floatValue;
		break;

	case variableType_e::Float + variableType_e::Integer * variableType_e::Max: // ( float ) * ( int )
		m_data.floatValue = m_data.floatValue * value.m_data.long64Value;
		break;

	case variableType_e::Integer + variableType_e::Vector * variableType_e::Max: // ( int ) * ( vector )
		setVectorValue((float)m_data.long64Value * Vector(value.m_data.vectorValue));
		break;

	case variableType_e::Float + variableType_e::Vector * variableType_e::Max: // ( float ) * ( vector )
		setVectorValue(m_data.floatValue * Vector(value.m_data.vectorValue));
		break;

	case variableType_e::Vector + variableType_e::Vector * variableType_e::Max: // ( vector ) * ( vector )
		m_data.vectorValue[0] = m_data.vectorValue[0] * value.m_data.vectorValue[0];
		m_data.vectorValue[1] = m_data.vectorValue[1] * value.m_data.vectorValue[1];
		m_data.vectorValue[2] = m_data.vectorValue[2] * value.m_data.vectorValue[2];
		break;
	}
}

void ScriptVariable::operator/=(const ScriptVariable& value)
{
	switch (type + value.type * variableType_e::Max)
	{
	default:
		Clear();

		ScriptError("binary '/' applied to incompatible types '%s' and '%s'", GetTypeName(), value.GetTypeName());

		break;

	case variableType_e::Integer + variableType_e::Integer * variableType_e::Max: // ( int ) / ( int )
		if (value.m_data.long64Value == 0) {
			ScriptError("Division by zero error\n");
		}

		m_data.long64Value = m_data.long64Value / value.m_data.long64Value;
		break;

	case variableType_e::Vector + variableType_e::Integer * variableType_e::Max: // ( vector ) / ( int )
		if (value.m_data.long64Value == 0) {
			ScriptError("Division by zero error\n");
		}

		(Vector)m_data.vectorValue = (Vector)m_data.vectorValue / (float)value.m_data.long64Value;
		break;

	case variableType_e::Vector + variableType_e::Float * variableType_e::Max: // ( vector ) / ( float )
		if (value.m_data.floatValue == 0) {
			ScriptError("Division by zero error\n");
		}

		m_data.vectorValue[0] = m_data.vectorValue[0] / value.m_data.floatValue;
		m_data.vectorValue[1] = m_data.vectorValue[1] / value.m_data.floatValue;
		m_data.vectorValue[2] = m_data.vectorValue[2] / value.m_data.floatValue;
		break;

	case variableType_e::Integer + variableType_e::Float * variableType_e::Max: // ( int ) / ( float )
		if (value.m_data.floatValue == 0) {
			ScriptError("Division by zero error\n");
		}

		setFloatValue((float)m_data.long64Value / value.m_data.floatValue);
		break;

	case variableType_e::Float + variableType_e::Float * variableType_e::Max: // ( float ) / ( float )
		if (value.m_data.floatValue == 0) {
			ScriptError("Division by zero error\n");
		}

		m_data.floatValue = m_data.floatValue / value.m_data.floatValue;
		break;

	case variableType_e::Float + variableType_e::Integer * variableType_e::Max: // ( float ) / ( int )
		if (value.m_data.long64Value == 0) {
			ScriptError("Division by zero error\n");
		}

		m_data.floatValue = m_data.floatValue / value.m_data.long64Value;
		break;

	case variableType_e::Integer + variableType_e::Vector * variableType_e::Max: // ( int ) / ( vector )
		if (m_data.long64Value == 0) {
			ScriptError("Division by zero error\n");
		}

		setVectorValue((float)m_data.long64Value / Vector(value.m_data.vectorValue));
		break;

	case variableType_e::Float + variableType_e::Vector * variableType_e::Max: // ( float ) / ( vector )
		if (m_data.floatValue == 0) {
			ScriptError("Division by zero error\n");
		}

		setVectorValue(m_data.floatValue / Vector(value.m_data.vectorValue));
		break;

	case variableType_e::Vector + variableType_e::Vector * variableType_e::Max: // ( vector ) / ( vector )
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

	switch (type + value.type * variableType_e::Max)
	{
	default:
		Clear();

		ScriptError("binary '%' applied to incompatible types '%s' and '%s'", GetTypeName(), value.GetTypeName());

		break;

	case variableType_e::Integer + variableType_e::Integer * variableType_e::Max: // ( int ) % ( int )
		if (value.m_data.long64Value == 0) {
			ScriptError("Division by zero error\n");
		}

		m_data.long64Value = m_data.long64Value % value.m_data.long64Value;
		break;

	case variableType_e::Vector + variableType_e::Integer * variableType_e::Max: // ( vector ) % ( int )
		if (value.m_data.long64Value == 0) {
			ScriptError("Division by zero error\n");
		}

		m_data.vectorValue[0] = (float)fmodf(m_data.vectorValue[0], (float)value.m_data.long64Value);
		m_data.vectorValue[1] = (float)fmodf(m_data.vectorValue[1], (float)value.m_data.long64Value);
		m_data.vectorValue[2] = (float)fmodf(m_data.vectorValue[2], (float)value.m_data.long64Value);
		break;

	case variableType_e::Vector + variableType_e::Float * variableType_e::Max: // ( vector ) % ( float )
		if (value.m_data.floatValue == 0) {
			ScriptError("Division by zero error\n");
		}

		m_data.vectorValue[0] = (float)fmodf(m_data.vectorValue[0], value.m_data.floatValue);
		m_data.vectorValue[1] = (float)fmodf(m_data.vectorValue[1], value.m_data.floatValue);
		m_data.vectorValue[2] = (float)fmodf(m_data.vectorValue[2], value.m_data.floatValue);
		break;

	case variableType_e::Integer + variableType_e::Float * variableType_e::Max: // ( int ) % ( float )
		if (value.m_data.floatValue == 0) {
			ScriptError("Division by zero error\n");
		}

		setFloatValue(fmodf((float)m_data.long64Value, value.m_data.floatValue));
		break;

	case variableType_e::Float + variableType_e::Float * variableType_e::Max: // ( float ) % ( float )
		if (value.m_data.floatValue == 0) {
			ScriptError("Division by zero error\n");
		}

		m_data.floatValue = fmodf(m_data.floatValue, value.m_data.floatValue);
		break;

	case variableType_e::Float + variableType_e::Integer * variableType_e::Max: // ( float ) % ( int )
		if (value.m_data.long64Value == 0) {
			ScriptError("Division by zero error\n");
		}

		m_data.floatValue = fmodf(m_data.floatValue, (float)value.m_data.long64Value);
		break;

	case variableType_e::Integer + variableType_e::Vector * variableType_e::Max: // ( int ) % ( vector )
		mult = (float)m_data.long64Value;

		if (mult == 0) {
			ScriptError("Division by zero error\n");
		}

		setVectorValue(vec_zero);

		m_data.vectorValue[0] = fmodf(value.m_data.vectorValue[0], mult);
		m_data.vectorValue[1] = fmodf(value.m_data.vectorValue[1], mult);
		m_data.vectorValue[2] = fmodf(value.m_data.vectorValue[2], mult);
		break;

	case variableType_e::Float + variableType_e::Vector * variableType_e::Max: // ( float ) % ( vector )
		mult = m_data.floatValue;

		if (mult == 0) {
			ScriptError("Division by zero error\n");
		}

		setVectorValue(vec_zero);

		m_data.vectorValue[0] = fmodf(m_data.vectorValue[0], mult);
		m_data.vectorValue[1] = fmodf(m_data.vectorValue[1], mult);
		m_data.vectorValue[2] = fmodf(m_data.vectorValue[2], mult);
		break;

	case variableType_e::Vector + variableType_e::Vector * variableType_e::Max: // ( vector ) % ( vector )
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
	switch (type + value.type * variableType_e::Max)
	{
	default:
		Clear();

		ScriptError("binary '&' applied to incompatible types '%s' and '%s'", GetTypeName(), value.GetTypeName());

		break;

	case variableType_e::Integer + variableType_e::Integer * variableType_e::Max: // ( int ) &= ( int )
		m_data.long64Value &= value.m_data.long64Value;
		break;
	}
}

void ScriptVariable::operator^=(const ScriptVariable& value)
{
	switch (type + value.type * variableType_e::Max)
	{
	default:
		Clear();

		ScriptError("binary '^' applied to incompatible types '%s' and '%s'", GetTypeName(), value.GetTypeName());

		break;

	case variableType_e::Integer + variableType_e::Integer * variableType_e::Max: // ( int ) ^= ( int )
		m_data.long64Value ^= value.m_data.long64Value;
		break;
	}
}

void ScriptVariable::operator|=(const ScriptVariable& value)
{
	switch (type + value.type * variableType_e::Max)
	{
	default:
		Clear();

		ScriptError("binary '|' applied to incompatible types '%s' and '%s'", GetTypeName(), value.GetTypeName());

		break;

	case variableType_e::Integer + variableType_e::Integer * variableType_e::Max: // ( int ) |= ( int )
		m_data.long64Value |= value.m_data.long64Value;
		break;
	}
}

void ScriptVariable::operator<<=(const ScriptVariable& value)
{
	switch (type + value.type * variableType_e::Max)
	{
	default:
		Clear();

		ScriptError("binary '<<' applied to incompatible types '%s' and '%s'", GetTypeName(), value.GetTypeName());

		break;

	case variableType_e::Integer + variableType_e::Integer * variableType_e::Max: // ( int ) <<= ( int )
		m_data.long64Value <<= value.m_data.long64Value;
		break;
	}
}

void ScriptVariable::operator>>=(const ScriptVariable& value)
{
	switch (type + value.type * variableType_e::Max)
	{
	default:
		Clear();

		ScriptError("binary '>>' applied to incompatible types '%s' and '%s'", GetTypeName(), value.GetTypeName());

		break;

	// ( int ) >>= ( int )
	case variableType_e::Integer + variableType_e::Integer * variableType_e::Max: 
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
	switch (type + value.type * variableType_e::Max)
	{
	// ( lval )	==	( nil )
	// ( nil )	==	( rval )
	default:
		return false;

	// ( nil ) == ( nil )
	case variableType_e::None + variableType_e::None * variableType_e::Max:
		return true;

	// ( listener ) == ( listener )
	case variableType_e::Listener + variableType_e::Listener * variableType_e::Max:
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
	case variableType_e::Integer + variableType_e::Integer * variableType_e::Max:
		return m_data.long64Value == value.m_data.long64Value;

	// ( int ) == ( float )
	case variableType_e::Integer + variableType_e::Float * variableType_e::Max:
		return m_data.long64Value == value.m_data.floatValue;

	// ( float ) == ( float )
	case variableType_e::Float + variableType_e::Float * variableType_e::Max:
		return m_data.floatValue == value.m_data.floatValue;

	// ( float ) == ( int )
	case variableType_e::Float + variableType_e::Integer * variableType_e::Max:
		return m_data.floatValue == value.m_data.long64Value;

	case variableType_e::ConstString + variableType_e::ConstString * variableType_e::Max:	// ( const string )		==		( const string )
		return m_data.long64Value == value.m_data.long64Value;

	case variableType_e::String + variableType_e::String * variableType_e::Max:				// ( string )			==		( string )
	case variableType_e::Integer + variableType_e::String * variableType_e::Max:				// ( int )				==		( string )
	case variableType_e::Float + variableType_e::String * variableType_e::Max:				// ( float )			==		( string )
	case variableType_e::Char + variableType_e::String * variableType_e::Max:				// ( rawchar_t )				==		( string )
	case variableType_e::ConstString + variableType_e::String * variableType_e::Max:			// ( const string )		==		( string )
	case variableType_e::Listener + variableType_e::String * variableType_e::Max:			// ( listener )			==		( string )
	case variableType_e::Vector + variableType_e::String * variableType_e::Max:				// ( vector )			==		( string )
	case variableType_e::String + variableType_e::ConstString * variableType_e::Max:			// ( string )			==		( const string )
	case variableType_e::Integer + variableType_e::ConstString * variableType_e::Max:		// ( int )				==		( const string )
	case variableType_e::Float + variableType_e::ConstString * variableType_e::Max:			// ( float )			==		( const string )
	case variableType_e::Char + variableType_e::ConstString * variableType_e::Max:			// ( rawchar_t )				==		( const string )
	case variableType_e::Listener + variableType_e::ConstString * variableType_e::Max:		// ( listener )			==		( const string )
	case variableType_e::Vector + variableType_e::ConstString * variableType_e::Max:			// ( vector )			==		( const string )
	case variableType_e::String + variableType_e::Integer * variableType_e::Max:				// ( string )			==		( int )
	case variableType_e::ConstString + variableType_e::Integer * variableType_e::Max:		// ( const string )		==		( int )
	case variableType_e::String + variableType_e::Float * variableType_e::Max:				// ( string )			==		( float )
	case variableType_e::ConstString + variableType_e::Float * variableType_e::Max:			// ( const string )		==		( float )
	case variableType_e::String + variableType_e::Char * variableType_e::Max:				// ( string )			==		( rawchar_t )
	case variableType_e::ConstString + variableType_e::Char * variableType_e::Max:			// ( const string )		==		( rawchar_t )
	case variableType_e::String + variableType_e::Listener * variableType_e::Max:			// ( string )			==		( listener )
	case variableType_e::ConstString + variableType_e::Listener * variableType_e::Max:		// ( const string )		==		( listener )
	case variableType_e::String + variableType_e::Vector * variableType_e::Max:				// ( string )			==		( vector )
	case variableType_e::ConstString + variableType_e::Vector * variableType_e::Max:			// ( const string )		==		( vector )
	{
		xstr lval = stringValue();
		xstr rval = value.stringValue();

		return (!lval.length() && !rval.length()) || (lval == rval);
	}

	case variableType_e::Vector + variableType_e::Vector * variableType_e::Max: // ( vector ) == ( vector )
		return VecCompare(m_data.vectorValue, value.m_data.vectorValue) ? true : false;
	}
}

ScriptVariable &ScriptVariable::operator[](ScriptVariable& index)
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
			ScriptError("array index %d out of range", i);
		}

		return m_data.constArrayValue->constArrayValue[i];

	default:
		ScriptError("[] applied to invalid type '%s'", GetTypeName());
	}
}

ScriptVariable *ScriptVariable::operator[](uintptr_t index) const
{
	return &m_data.constArrayValue->constArrayValue[index];
}

ScriptVariable *ScriptVariable::operator*()
{
	return m_data.refValue;
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
	switch (type + variable.type * variableType_e::Max)
	{
	default:
		Clear();

		ScriptError("binary '>' applied to incompatible types '%s' and '%s'", GetTypeName(), variable.GetTypeName());

		break;

	// ( int ) <= ( int )
	case variableType_e::Integer + variableType_e::Integer * variableType_e::Max:
		m_data.long64Value = m_data.long64Value > variable.m_data.long64Value;
		break;

	// ( int ) <= ( float )
	case variableType_e::Integer + variableType_e::Float * variableType_e::Max:
		m_data.long64Value = m_data.long64Value > variable.m_data.floatValue;
		break;

	// ( float ) <= ( float )
	case variableType_e::Float + variableType_e::Float * variableType_e::Max:
		type = variableType_e::Integer;
		m_data.long64Value = m_data.floatValue > variable.m_data.floatValue;
		break;

	// ( float ) <= ( int )
	case variableType_e::Float + variableType_e::Integer * variableType_e::Max:
		type = variableType_e::Integer;
		m_data.long64Value = m_data.floatValue > variable.m_data.long64Value;
		break;
	}
}

void ScriptVariable::greaterthanorequal(const ScriptVariable &variable)
{
	switch (type + variable.type * variableType_e::Max)
	{
	default:
		Clear();

		ScriptError("binary '>=' applied to incompatible types '%s' and '%s'", GetTypeName(), variable.GetTypeName());

		break;

	// ( int ) <= ( int )
	case variableType_e::Integer + variableType_e::Integer * variableType_e::Max:
		m_data.long64Value = m_data.long64Value >= variable.m_data.long64Value;
		break;

	// ( int ) <= ( float )
	case variableType_e::Integer + variableType_e::Float * variableType_e::Max:
		m_data.long64Value = m_data.long64Value >= variable.m_data.floatValue;
		break;

	// ( float ) <= ( float )
	case variableType_e::Float + variableType_e::Float * variableType_e::Max:
		type = variableType_e::Integer;
		m_data.long64Value = m_data.floatValue >= variable.m_data.floatValue;
		break;

	// ( float ) <= ( int )
	case variableType_e::Float + variableType_e::Integer * variableType_e::Max:
		type = variableType_e::Integer;
		m_data.long64Value = m_data.floatValue >= variable.m_data.long64Value;
		break;
	}
}

void ScriptVariable::lessthan(const ScriptVariable &variable)
{
	switch (type + variable.type * variableType_e::Max)
	{
	default:
		Clear();
		ScriptError("binary '<' applied to incompatible types '%s' and '%s'", GetTypeName(), variable.GetTypeName());
		break;

	// ( int ) <= ( int )
	case variableType_e::Integer + variableType_e::Integer * variableType_e::Max:
		m_data.long64Value = m_data.long64Value < variable.m_data.long64Value;
		break;

	// ( int ) <= ( float )
	case variableType_e::Integer + variableType_e::Float * variableType_e::Max:
		m_data.long64Value = m_data.long64Value < variable.m_data.floatValue;
		break;

	// ( float ) <= ( float )
	case variableType_e::Float + variableType_e::Float * variableType_e::Max:
		type = variableType_e::Integer;
		m_data.long64Value = m_data.floatValue < variable.m_data.floatValue;
		break;

	// ( float ) <= ( int )
	case variableType_e::Float + variableType_e::Integer * variableType_e::Max:
		type = variableType_e::Integer;
		m_data.long64Value = m_data.floatValue < variable.m_data.long64Value;
		break;
	}
}

void ScriptVariable::lessthanorequal(const ScriptVariable &variable)
{
	switch (type + variable.type * variableType_e::Max)
	{
	default:
		Clear();

		ScriptError("binary '<=' applied to incompatible types '%s' and '%s'", GetTypeName(), variable.GetTypeName());

		break;

	// ( int ) <= ( int )
	case variableType_e::Integer + variableType_e::Integer * variableType_e::Max:
		m_data.long64Value = m_data.long64Value <= variable.m_data.long64Value;
		break;

	// ( int ) <= ( float )
	case variableType_e::Integer + variableType_e::Float * variableType_e::Max:
		m_data.long64Value = m_data.long64Value <= variable.m_data.floatValue;
		break;

	// ( float ) <= ( float )
	case variableType_e::Float + variableType_e::Float * variableType_e::Max:
		type = variableType_e::Integer;
		m_data.long64Value = m_data.floatValue <= variable.m_data.floatValue;
		break;

	// ( float ) <= ( int )
	case variableType_e::Float + variableType_e::Integer * variableType_e::Max:
		type = variableType_e::Integer;
		m_data.long64Value = m_data.floatValue <= variable.m_data.long64Value;
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
		m_data.stringValue = new xstr(*variable.m_data.stringValue);
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
		m_data.containerValue = new con::Container<SafePtr<Listener>>(*variable.m_data.containerValue);
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

ScriptVariable* ScriptVariableList::GetOrCreateVariable(const xstr& name)
{
	return GetOrCreateVariable(ScriptContext::Get().GetDirector().AddString(name));
}

ScriptVariable* ScriptVariableList::GetOrCreateVariable(const_str name)
{
	ScriptVariable& var = list.addKeyValue(name);
	assert(var.GetKey());
	return &var;
}

ScriptVariable *ScriptVariableList::GetVariable(const xstr& name)
{
	return GetVariable(ScriptContext::Get().GetDirector().AddString(name));
}

ScriptVariable *ScriptVariableList::GetVariable(const_str name)
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

void ScriptVariableList::Archive(Archiver &arc)
{
	//BaseScriptClass::Archive(arc);
	//list.Archive(arc);
}
