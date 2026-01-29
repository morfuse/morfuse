#include <morfuse/Common/ConstStr.h>
#include <morfuse/Script/EventSystem.h>
#include <morfuse/Script/Context.h>
#include <morfuse/Script/ScriptMaster.h>
#include <morfuse/Script/Parm.h>
#include <morfuse/Script/Listener.h>
#include <morfuse/Script/Level.h>
#include <morfuse/Script/ScriptThread.h>
#include <morfuse/Script/ScriptClass.h>
#include <morfuse/Script/ScriptException.h>
#include <morfuse/Script/ScriptVM.h>
#include <morfuse/Script/ProgramScript.h>
#include <morfuse/Script/PredefinedString.h>
#include <morfuse/Script/Archiver.h>

using namespace mfuse;

ScriptMaster::ScriptMaster()
    : headScript(nullptr)
{
    InitConstStrings();
}

ScriptMaster::~ScriptMaster()
{
    ClearAll();
}

void ScriptMaster::InitConstStrings()
{
    dict.AllocateMoreString(PredefinedString::GetNumStrings());

    for (PredefinedString::List::iterator it = PredefinedString::GetList(); it; it = it.Next())
    {
        const const_str value = dict.Add(it->GetString());
        (void)value;
        assert(value == it->GetIndex());
    }
}

void ScriptMaster::AddTiming(ScriptThread* Thread, uint64_t Time)
{
    const uinttime_t t = ScriptContext::Get().GetTimeManager().GetScaledTime();
    timerList.AddElement(Thread, t + Time);
}

void ScriptMaster::RemoveTiming(ScriptThread* Thread)
{
    timerList.RemoveElement(Thread);
}

ScriptClass* ScriptMaster::GetHeadContainer() const
{
    return headScript;
}

ScriptThread* ScriptMaster::CreateScriptThread(const ProgramScript *scr, Listener *self, const StringResolvable& label)
{
    ScriptClass* scriptClass = nullptr;
    
    try
    {
        scriptClass = new ScriptClass(scr, self);

        return CreateScriptThread(scriptClass, label);
    }
    catch (std::exception&)
    {
        if (scriptClass) {
            delete scriptClass;
        }

        throw;
    }
}

ScriptThread* ScriptMaster::CreateScriptThread(ScriptClass* scriptClass, const StringResolvable& label)
{
    const opval_t* codePos;

    if (!label.IsEmpty())
    {
        const script_label_t* s = scriptClass->FindLabel(label.GetConstString());
        if (!s) {
            throw StateScriptErrors::LabelNotFound(label, scriptClass->Filename());
        }
        
        codePos = s->codepos;
    }
    else
    {
        // start at the beginning
        codePos = scriptClass->GetScript()->GetProgBuffer();
    }

    assert(codePos);
    return CreateScriptThread(scriptClass, codePos);
}

ScriptThread* ScriptMaster::CreateScriptThread(ScriptClass* scriptClass, const opval_t* codePos)
{
    return new ScriptThread(scriptClass, codePos);
}

ScriptThread* ScriptMaster::ExecuteThread(const ProgramScript* scr, const StringResolvable& label)
{
    ScriptThread* const thread = CreateScriptThread(scr, nullptr, label);
    SafePtr<ScriptThread> threadPtr = thread;
    thread->Execute();

    return threadPtr.Pointer();
}

ScriptThread* ScriptMaster::ExecuteThread(const StringResolvable& scriptName, const StringResolvable& label)
{
    const ProgramScript* const script = GetProgramScript(scriptName);
    return ExecuteThread(script, label);
}

ScriptThread* ScriptMaster::ExecuteThread(const ProgramScript* scr, Event& parms, const StringResolvable& label)
{
    ScriptThread* const thread = CreateScriptThread(scr, nullptr, label);
    SafePtr<ScriptThread> threadPtr = thread;
    thread->Execute(parms);

    return threadPtr.Pointer();
}

ScriptThread* ScriptMaster::ExecuteThread(const StringResolvable& scriptName, Event& parms, const StringResolvable& label)
{
    const ProgramScript* const script = GetProgramScript(scriptName);
    return ExecuteThread(script, parms, label);
}

ScriptClass* ScriptMaster::CurrentScriptClass()
{
    return CurrentThread()->GetScriptClass();
}

ScriptThread* ScriptMaster::CurrentThread() noexcept
{
    return m_CurrentThread;
}

ScriptThread* ScriptMaster::PreviousThread() noexcept
{
    return m_PreviousThread;
}

const ProgramScript* ScriptMaster::GetTempScript(std::istream& stream)
{
    ProgramScript* const scr = new ProgramScript(ConstStrings::Empty);

    scr->Load(stream);

    if (!scr->IsCompileSuccess())
    {
        return NULL;
    }

    return scr;
}

const ProgramScript* ScriptMaster::GetProgramScriptInternal(const_str scriptName, std::istream& stream)
{
    const ProgramScript* const* const pSrc = m_ProgramScripts.find(scriptName);

    if (pSrc && *pSrc)
    {
        return *pSrc;
    }

    ProgramScript* const scr = new ProgramScript(scriptName);

    m_ProgramScripts[scriptName] = scr;

    /*
    if (GetCompiledScript(scr))
    {
        scr->m_Filename = AddString(filename);
        return scr;
    }
    */

    scr->Load(stream);

    return scr;
}

const ProgramScript* ScriptMaster::GetProgramScript(const StringResolvable& scriptName, std::istream& stream, bool recompile)
{
    const const_str constStringValue = scriptName.GetConstString(GetDictionary());
    ProgramScript* scr = FindScript(constStringValue);

    if (scr && !recompile)
    {
        if (!scr->IsCompileSuccess())
        {
            throw ScriptException(
                "Script '" + scriptName.GetString(GetDictionary()) + "' was not properly loaded"
            );
        }

        return scr;
    }
    else
    {
        if (scr && recompile)
        {
            // since it's about recompiling, delete the previous script
            DeleteProgramScript(scr);
        }

        return GetProgramScriptInternal(constStringValue, stream);
    }
}

