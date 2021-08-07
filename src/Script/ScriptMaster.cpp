#include <morfuse/Script/EventSystem.h>
#include <morfuse/Script/Context.h>
#include <morfuse/Script/ScriptMaster.h>
#include <morfuse/Script/ConstStr.h>
#include <morfuse/Script/Parm.h>
#include <morfuse/Script/Listener.h>
#include <morfuse/Script/Level.h>
#include <morfuse/Script/ScriptThread.h>
#include <morfuse/Script/ScriptClass.h>
#include <morfuse/Script/ScriptException.h>
#include <morfuse/Script/ScriptVM.h>
#include <morfuse/Script/ProgramScript.h>

using namespace mfuse;

const rawchar_t* ScriptMaster::ConstStrings[] =
{
	"",
	"remove",
	"delete"
};

ScriptMaster::ScriptMaster()
	: stackCount(0)
	, ContainerHead(nullptr)
{
	InitConstStrings();
}

ScriptMaster::~ScriptMaster()
{
	ClearAll();
}

void ScriptMaster::InitConstStrings()
{
	const size_t numStrings = sizeof(ConstStrings) / sizeof(ConstStrings[0]);

	StringDict.resize(numStrings);
	for (uintptr_t i = 0; i < numStrings; i++)
	{
		AddString(ConstStrings[i]);
	}
}

const_str ScriptMaster::AddString(xstrview s)
{
	if(s.isDynamic()) {
		return (const_str)StringDict.addKeyIndex(s.getDynamicString());
	}
	else {
		return StringDict.addKeyIndex(xstr(s.c_str(), s.len()));
	}
}

const_str ScriptMaster::GetString(const rawchar_t *s)
{
	const_str cs = (const_str)StringDict.findKeyIndex(s);
	return cs ? cs : STRING_EMPTY;
}

const xstr& ScriptMaster::GetString(const_str s)
{
	return StringDict[s];
}

void ScriptMaster::AllocateMoreString(size_t count)
{
	if (StringDict.size() + count > StringDict.allocated()) {
		StringDict.resize(StringDict.size() + count);
	}
}

void ScriptMaster::AddTiming(ScriptThread* Thread, uint64_t Time)
{
	const uinttime_t t = ScriptContext::Get().GetTimeManager().GetTime();
	timerList.AddElement(Thread, t + Time);
}

void ScriptMaster::RemoveTiming(ScriptThread* Thread)
{
	timerList.RemoveElement(Thread);
}

uintptr_t ScriptMaster::GetStackCount() const
{
	return stackCount;
}

void ScriptMaster::AddStack()
{
	stackCount++;
}

void ScriptMaster::RemoveStack()
{
	stackCount--;
	assert(stackCount >= 0);
}

bool ScriptMaster::HasLoopDrop() const
{
	return false;
}

bool ScriptMaster::HasLoopProtection() const
{
	return true;
}

ScriptClass* ScriptMaster::GetHeadContainer() const
{
	return ContainerHead;
}

ScriptThread* ScriptMaster::CreateScriptThread(const ProgramScript *scr, Listener *self, const_str label)
{
	ScriptClass* scriptClass = new ScriptClass(scr, self);

	return CreateScriptThread(scriptClass, label);
}

ScriptThread* ScriptMaster::CreateScriptThread(const ProgramScript *scr, Listener *self, const rawchar_t* label)
{
	return CreateScriptThread(scr, self, AddString(label));
}

ScriptThread* ScriptMaster::CreateScriptThread(ScriptClass* scriptClass, const_str label)
{
	const ProgramScript* const script = scriptClass->GetScript();
	if (!script) {
		return nullptr;
	}

	const opval_t* const m_pCodePos =
		label != STRING_EMPTY
		? scriptClass->FindLabel(label)
		: script->GetProgBuffer();

	if (!m_pCodePos)
	{
		ScriptError("ScriptMaster::CreateScriptThread: label '%s' does not exist in '%s'.", GetString(label).c_str(), scriptClass->Filename().c_str());
	}

	return CreateScriptThread(scriptClass, m_pCodePos);
}

ScriptThread* ScriptMaster::CreateScriptThread(ScriptClass* scriptClass, const rawchar_t* label)
{
	if (label && *label)
	{
		return CreateScriptThread(scriptClass, AddString(label));
	}
	else
	{
		return CreateScriptThread(scriptClass, STRING_EMPTY);
	}
}

ScriptThread* ScriptMaster::CreateScriptThread(ScriptClass* scriptClass, const opval_t* m_pCodePos)
{
	return new ScriptThread(scriptClass, m_pCodePos);
}

