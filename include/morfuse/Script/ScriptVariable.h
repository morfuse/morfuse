#pragma once

#include "Class.h"
#include "ContainerClass.h"
#include "../Container/Container.h"
#include "../Container/set.h"
#include "../Common/str.h"
#include "../Common/Vector.h"
#include "../Common/SafePtr.h"

namespace mfuse
{
	class ScriptVariable;
	class Archiver;
	class Listener;
	class SimpleEntity;
	class Entity;
	class Vector;

	using ScriptVariableContainer = con::Container<ScriptVariable>;

	enum class variableType_e
	{
		/** A none variable. */
		None,
		/** Variable containing raw string value. */
		String,
		/** Integer value. */
		Integer,
		/** Float value. */
		Float,
		/** rawchar_t/byte value. */
		Char,
		/** An index to the string table. */
		ConstString,
		/** Points to another listener. */
		Listener,
		/** Reference another script variable. */
		Ref,
		/** Standard map array. */
		Array,
		/** Constant C-style array. */
		ConstArray,
		/** Array of listeners. */
		Container,
		/** Safe array of listeners. */
		SafeContainer,
		/** Pointer containing a list of script variable to set value on later. */
		Pointer,
		/** Vector value. */
		Vector,
		Max
	};

	constexpr variableType_e operator+(variableType_e lhs, variableType_e rhs)
	{
		return (variableType_e)((uintptr_t)lhs + (uintptr_t)rhs);
	}

	constexpr variableType_e operator*(variableType_e lhs, variableType_e rhs)
	{
		return (variableType_e)((uintptr_t)lhs * (uintptr_t)rhs);
	}

	static const rawchar_t *typenames[] =
	{
		"none",
		"string",
		"int",
		"float",
		"char",
		"const string",
		"listener",
		"ref",
		"array",
		"const array",
		"array",
		"array",
		"pointer",
		"vector"
	};

	class ScriptArrayHolder
	{
	public:
		ScriptArrayHolder();
		~ScriptArrayHolder();

		void			Archive(Archiver& arc);
		static void		Archive(Archiver& arc, ScriptArrayHolder*& arrayValue);

	public:
		con::map<ScriptVariable, ScriptVariable> arrayValue;
		unsigned int refCount;
	};

	class ScriptConstArrayHolder
	{
	public:
		ScriptVariable*  constArrayValue;
		unsigned int refCount;
		size_t size;

	public:
		void			Archive(Archiver& arc);
		static void		Archive(Archiver& arc, ScriptConstArrayHolder *& constArrayValue);

		ScriptConstArrayHolder(ScriptVariable* pVar, size_t size);
		ScriptConstArrayHolder(size_t size);
		ScriptConstArrayHolder();
		~ScriptConstArrayHolder();
	};

	/**
	 * The script pointer holds a list of variable to set value on at a later stage.
	 */
	class ScriptPointer
	{
	public:
		con::Container<ScriptVariable*> list;

	public:
		void			Archive(Archiver& arc);
		static void		Archive(Archiver& arc, ScriptPointer *& pointerValue);

		void		Clear();

		void		add(ScriptVariable* var);
		void		remove(ScriptVariable* var);
		void		setValue(const ScriptVariable& var);
	};
	using ConList = con::ContainerClass<SafePtr<Listener>>;
	using ConListPtr = SafePtr<ConList>;
	
	union mfuse_EXPORTS scriptData_u
	{
		rawchar_t charValue;
		float floatValue;
		uint32_t int32Value;
		int64_t long64Value;
		SafePtr<Listener>* listenerValue;
		const_str constStringValue;
		xstr* stringValue;
		float* vectorValue;
		ScriptVariable* refValue;
		ScriptArrayHolder* arrayValue;
		ScriptConstArrayHolder* constArrayValue;
		const con::Container<SafePtr<Listener>>* containerValue;
		const ConListPtr* safeContainerValue;
		ScriptPointer* pointerValue;
		ScriptVariable* otherVarValue;

		scriptData_u();
		scriptData_u(const scriptData_u& other);
		scriptData_u& operator=(const scriptData_u& other);
	};

	class mfuse_EXPORTS ScriptVariable
	{
	friend class ScriptPointer;
	public:
		ScriptVariable();
		ScriptVariable(const_str keyValue);
		ScriptVariable(const_str keyValue, const ScriptVariable& variable);
		ScriptVariable(const ScriptVariable& variable);
		ScriptVariable(ScriptVariable&& variable);

		ScriptVariable(int32_t initialValue);
		ScriptVariable(int64_t initialValue);
		ScriptVariable(float initialValue);
		ScriptVariable(char initialValue);
		ScriptVariable(Listener* initialValue);
		ScriptVariable(const rawchar_t* initialValue);
		ScriptVariable(const xstr& initialValue);
		ScriptVariable(const con::Container<SafePtr<Listener>>* initialValue);
		ScriptVariable(const Vector& initialValue);

		~ScriptVariable();

		void Archive(Archiver& arc);
		static void				Archive(Archiver& arc, ScriptVariable** obj);
		void ArchiveInternal(Archiver& arc);