mfuse::ProgramScript* ScriptMaster::FindScript(const_str scriptName) const
{
    ProgramScript* const* pScr = m_ProgramScripts.find(scriptName);
    return pScr ? *pScr : nullptr;
}

void ScriptMaster::DeleteProgramScript(ProgramScript* script)
{
    m_ProgramScripts.remove(script->Filename());

    con::Container<ScriptClass*> list;
    ScriptClass* scriptClass;

    ScriptClass* next;
    for (scriptClass = GetHeadContainer(); scriptClass != nullptr; scriptClass = next)
    {
        next = scriptClass->GetNext();
        if (scriptClass->GetScript() == script) {
            delete scriptClass;
        }
    }

    delete script;
}

const ProgramScript* ScriptMaster::GetProgramScript(const StringResolvable& scriptName, bool recompile)
{
    const ScriptContext& context = ScriptContext::Get();
    IFileManagement* fileManagement = context.GetScriptInterfaces().fileManagement;
    if (!fileManagement) {
        throw ScriptException("Not implementation for file provider");
    }

    const const_str constScriptName = scriptName.GetConstString(GetDictionary());
    ProgramScript* const scr = FindScript(constScriptName);
    if (scr && !recompile)
    {
        return scr;
    }
    else
    {
        if (scr && recompile)
        {
            // since it's about recompiling, delete the previous script
            DeleteProgramScript(scr);
        }
    }

    const rawchar_t* fname = scriptName.GetRawString(GetDictionary());
    // now open the file
    IFile* file = fileManagement->OpenFile(fname);
    if (!file)
    {
        throw ScriptException(
            "Can't find '" + str(fname) + "'"
        );
    }

    std::istream& stream = file->getStream();
    // compile as the stream has been obtained
    const ProgramScript* script = GetProgramScript(scriptName, stream, recompile);

    fileManagement->CloseFile(file);

    return script;
}

void ScriptMaster::CloseProgramScript()
{
    con::map_enum<const_str, ProgramScript*> en(m_ProgramScripts);
    ProgramScript* const* pg;
    con::Container<ProgramScript*> programScripts;

    // delete all programs
    for (pg = en.NextValue(); pg; pg = en.NextValue())
    {
        if (*pg) {
            delete* pg;
        }
    }

    m_ProgramScripts.clear();
}

void ScriptMaster::ClearAll()
{
    // Destroy and free all script class
    ScriptContext::Get().GetAllocator().GetBlock<ScriptClass>().FreeAll();

    CloseProgramScript();
    dict.Reset();

    // reinitialize const strings
    InitConstStrings();
}

void ScriptMaster::Reset()
{
    ClearAll();
}

void ScriptMaster::ExecuteRunning()
{
    if (CurrentThread())
    {
        // don't do anything if there is a running thread
        return;
    }

    if (timerList.IsDirty())
    {
        uint64_t i = 0;
        while ((m_CurrentThread = (ScriptThread*)timerList.GetNextElement(i)))
        {
            m_CurrentThread->Resume();
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
}

ThreadExecutionProtection& ScriptMaster::GetThreadExecutionProtection()
{
    return execProtection;
}

const ThreadExecutionProtection& ScriptMaster::GetThreadExecutionProtection() const
{
    return execProtection;
}

const con::timer& ScriptMaster::GetTimerList() const
{
    return timerList;
}

size_t ScriptMaster::GetNumScripts() const
{
    return m_ProgramScripts.size();
}

size_t ScriptMaster::GetNumRunningScripts() const
{
    return ScriptContext::Get().GetAllocator().GetBlock<ScriptClass>().Count();
}

StringDictionary& ScriptMaster::GetDictionary()
{
    return dict;
}

const StringDictionary& ScriptMaster::GetDictionary() const
{
    return dict;
}

void ScriptMaster::Archive(Archiver& arc)
{
    if (arc.Loading())
    {
        // make sure to kill other script classes before continuing
        KillScripts();

        uint32_t count;
        arc.ArchiveUInt32(count);

        for (uint32_t i = 0; i < count; ++i)
        {
            ScriptClass* s;
            ScriptClass::ArchiveScript(*this, arc, s);
        }
    }
    else
    {
        uint32_t count = 0;
        // count the number of script class in this script master
        for (const ScriptClass* s = headScript; s; s = s->GetNext()) {
            ++count;
        }

        arc.ArchiveUInt32(count);
        // now archive all script classes individually
        for (ScriptClass* s = headScript; s; s = s->GetNext())
        {
            ScriptClass::ArchiveScript(*this, arc, s);
        }
    }

    timerList.Archive(arc);
}

void ScriptMaster::KillScripts()
{
    ScriptClass* next;
    for (ScriptClass* s = headScript; s; s = next)
    {
        next = s->GetNext();
        delete s;
    }

    headScript = nullptr;
}

ThreadExecutionProtection::ThreadExecutionProtection()
    : maxExecutionTime(5000)
    , loopProtection(false)
{
}

mfuse::uinttime_t ThreadExecutionProtection::GetMaxExecutionTime() const
{
    return maxExecutionTime;
}

void ThreadExecutionProtection::SetMaxExecutionTime(uinttime_t time)
{
    maxExecutionTime = time;
}

bool ThreadExecutionProtection::ShouldDrop() const
{
    return loopProtection;
}

void ThreadExecutionProtection::SetLoopProtection(bool protect)
{
    loopProtection = protect;
}
