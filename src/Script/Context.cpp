#include <morfuse/Script/Context.h>
#include <morfuse/Script/Game.h>
#include <morfuse/Script/Level.h>
#include <morfuse/Script/EventQueue.h>

using namespace mfuse;

ScriptContext::ScriptContext()
	//: ThreadSingleton(this)
{
}

ScriptContext::~ScriptContext()
{
}

TargetList& ScriptContext::GetTargetList()
{
	return targetList;
}

Level* ScriptContext::GetLevel()
{
	return &level;
}

Game* ScriptContext::GetGame()
{
	return &game;
}

ScriptMaster& ScriptContext::GetDirector()
{
	return director;
}

void ScriptContext::Execute(float timeScale)
{
	uint64_t deltaTime;
	TimeManager& timeMan = GetTimeManagerInternal();
	if (timeScale == 1.f) {
		deltaTime = timeMan.Frame();
	} else {
		deltaTime = timeMan.Frame(timeScale);
	}

	director.SetTime(timeMan.GetTime());

	// process current pending events
	GetEventQueue().ProcessPendingEvents();

	// process all scripts
	director.ExecuteRunning();
}

DefaultScriptAllocator& ScriptContext::GetAllocator()
{
	return scriptAllocator;
}

OutputInfo& ScriptContext::GetOutputInfo()
{
	return outputInfo;
}

ScriptSettings& ScriptContext::GetSettings()
{
	return settings;
}

ScriptSettings::ScriptSettings()
	: bDeveloper(false)
{
}

void ScriptSettings::SetDeveloperEnabled(bool value)
{
	bDeveloper = value;
}

bool ScriptSettings::IsDeveloperEnabled() const
{
	return bDeveloper;
}
