#pragma once

#include <morfuse/Script/Class.h>
#include <morfuse/Script/ScriptOpcodes.h>
#include <morfuse/Script/ScriptException.h>
#include "../Parser/parsetree.h"

#include <ostream>

namespace mfuse
{
    class EventSystem;
    class ScriptVariable;
    class StateScript;
    class ProgramScript;
    class StateScript;
    class StringDictionary;
    class OutputInfo;

    constexpr unsigned int BREAK_JUMP_LOCATION_COUNT = 100;
    constexpr unsigned int CONTINUE_JUMP_LOCATION_COUNT = 100;
    constexpr unsigned int MAX_PREV_OPCODES = 100;

    struct sizeInfo_t {
        size_t length;
        size_t numLabels;
        size_t numCaseLabels;
        size_t numCatchLabels;
        size_t numCommands;
        size_t numStrings;
        size_t numCatches;
        size_t numSwitches;
        size_t progLength;

    public:
        sizeInfo_t();
    };

    class IScriptManager
    {
    public:
        virtual ~IScriptManager() = default;
        virtual StateScript* CreateCatchStateScript(const opval_t* try_begin_code_pos, const opval_t* code_pos, size_t labelCount) = 0;
        virtual StateScript* CreateSwitchStateScript(size_t labelCount) = 0;
        virtual bool AddLabel(StateScript* stateScript, const rawchar_t* name, const opval_t* code_pos, bool privateLabel = false) = 0;
        virtual bool AddCaseLabel(StateScript* stateScript, const rawchar_t* name, const opval_t* code_pos) = 0;
        virtual const_str AddString(const strview& value) = 0;
        virtual size_t GetCodePosition() = 0;
        virtual opval_t* GetCodePositionPtr() = 0;
        virtual size_t GetProgLength() = 0;
        virtual opval_t* MoveCodeBack(size_t backCount) = 0;
        virtual opval_t* MoveCodeForward(size_t fwdCount) = 0;
        virtual void SetValueAtCodePosition(opval_t* pos, const void* value, size_t size) = 0;
        virtual void GetValueAt(uintptr_t offset, void* value, size_t size) = 0;
        virtual void WriteOpcodeValue(const void* value, size_t size) = 0;
        virtual void AddSourcePos(const opval_t* code_pos, sourceLocation_t sourceLoc) = 0;

        template<typename T>
        void WriteOpcodeValue(T value)
        {
            WriteOpcodeValue(&value, sizeof(T));
        }

        template<typename T>
        T ReadOpcodeValue(uintptr_t backOffset)
        {
            T value;
            GetValueAt(backOffset, &value, sizeof(T));
            return value;
        }
    };

    class ScriptParser
    {
    public:
        ScriptParser();

        /**
         * Preprocess the source buffer by replacing macros
         * with their corresponding values and by parsing includes.
         *
         * @status not implemented
         */
        //const prchar_t* Preprocess(std::istream& stream);

        /**
         * Parse the source buffer and return an abstraction syntax tree.
         *
         * @param gameScript The input game script to use.
         * @param sourceBuffer The buffer to parse.
         * @param sourceLength The size of the buffer.
         */
        ParseTree Parse(const prchar_t* scriptName, std::istream& stream, const rawchar_t* sourceBuffer, uint64_t sourceLength);

        /**
         * Set the output info used to display debug info to end-user.
         *
         * @param infoValue The output info.
         */
        void SetOutputInfo(const OutputInfo* infoValue);

    private:
        const OutputInfo* info;
    };

    class ScriptEmitter
    {
    public:
        ScriptEmitter(IScriptManager& manager, StateScript& stateScriptValue, const OutputInfo* info, size_t maxDepth = -1);
        void Reset();

    public:
        void EmitRoot(sval_t root);
        uint32_t GetInternalMaxVarStackOffset() const;
        uint32_t GetMaxExternalVarStackOffset() const;

    private:
        template<typename...Args>
        void CompileError(sourceLocation_t sourceLoc, const rawchar_t* format, Args&& ...args);

        opval_t PrevOpcode();
        int8_t PrevVarStackOffset();
        void AbsorbPrevOpcode();
        void ClearPrevOpcode();
        void AccumulatePrevOpcode(opval_t op, int32_t iVarStackOffset);
        opval_t* code_pos() const;
        template<typename T> void WriteOpValue(T value) { manager.WriteOpcodeValue(&value, sizeof(T)); }
        template<typename T> T ReadOpValue(uintptr_t backOffset) { return manager.ReadOpcodeValue<T>(backOffset); }