ScriptThread* ScriptMaster::CreateThread(const ProgramScript *scr, const rawchar_t* label, Listener *self)
{
	try
	{
		return CreateScriptThread(scr, self, label);
	}
	catch (ScriptException&)
	{
		//glbs.DPrintf("ScriptMaster::CreateThread: %s\n", exc.string.c_str());
		return NULL;
	}
}

ScriptThread* ScriptMaster::CreateThread(const rawchar_t* scriptName, const rawchar_t* label, Listener* self)
{
	const ProgramScript* const scr = GetScript(scriptName);
	if(scr)
	{
		return CreateThread(scr, label, self);
	}

	return nullptr;
}

ScriptThread* ScriptMaster::CreateThread(const ProgramScript* scr, const_str label, Listener* self)
{
	try
	{
		return CreateScriptThread(scr, self, label);
	}
	catch (ScriptException&)
	{
		//glbs.DPrintf("ScriptMaster::CreateThread: %s\n", exc.string.c_str());
		return NULL;
	}
}

ScriptThread* ScriptMaster::CreateThread(const rawchar_t* scriptName, const_str label, Listener* self)
{
	const ProgramScript* const scr = GetScript(scriptName);
	if (scr)
	{
		return CreateThread(scr, label, self);
	}

	return nullptr;
}

void ScriptMaster::ExecuteThread(const ProgramScript* scr, const rawchar_t* label)
{
	ScriptThread* const Thread = label
		? CreateThread(scr, label)
		: CreateThread(scr, STRING_EMPTY);

	try
	{
		if (Thread)
		{
			Thread->Execute();
		}
	}
	catch (ScriptException&)
	{
		//glbs.DPrintf("ScriptMaster::ExecuteThread: %s\n", exc.string.c_str());
	}
}

void ScriptMaster::ExecuteThread(const ProgramScript* scr, const rawchar_t* label, Event& parms)
{
	ScriptThread* const Thread = label
		? CreateThread(scr, label)
		: CreateThread(scr, STRING_EMPTY);

	try
	{
		Thread->Execute(parms);
	}
	catch (ScriptException&)
	{
		//glbs.DPrintf("ScriptMaster::ExecuteThread: %s\n", exc.string.c_str());
	}
}

void ScriptMaster::ExecuteThread(const rawchar_t* scriptName, const rawchar_t* label)
{
	ScriptThread* const Thread = label
		? CreateThread(scriptName, label)
		: CreateThread(scriptName, STRING_EMPTY);

	try
	{
		Thread->Execute();
	}
	catch (ScriptException&)
	{
		//glbs.DPrintf("ScriptMaster::ExecuteThread: %s\n", exc.string.c_str());
	}
}

void ScriptMaster::ExecuteThread(const rawchar_t* scriptName, const rawchar_t* label, Event& parms)
{
	ScriptThread* const Thread = label
		? CreateThread(scriptName, label)
		: CreateThread(scriptName, STRING_EMPTY);

	try
	{
		Thread->Execute(parms);
	}
	catch (ScriptException&)
	{
		//glbs.DPrintf("ScriptMaster::ExecuteThread: %s\n", exc.string.c_str());
	}
}

ScriptClass* ScriptMaster::CurrentScriptClass()
{
	return CurrentThread()->GetScriptClass();
}

ScriptThread* ScriptMaster::CurrentThread()
{
	return m_CurrentThread;
}

ScriptThread* ScriptMaster::PreviousThread()
{
	return m_PreviousThread;
}

const ProgramScript* ScriptMaster::GetTempScript(const char* data, uint64_t dataLength)
{
	ProgramScript* const scr = new ProgramScript(STRING_EMPTY);

	scr->Load((void *)data, dataLength);

	if (!scr->IsCompileSuccess())
	{
		return NULL;
	}

	return scr;
}

const ProgramScript* ScriptMaster::GetGameScriptInternal(const_str scriptName, const char* sourceBuffer, size_t sourceLength)
{
	const ProgramScript* const* const pSrc = m_GameScripts.find(scriptName);

	if (pSrc && *pSrc)
	{
		return *pSrc;
	}

	ProgramScript* const scr = new ProgramScript(scriptName);

	m_GameScripts[scriptName] = scr;

	/*
	if (GetCompiledScript(scr))
	{
		scr->m_Filename = AddString(filename);
		return scr;
	}
	*/

	scr->Load(sourceBuffer, sourceLength);

	if (!scr->IsCompileSuccess())
	{
		ScriptError("Script '%s' was not properly loaded", GetString(scriptName).c_str());
	}

	return scr;
}

const ProgramScript* ScriptMaster::GetGameScript(const rawchar_t* scriptName, const char* data, uint64_t dataLength, bool recompile)
{
	const_str s = AddString(scriptName);
	return GetGameScript(s, data, dataLength, recompile);
}

