#include <morfuse/Script/ScriptClass.h>
#include <morfuse/Script/ScriptThread.h>
#include <morfuse/Script/ProgramScript.h>
#include <morfuse/Script/ScriptVariable.h>
#include <morfuse/Script/ScriptException.h>
#include <morfuse/Script/ScriptVM.h>
#include <morfuse/Script/ScriptMaster.h>
#include <morfuse/Script/Context.h>
#include <morfuse/Script/Archiver.h>

using namespace mfuse;

MFUS_CLASS_DECLARATION(Listener, ScriptClass, NULL)
{
    { NULL, NULL }
};

ScriptClass::ScriptClass(const ProgramScript *gameScript, Listener *self)
    : ScriptClass(ScriptContext::Get().GetDirector())
{
    m_Self = self;
    m_Script = gameScript;
}

ScriptClass::ScriptClass()
    : ScriptClass(ScriptContext::Get().GetDirector())
{
}

ScriptClass::ScriptClass(ScriptMaster& director)
    : m_Script(nullptr)
    , m_Self(nullptr)
    , m_Threads(nullptr)
    , headScript(director.headScript)
{
    LL::SafeAddFront<ScriptClass*, &ScriptClass::Next, &ScriptClass::Prev>(headScript, this);
}

ScriptClass::~ScriptClass()
{
    if (m_Script == NULL)
    {
        //ScriptError("Attempting to delete dead class.");
        return;
    }

    LL::SafeRemoveRoot<ScriptClass*, &ScriptClass::Next, &ScriptClass::Prev>(headScript, this);

    KillThreads();

    if (!m_Script->Filename())
    {
        // This is a temporary gamescript created for this script class, so delete it
        delete m_Script;
    }
}

void ScriptClass::StoppedNotify()
{
    delete this;
}

void* ScriptClass::operator new(size_t)
{
    return ScriptContext::Get().GetAllocator().ScriptClass_allocator.Alloc();
}

void ScriptClass::operator delete(void* ptr)
{
    ScriptContext::Get().GetAllocator().ScriptClass_allocator.Free(ptr);
}

void ScriptClass::Archive(Archiver&)
{
}

void ScriptClass::ArchiveInternal(Archiver& arc)
{
    Listener::Archive(arc);

    arc.ArchiveObjectPosition(this);
    arc.ArchiveSafePointer(m_Self);
    ProgramScript::Archive(arc, m_Script);
}

void ScriptClass::ArchiveScript(ScriptMaster& director, Archiver& arc, ScriptClass*& classRef)
{
    if (arc.Loading())
    {
        ScriptClass* const s = new ScriptClass(director);
        s->ArchiveInternal(arc);

        uint32_t threadCount;
        arc.ArchiveUInt32(threadCount);

        ScriptVM* prevVM = nullptr;
        for (uint32_t j = 0; j < threadCount; ++j)
        {
            ScriptVM* vm = new ScriptVM();
            ScriptThread* thread = new ScriptThread();

            vm->m_Thread = thread;
            thread->m_ScriptVM = vm;
            vm->m_ScriptClass = s;

            if (prevVM) {
                prevVM->next = vm;
            }
            else {
                s->m_Threads = vm;
            }

            thread->ArchiveInternal(arc);
        }

        classRef = s;
    }
    else
    {
        ScriptClass* const s = classRef;
        s->ArchiveInternal(arc);

        // count the number of threads
        uint32_t threadCount = 0;
        for (const ScriptVM* vm = s->FirstThread(); vm; vm = s->NextThread(vm)) {
            ++threadCount;
        }
        arc.ArchiveUInt32(threadCount);

        // now archive threads individually
        for (ScriptVM* vm = s->FirstThread(); vm; vm = s->NextThread(vm))
        {
            ScriptThread* const thread = vm->GetScriptThread();
            thread->ArchiveInternal(arc);
        }
    }
}

void ScriptClass::ArchiveCodePos(Archiver& arc, const opval_t*& codePos)
{
    m_Script->ArchiveCodePos(arc, codePos);
}