        void AddBreakJumpLocation(opval_t* pos, sourceLocation_t sourceLoc);
        void AddContinueJumpLocation(opval_t* pos, sourceLocation_t sourceLoc);
        void AddJumpLocation(opval_t* pos);
        void AddJumpBackLocation(opval_t* pos);
        void AddJumpToLocation(opval_t* pos);

        bool BuiltinReadVariable(sourceLocation_t sourceLoc, uint8_t type, const rawchar_t* name, eventNum_t eventnum);
        bool BuiltinWriteVariable(sourceLocation_t sourceLoc, uint8_t type, const rawchar_t* name, eventNum_t eventnum);

        void EmitAssignmentStatement(sval_t lhs, sourceLocation_t sourceLoc);

        void EmitBoolJumpFalse(sourceLocation_t sourceLoc);
        void EmitBoolJumpTrue(sourceLocation_t sourceLoc);
        void EmitBoolNot(sourceLocation_t sourceLoc);
        void EmitBoolToVar(sourceLocation_t sourceLoc);
        void EmitVarToBool(sourceLocation_t sourceLoc);

        void EmitBreak(sourceLocation_t sourceLoc);
        void EmitCaseLabel(sval_t case_parm, sval_t parameter_list, sourceLocation_t sourceLoc);
        void EmitCaseLabel(const rawchar_t* name, sourceLocation_t sourceLoc);
        void EmitCaseLabel(int32_t label, sourceLocation_t sourceLoc);
        void EmitCatch(sval_t val, const opval_t* try_begin_code_pos, sourceLocation_t sourceLoc);
        void EmitCommandMethod(sval_t listener, const rawchar_t* commandName, sval_t parameter_list, sourceLocation_t sourceLoc);
        void EmitCommandMethodRet(sval_t listener, const rawchar_t* commandName, sval_t parameter_list, sourceLocation_t sourceLoc);
        void EmitCommandScript(const rawchar_t* commandName, sval_t parameter_list, sourceLocation_t sourceLoc);
        void EmitCommandScriptRet(const rawchar_t* commandName, sval_t parameter_list, sourceLocation_t sourceLoc);
        void EmitConstArray(sval_t lhs, sval_t rhs, sourceLocation_t sourceLoc);
        void EmitConstArrayOpcode(uint32_t iCount, sourceLocation_t sourceLoc);
        void EmitContinue(sourceLocation_t sourceLoc);
        void EmitDoWhileJump(sval_t while_stmt, sval_t while_expr, sourceLocation_t sourceLoc);
        void EmitEof(sourceLocation_t sourceLoc);
        void EmitField(sval_t listener_val, sval_t field_val, sourceLocation_t sourceLoc);
        void EmitFloat(float value, sourceLocation_t sourceLoc);
        void EmitFunc1(opval_t opcode, sourceLocation_t sourceLoc);
        void EmitFunction(uint32_t iParamCount, const rawchar_t* functionName, sourceLocation_t sourceLoc);
        void EmitIfElseJump(sval_t if_stmt, sval_t else_stmt, sourceLocation_t sourceLoc);
        void EmitIfJump(sval_t if_stmt, sourceLocation_t sourceLoc);
        void EmitInteger(uint64_t value, sourceLocation_t sourceLoc);
        void EmitJump(opval_t *pos, sourceLocation_t sourceLoc);
        void EmitJumpBack(opval_t *pos, sourceLocation_t sourceLoc);
        void EmitLabel(const rawchar_t* name, sourceLocation_t sourceLoc);
        void EmitLabel(unsigned int number, sourceLocation_t sourceLoc);
        void EmitLabelParameterList(sval_t parameter_list, sourceLocation_t sourceLoc);
        void EmitLabelPrivate(const rawchar_t* name, sourceLocation_t sourceLoc);
        void EmitAndJump(sval_t logic_stmt, sourceLocation_t sourceLoc);
        void EmitOrJump(sval_t logic_stmt, sourceLocation_t sourceLoc);
        void EmitMakeArray(sval_t val, sourceLocation_t sourceLoc);
        void EmitMethodExpression(uint32_t iParamCount, eventNum_t eventnum, sourceLocation_t sourceLoc);
        void EmitNil(sourceLocation_t sourceLoc);
        void EmitNop();
        uintptr_t EmitNot(sourceLocation_t sourceLoc);
        void EmitOpcode(opval_t opcode, sourceLocation_t sourceLoc);
        void EmitOpcodeWithStack(opval_t opcode, int32_t varStackOffset, sourceLocation_t sourceLoc);
        void EmitParameter(sval_t lhs, sourceLocation_t sourceLoc);
        uint32_t EmitParameterList(sval_t event_parameter_list);
        void EmitRef(sval_t val, sourceLocation_t sourceLoc);
        void EmitStatementList(sval_t val);
        void EmitString(const rawchar_t* value, sourceLocation_t sourceLoc);
        void EmitSwitch(sval_t val, sourceLocation_t sourceLoc);
        void EmitValue(sval_t val);
        void EmitValue(ScriptVariable& var, sourceLocation_t sourceLoc);
        void EmitWhileJump(sval_t while_expr, sval_t while_stmt, sval_t inc_stmt, sourceLocation_t sourceLoc);

