#include <morfuse/Script/SpawnArgs.h>
#include <morfuse/Script/ScriptVariable.h>
#include <morfuse/Script/Listener.h>
#include <morfuse/Script/EventSystem.h>
#include <morfuse/Script/Context.h>

using namespace mfuse;

SpawnArgs::SpawnArgs()
{
}

SpawnArgs::SpawnArgs(SpawnArgs& otherlist)
{
	const size_t num = otherlist.NumArgs();
	keyList.Resize(num);
	valueList.Resize(num);
	for (uintptr_t i = 1; i <= num; i++)
	{
		keyList.AddObject(otherlist.keyList.ObjectAt(i));
		valueList.AddObject(otherlist.valueList.ObjectAt(i));
	}
}

void SpawnArgs::Clear(void)
{
	keyList.FreeObjectList();
	valueList.FreeObjectList();
}

const char* SpawnArgs::getArg(const char* key, const char* defaultValue)
{
	const size_t num = keyList.NumObjects();
	for (uintptr_t i = 1; i <= num; i++)
	{
		if (keyList.ObjectAt(i) == key)
		{
			return valueList.ObjectAt(i);
		}
	}

	return defaultValue;
}

void SpawnArgs::setArg(const char* key, const char* value)
{
	const size_t num = keyList.NumObjects();
	for (uintptr_t i = 1; i <= num; i++)
	{
		if (keyList.ObjectAt(i) == key)
		{
			valueList.ObjectAt(i) = value;
			return;
		}
	}

	keyList.AddObject(str(key));
	valueList.AddObject(str(value));
}

void SpawnArgs::operator=(SpawnArgs& otherlist)
{
	Clear();

	const size_t num = otherlist.NumArgs();
	keyList.Resize(num);
	valueList.Resize(num);

	for (uintptr_t i = 1; i <= num; i++)
	{
		keyList.AddObject(otherlist.keyList.ObjectAt(i));
		valueList.AddObject(otherlist.valueList.ObjectAt(i));
	}
}

size_t SpawnArgs::NumArgs(void)
{
	return keyList.NumObjects();
}

const char* SpawnArgs::getKey(uintptr_t index)
{
	return keyList.ObjectAt(index + 1);
}

const char* SpawnArgs::getValue(uintptr_t index)
{
	return valueList.ObjectAt(index + 1);
}

void SpawnArgs::Archive(Archiver& arc)
{
	Class::Archive(arc);

	//keyList.Archive( arc );
	//valueList.Archive( arc );
}

/*
===============
getClass

Finds the spawn function for the entity and returns ClassDef *
===============
*/

const ClassDef* SpawnArgs::getClassDef()
{
	const ClassDef* cls = nullptr;

	const rawchar_t* classname = getArg("classname");

	//
	// check normal spawn functions
	// see if the class name is stored within the model
	//
	if (classname)
	{
		cls = ClassDef::GetClassForID(classname);
		if (!cls)
		{
			cls = ClassDef::GetClass(classname);
		}
	}

	return cls;
}

Listener* SpawnArgs::Spawn(void)
{
	Listener* ent = (Listener*)SpawnInternal();

	if (ent)
	{
		ent->ProcessPendingEvents();
	}

	return ent;
}

Listener* SpawnArgs::SpawnInternal(void)
{
	const ClassDef* cls = getClassDef();

	if (!cls && !cls->inheritsFrom(Listener::staticclass()))
	{
		throw SpawnErrors::NoSpawnFunction();
	}

	const EventSystem& evt = EventSystem::Get();
	std::ostream* dbg = ScriptContext::Get().GetOutputInfo().GetOutput(outputLevel_e::Debug);

	Listener* const obj = dynamic_cast<Listener*>(cls->createInstance());

	// post spawnarg events
	for (uintptr_t i = 0; i < NumArgs(); i++)
	{
		const rawchar_t* key = getKey(i);
		const rawchar_t* value = getValue(i);

		if (*key == '#')
		{
			// don't count the prefix
			const rawchar_t* const keyname = (key + 1);
			// initialize the object vars
			ScriptVariableList* const varList = obj->Vars();

			const ScriptVariable* const var = varList->GetVariable(keyname);
			if (var)
			{
				if (dbg)
				{
					*dbg << "^~^~^ variable '"
						<< keyname
						<< "' already set with string value '"
						<< var->stringValue().c_str()
						<< "' - failed to attempt to set with numeric value '"
						<< value
						<< "'"
						<< std::endl;
				}

				continue;
			}

			const EventDef* def;
			const eventNum_t eventnum = evt.FindSetterEventNum(keyname);

			if (!eventnum || !(def = cls->GetDef(eventnum)))
			{
				if (strchr(keyname, '.'))
				{
					// it's a float
					varList->SetVariable(keyname, (float)atof(value));
				}
				else
				{
					varList->SetVariable(keyname, atoi(value));
				}
			}
			else if (def->GetAttributes().GetType() != evType_e::Setter)
			{
				if (dbg) {
					*dbg << "^~^~^ Cannot set a read-only variable '" << keyname << "'" << std::endl;
				}
			}
			else
			{
				Event* const ev = new Event(*def, 1);

				char* p = nullptr;
				if (str::findchar(keyname, '.'))
				{
					const float number = std::strtof(value, &p);
					ev->AddFloat(number);
				}
				else
				{
					const long int number = std::strtol(value, &p, 10);
					ev->AddInteger(number);
				}

				obj->PostEvent(ev, EV_SPAWNARG);
			}
		}
		else if (*key == '$')
		{
			const rawchar_t* const keyname = (key + 1);
			ScriptVariableList* const varList = obj->Vars();

			const ScriptVariable* const var = varList->GetVariable(keyname);
			if (var)
			{
				if (dbg)
				{
					*dbg << "^~^~^ variable '"
						<< keyname
						<< "' already set with string value '"
						<< var->stringValue().c_str()
						<< "' - failed to attempt to set with string value '"
						<< value
						<< "'"
						<< std::endl;
				}

				continue;
			}

			const EventDef* def;
			const eventNum_t eventnum = evt.FindSetterEventNum(keyname);

			if (!eventnum || !(def = cls->GetDef(eventnum)))
			{
				varList->SetVariable(keyname, value);
			}
			else if (def->GetAttributes().GetType() != evType_e::Setter)
			{
				if (dbg) {
					*dbg << "^~^~^ Cannot set a read-only variable '" << keyname << "'" << std::endl;
				}
			}
			else
			{
				Event* const ev = new Event(*def, 1);
				ev->AddString(value);

				obj->PostEvent(ev, EV_SPAWNARG);
			}
		}
		else
		{
			const eventInfo_t* info = evt.FindEventInfo(key);
			if (info)
			{
				eventNum_t num = info->normalNum;
				if (!num) {
					num = info->setterNum;
				}

				Event* const ev = new Event(num);
				ev->AddString(value);

				if (!str::icmp(key, "model"))
				{
					obj->PostEvent(ev, EV_PRIORITY_SPAWNARG);
				}
				else
				{
					obj->PostEvent(ev, EV_SPAWNARG);
				}
			}
		}
	}

	return obj;
}

const char* SpawnErrors::NoSpawnFunction::what() const noexcept
{
	return "No spawn function found";
}

MFUS_CLASS_DECLARATION(Class, SpawnArgs, nullptr)
{
	{ NULL, NULL }
};