		void CastBoolean();
		void CastConstArrayValue();
		void CastFloat();
		void CastInteger();
		void CastString();

		void Clear();
		void ClearPointer();

		const rawchar_t* GetTypeName() const;
		variableType_e			GetType() const;
		scriptData_u& GetData();
		scriptData_u			GetData() const;

		bool IsEntity() const;
		bool IsListener() const;
		bool IsConstArray() const;
		bool IsNumeric() const;
		bool IsSimpleEntity() const;
		bool IsString() const;
		bool IsVector() const;
		bool IsImmediate() const;
		bool IsNone() const;

		void PrintValue();

		void SetFalse();
		void SetTrue();

		size_t arraysize() const;
		size_t size() const;

		bool booleanNumericValue();
		bool booleanValue() const;

		const xstr& getName();

		const_str GetKey();
		void SetKey(const_str key);

		void evalArrayAt(ScriptVariable& var);

		void setArrayAt(const ScriptVariable& index, const ScriptVariable& value);
		void setArrayAtRef(const ScriptVariable& index, const ScriptVariable& value);
		void setArrayRefValue(ScriptVariable& var);

		rawchar_t charValue() const;
		void setCharValue(rawchar_t newvalue);

		ScriptVariable* constArrayValue();
		void setConstArrayValue(ScriptVariable* pVar, unsigned int size);

		const_str constStringValue() const;
		void setConstStringValue(const_str s);

		void setContainerValue(const con::Container<SafePtr<Listener>>* newvalue);
		void setSafeContainerValue(con::ContainerClass<SafePtr<Listener>>* newvalue);

		float floatValue() const;
		void setFloatValue(float newvalue);

		uint32_t intValue() const;
		void setIntValue(uint32_t newvalue);

		uint64_t longValue() const;
		void setLongValue(uint64_t newvalue);

		Listener* listenerValue() const;
		Listener* listenerAt(uintptr_t index) const;
		void setListenerValue(Listener* newvalue);

		void newPointer();
		void setPointer(const ScriptVariable& newvalue);

		void setRefValue(ScriptVariable* ref);

		xstr stringValue() const;
		void setStringValue(const xstr& newvalue);
		void setStringValue(const rawchar_t* newvalue);

		Vector vectorValue() const;
		void setVectorValue(const Vector& newvector);

		void greaterthan(const ScriptVariable& variable);
		void greaterthanorequal(const ScriptVariable& variable);
		void lessthan(const ScriptVariable& variable);
		void lessthanorequal(const ScriptVariable& variable);

		void complement();
		void minus();
		ScriptVariable& constArrayElement(uintptr_t index) const;
		void setDataInternal(const ScriptVariable& other);
		ScriptVariable& operator=(const ScriptVariable& variable);
		ScriptVariable& operator=(ScriptVariable&& variable);
		ScriptVariable& operator[](ScriptVariable& index);
		ScriptVariable* operator[](uintptr_t index) const;
		ScriptVariable* operator*();
		void operator+=(const ScriptVariable& value);
		void operator-=(const ScriptVariable& value);
		void operator*=(const ScriptVariable& value);
		void operator/=(const ScriptVariable& value);
		void operator%=(const ScriptVariable& value);
		void operator&=(const ScriptVariable& value);
		void operator^=(const ScriptVariable& value);
		void operator|=(const ScriptVariable& value);
		void operator<<=(const ScriptVariable& value);
		void operator>>=(const ScriptVariable& value);

		bool operator!=(const ScriptVariable& value) const;
		bool operator==(const ScriptVariable& value) const;

		ScriptVariable& operator++(int);
		ScriptVariable& operator--(int);

	private:
		void ClearInternal();
		void ClearPointerInternal();

	private:
		scriptData_u m_data;
		const_str key;
		variableType_e type;
	};

	class ScriptVariableList : public Class
	{
	public:
		ScriptVariableList();
		~ScriptVariableList();

		void Archive(Archiver &arc) override;

		void ClearList();

		ScriptVariable* GetOrCreateVariable(const xstr& name);
		ScriptVariable* GetOrCreateVariable(const_str name);

		ScriptVariable* GetVariable(const xstr& name);
		ScriptVariable* GetVariable(const_str name);

		ScriptVariable* SetVariable(const rawchar_t *name, int value);
		ScriptVariable* SetVariable(const rawchar_t *name, float value);
		ScriptVariable* SetVariable(const rawchar_t *name, const rawchar_t *value);
		ScriptVariable* SetVariable(const rawchar_t *name, Entity *value);
		ScriptVariable* SetVariable(const rawchar_t *name, Listener *value);
		ScriptVariable* SetVariable(const rawchar_t *name, Vector &value);
		ScriptVariable* SetVariable(const rawchar_t *name, const ScriptVariable& value);
		ScriptVariable* SetVariable(const_str name, const ScriptVariable& value);
		ScriptVariable* SetVariable(const_str name, ScriptVariable&& value);

	private:
		con::set<const_str, ScriptVariable> list;
	};
};
