#pragma once

#include "../Global.h"
#include "../Common/str.h"
#include "../Container/Container.h"
#include "Class.h"
#include "ScriptException.h"

namespace mfuse
{
	class Listener;
	class ClassDef;

class SpawnArgs : public Class
{
	MFUS_CLASS_PROTOTYPE(SpawnArgs);

public:
	SpawnArgs();
	SpawnArgs(SpawnArgs& arglist);

	void Clear(void);

	const char* getArg(const char* key, const char* defaultValue = nullptr);
	void setArg(const strview& key, const strview& value);

	size_t NumArgs(void);
	const char* getKey(uintptr_t index);
	const char* getValue(uintptr_t index);
	void operator=(SpawnArgs& a);

	const ClassDef* getClassDef();
	Listener* Spawn(void);
	Listener* SpawnInternal(void);

	void Archive(Archiver& arc) override;

private:
	con::Container<str> keyList;
	con::Container<str> valueList;
};

namespace SpawnErrors
{
	class Base : public ScriptExceptionBase {};

	class NoSpawnFunction : public Base
	{
	public:
		const char* what() const noexcept override;
	};
}
}