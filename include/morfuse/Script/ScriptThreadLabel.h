#pragma once

#include "Class.h"

namespace mfuse
{
    class Listener;
    class ScriptThread;
    class ScriptVariable;
    class ProgramScript;

    class ScriptThreadLabel
    {
    public:
        ScriptThreadLabel();

        ScriptThread* Create(Listener* listener);
        void Execute(Listener* listener = nullptr);
        void Execute(Listener* listener, Event &ev);

        void Set(const rawchar_t* label);
        void SetScript(const ScriptVariable& label);
        void SetScript(const rawchar_t* label);
        void SetThread(const ScriptVariable& label);

        bool TrySet(const_str label);
        bool TrySet(const rawchar_t* label);
        bool TrySetScript(const_str label);
        bool TrySetScript(const rawchar_t* label);

        bool IsSet();

        void Archive(Archiver& arc);

    private:
        const ProgramScript* m_Script;
        const_str m_Label;
    };
}