const ProgramScript* ScriptMaster::GetGameScript(const_str scriptName, const char* data, uint64_t dataLength, bool recompile)
{
	ProgramScript** pScr = m_GameScripts.find(scriptName);
	ProgramScript* scr = pScr ? *pScr : nullptr;

	if (scr != NULL && !recompile)
	{
		if (!scr->IsCompileSuccess())
		{
			ScriptError("Script '%s' was not properly loaded\n", GetString(scriptName).c_str());
		}

		return scr;
	}
	else
	{
		if (scr && recompile)
		{
			con::Container<ScriptClass*> list;
			ScriptClass* scriptClass;
			m_GameScripts[scriptName] = NULL;

			for (scriptClass = GetHeadContainer(); scriptClass != nullptr; scriptClass = scriptClass->GetNext())
			{
				if (scriptClass->GetScript() == scr)
				{
					list.AddObject(scriptClass);
				}
			}

			for (uintptr_t i = 1; i <= list.NumObjects(); i++)
			{
				delete list.ObjectAt(i);
			}

			delete scr;
		}

		if (data) {
			return GetGameScriptInternal(scriptName, data, dataLength);
		}

		return nullptr;
	}
}

const ProgramScript* ScriptMaster::GetGameScript(const rawchar_t* scriptName, bool recompile)
{
// FIXME
/*
	AssetManagerPtr assetManager = GetContext()->getAssetManager();
	if(assetManager)
	{
		FilePtr file = assetManager->GetFileManager()->OpenFile(scriptName);
		if(file)
		{
			const rawchar_t* data;
			uint64_t dataLen = file->ReadBuffer((void**)&data);
		}
	}
*/
	ScriptError("Can't find %s", scriptName);
	return nullptr;
}

const ProgramScript* ScriptMaster::GetGameScript(const_str scriptName, bool recompile)
{
	// FIXME
	ScriptError("Can't find %s", GetString(scriptName).c_str());
	return nullptr;
}

const ProgramScript* ScriptMaster::GetScript(const rawchar_t* scriptName, const char* data, uint64_t dataLength, bool recompile)
{
	try
	{
		return GetGameScript(scriptName, data, dataLength, recompile);
	}
	catch (std::exception& e)
	{
		ScriptContext::Get().GetOutputInfo().DPrintf("ScriptMaster::GetScript: %s\n", e.what());
	}

	return NULL;
}

const ProgramScript* ScriptMaster::GetScript(const rawchar_t* scriptName, bool recompile)
{
	try
	{
		return GetGameScript(scriptName, recompile);
	}
	catch (std::exception&)
	{
		//glbs.Printf("ScriptMaster::GetScript: %s\n", exc.string.c_str());
	}

	return NULL;
}

const ProgramScript* ScriptMaster::GetScript(const_str scriptName, const char* data, uint64_t dataLength, bool recompile)
{
	try
	{
		return GetGameScript(scriptName, data, dataLength, recompile);
	}
	catch (std::exception& e)
	{
		ScriptContext::Get().GetOutputInfo().DPrintf("ScriptMaster::GetScript: %s\n", e.what());
	}

	return NULL;
}

const ProgramScript* ScriptMaster::GetScript(const_str scriptName, bool recompile)
{
	try
	{
		return GetGameScript(scriptName, recompile);
	}
	catch (std::exception&)
	{
		//glbs.Printf("ScriptMaster::GetScript: %s\n", exc.string.c_str());
	}

	return NULL;
}

void ScriptMaster::CloseGameScript()
{
	con::map_enum<const_str, ProgramScript*> en(m_GameScripts);
	ProgramScript** g;
	con::Container<ProgramScript*> gameScripts;

	for (g = en.NextValue(); g; g = en.NextValue())
	{
		if (*g) delete *g;
	}

	m_GameScripts.clear();
}

void ScriptMaster::ClearAll()
{
	// Destroy and free all script class
	ScriptContext::Get().GetAllocator().GetBlock<ScriptClass>().FreeAll();

	stackCount = 0;

	CloseGameScript();
	StringDict.clear();
}

void ScriptMaster::Reset()
{
	ClearAll();
	InitConstStrings();
}

void ScriptMaster::ExecuteRunning()
{
	if (stackCount) {
		return;
	}

	if (timerList.IsDirty())
	{
		uint64_t i = 0;
		while ((m_CurrentThread = (ScriptThread*)timerList.GetNextElement(i)))
		{
			ScriptVM* const scriptVM = m_CurrentThread->GetScriptVM();
			scriptVM->SetThreadState(threadState_e::Running);
			scriptVM->Execute();
		}
	}
}

Parm& ScriptMaster::GetParm()
{
	return parm;
}

void ScriptMaster::SetTime(uinttime_t time)
{
	timerList.SetTime(time);
	timerList.SetDirty();
}