        void ProcessBreakJumpLocations(int iStartBreakJumpLocCount);
        void ProcessContinueJumpLocations(int iStartContinueJumpLocCount);

        bool EvalPrevValue(ScriptVariable& var);

        bool IsDebugEnabled() const;

        bool GetCompiledScript(ProgramScript* scr);

        void OptimizeInstructions(opval_t* code, opval_t* op1, opval_t* op2);
        int OptimizeValue(int val1, int val2, opval_t opcode);

    private:
        StateScript* stateScript;
        IScriptManager& manager;
        EventSystem& eventSystem;
        size_t depth;
        size_t switchDepth;
        const OutputInfo* info;
        const rawchar_t* fileName;
        const rawchar_t* sourceBuffer;
        size_t sourceLength;

        uintptr_t current_label;
        opval_t* apucBreakJumpLocations[BREAK_JUMP_LOCATION_COUNT];
        opval_t* apucContinueJumpLocations[CONTINUE_JUMP_LOCATION_COUNT];

        uint32_t prev_opcode_pos;

        int16_t m_iVarStackOffset;
        int16_t m_iInternalMaxVarStackOffset;
        int16_t m_iMaxExternalVarStackOffset;
        int16_t m_iMaxCallStackOffset;
        uint16_t iBreakJumpLocCount;
        uint16_t iContinueJumpLocCount;

        opcode_info_t prev_opcodes[MAX_PREV_OPCODES];

        bool canBreak;
        bool canContinue;
        bool hasExternal;

    private:
        class StackDepth
        {
        public:
            StackDepth(size_t& depthRef);
            ~StackDepth();

        private:
            size_t& depth;
        };
    };

    class ScriptCompiler
    {
    public:
        ScriptCompiler();

        /**
         * Compile the abstract syntax tree into opcodes.
         *
         * @param progBuffer The program buffer where opcodes will be written to.
         */
        size_t Compile(ProgramScript* script, sval_t rootNode, opval_t*& progBuffer);

        /**
         * Optimize the result program buffer by removing useless opcodes.
         *
         * @status not implemented
         */
        void Optimize(opval_t* progBuffer);

        /**
         * Set the output info used to display debug info to end-user.
         *
         * @param infoValue The output info.
         */
        void SetOutputInfo(const OutputInfo* infoValue);

        /**
         * Return whether or not the compiler can show debug info.
         * It is disabled when the debug stream is null.
         */
        bool IsDebugEnabled() const;

        uint32_t GetInternalMaxVarStackOffset() const;
        uint32_t GetMaxExternalVarStackOffset() const;

    private:
        void CalculateProgLength(const sval_t& rootNode);
        size_t Preallocate(ProgramScript* script, sval_t rootNode, opval_t*& progBuffer);
        void EmitProgram(ProgramScript* script, sval_t rootNode, opval_t*& progBuffer, size_t progLength);

    private:
        StringDictionary& dict;
        const OutputInfo* info;
        int16_t m_iInternalMaxVarStackOffset;
        int16_t m_iMaxExternalVarStackOffset;
        bool compileSuccess;
    };

    namespace CompileErrors
    {
        class Base : public ScriptExceptionBase {};

        class StackOverflow : public Base
        {
        public:
            const char* what() const noexcept override;
        };
    }
};