ScriptThread* ScriptClass::CreateScriptInternal(const ScriptVariable& label)
{
    ScriptMaster& director = ScriptContext::Get().GetDirector();

    if (label.GetType() == variableType_e::String || label.GetType() == variableType_e::ConstString)
    {
        const ProgramScript* const scr = director.GetProgramScript(label.constStringValue());
        return director.CreateScriptThread(scr, m_Self, StringResolvable());
    }
    else if (label.GetType() == variableType_e::ConstArray && label.arraysize() > 1)
    {
        const ScriptVariable& script = label[1];
        const ScriptVariable& labelname = label[2];

        const ProgramScript* const scr = director.GetProgramScript(script.constStringValue());
        return director.CreateScriptThread(scr, m_Self, labelname.constStringValue());
    }
    else
    {
        throw ListenerErrors::BadLabelType(label.GetTypeName());
    }
}

ScriptThread *ScriptClass::CreateThreadInternal(const ScriptVariable& label)
{
    ScriptMaster& director = ScriptContext::Get().GetDirector();

    if (label.GetType() == variableType_e::String || label.GetType() == variableType_e::ConstString)
    {
        // create a scriptclass in the same group
        return director.CreateScriptThread(this, label.constStringValue());
    }
    else if (label.GetType() == variableType_e::ConstArray && label.arraysize() > 1)
    {
        const ScriptVariable& script = label[1];
        const ScriptVariable& labelname = label[2];

        const ProgramScript* const scr = director.GetProgramScript(script.constStringValue());
        return director.CreateScriptThread(scr, m_Self, labelname.constStringValue());
    }
    else
    {
        throw ListenerErrors::BadLabelType(label.GetTypeName());
    }
}

void ScriptClass::AddThread(ScriptVM *m_ScriptVM)
{
    m_ScriptVM->SetNext(m_Threads);
    m_Threads = m_ScriptVM;
}

void ScriptClass::KillThreads()
{
    if (!m_Threads)
    {
        return;
    }

    ScriptVM *m_current;
    ScriptVM *m_next;

    m_current = m_Threads;

    do
    {
        m_current->ClearScriptClass();

        m_next = m_current->GetNext();
        delete m_current->GetScriptThread();

    } while ((m_current = m_next));

    m_Threads = NULL;
}

void ScriptClass::RemoveThread(ScriptVM *m_ScriptVM)
{
    if (m_Threads == m_ScriptVM)
    {
        m_Threads = m_ScriptVM->GetNext();

        if (m_Threads == NULL) {
            delete this;
        }
    }
    else
    {
        ScriptVM *m_current = m_Threads;
        ScriptVM *i;

        for (i = m_Threads->GetNext(); i != m_ScriptVM; i = i->GetNext()) {
            m_current = i;
        }

        m_current->SetNext(i->GetNext());
    }
}

const_str ScriptClass::Filename() const
{
    return m_Script->Filename();
}

const script_label_t* ScriptClass::FindLabel(const_str label) const
{
    return m_Script->GetStateScript().FindLabel(label);
}

StateScript *ScriptClass::GetCatchStateScript(const opval_t *in, const opval_t *&out) const
{
    return m_Script->GetCatchStateScript(in, out);
}

const ProgramScript *ScriptClass::GetScript() const
{
    return m_Script;
}

Listener *ScriptClass::GetSelf() const
{
    return static_cast<Listener *>(m_Self.Pointer());
}

void ScriptClass::SetSelf(Listener* l)
{
    m_Self = l;
}

ScriptClass* ScriptClass::GetNext() const
{
    return Next;
}

ScriptClass* ScriptClass::GetPrev() const
{
    return Prev;
}

ScriptVM* ScriptClass::FirstThread() const
{
    return m_Threads;
}

ScriptVM* ScriptClass::NextThread(ScriptVM* vm) const
{
    return vm->GetNext();
}

const ScriptVM* ScriptClass::NextThread(const ScriptVM* vm) const
{
    return vm->GetNext();
}
