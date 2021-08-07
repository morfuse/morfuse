#pragma once

namespace mfuse
{
	class EventSystem;
	class ScriptContext;
	class ScriptMaster;
	class VMOperation;

	using opMethod_t = void (VMOperation::*)();

	struct opMap_t {
		opcode_e op;
		opMethod_t method;

		constexpr opMap_t(opcode_e opValue, opMethod_t methodValue)
			: op(opValue)
			, method(methodValue)
		{}
	};

	template<size_t numOps, size_t numMap>
	class opList_c
	{
	public:
		constexpr opList_c(const opMap_t (&opMap)[numMap])
			: method{}
		{
			// map function by opcode index
			for (size_t i = 0; i < numMap; ++i)
			{
				const opMap_t& map = opMap[i];
				const uintptr_t opNum = map.op;
				method[opNum] = map.method;
			}
		}

		constexpr opMethod_t operator[](opval_t index) const
		{
			return method[index];
		}

	private:
		opMethod_t method[numOps];
	};

	class VMOperation
	{
	public:
		VMOperation(ScriptVM& vmParent, ScriptContext& contextValue, ScriptMaster& directorValue, EventSystem& eventSystemValue);

		void Handle(opval_t op);

	public:

		void executeCommand(Listener* listener, uint16_t iParamCount, int eventnum, bool bMethod = false, bool bReturn = false);
		bool executeGetter(Listener* listener, op_evName_t eventName);
		bool executeSetter(Listener* listener, op_evName_t eventName);

		void jump(op_offset_t offset);
		void jumpBool(op_offset_t offset, bool value);

		template<bool noTop = false>
		void loadTop(Listener* listener);

		template<bool noTop = false>
		void storeTop(Listener* listener);

		void Exec_Done();
		void Exec_BinAnd();
		void Exec_BinOr();
		void Exec_BinExclOr();
		void Exec_BinEquality();
		void Exec_BinInequality();
		void Exec_BinGreater();
		void Exec_BinGreaterEq();
		void Exec_BinLess();
		void Exe_BinLessEq();
		void Exec_BinPlus();
		void Exec_BinMinus();
		void Exec_BinMult();
		void Exec_BinDiv();
		void Exec_BinMod();
		void Exec_BinShl();
		void Exec_BinShr();
		void Exec_BoolJumpFalse();
		void Exec_BoolJumpTrue();
		void Exec_VarJumpFalse();
		void Exec_VarJumpTrue();
		void Exec_BoolLogicalAnd();
		void Exec_BoolLogicalOr();
		void Exec_VarLogicalAnd();
		void Exec_VarLogicalOr();
		void Exec_BoolStoreFalse();
		void Exec_BoolStoreTrue();
		void Exec_BoolUnNot();
		void Exec_CalcVec();
		void Exec_Cmd0();
		void Exec_Cmd1();
		void Exec_Cmd2();
		void Exec_Cmd3();
		void Exec_Cmd4();
		void Exec_Cmd5();
		void Exec_CmdCount();
		void Exec_CmdCommon(size_t param);
		void Exec_CmdMethod0();
		void Exec_CmdMethod1();
		void Exec_CmdMethod2();
		void Exec_CmdMethod3();
		void Exec_CmdMethod4();
		void Exec_CmdMethod5();
		void Exec_CmdMethodCount();
		void Exec_CmdMethodCommon(size_t param);
		void Exec_Method0();
		void Exec_Method1();
		void Exec_Method2();
		void Exec_Method3();
		void Exec_Method4();
		void Exec_Method5();
		void Exec_MethodCount();
		void Exec_MethodCommon(size_t param);
		void Exec_Func();
		void Exec_Jump4();
		void Exec_JumpBack4();
		void Exec_LoadArrayVar();
		void Exec_LoadFieldVar();
		void Exec_LoadConstArray();
		void Exec_LoadGameVar();
		void Exec_LoadGroupVar();
		void Exec_LoadLevelVar();
		void Exec_LoadLocalVar();
		void Exec_LoadOwnerVar();
		void Exec_LoadParmVar();
		void Exec_LoadSelfVar();
		void Exec_LoadStoreGameVar();
		void Exec_LoadStoreGroupVar();
		void Exec_LoadStoreLevelVar();
		void Exec_LoadStoreLocalVar();
		void Exec_LoadStoreOwnerVar();
		void Exec_LoadStoreParmVar();
		void Exec_LoadStoreSelfVar();
		void Exec_MarkStackPos();
		void Exec_StoreParam();
		void Exec_RestoreStackPos();
		void Exec_StoreArray();
		void Exec_StoreArrayRef();
		void Exec_StoreFieldRef();
		void Exec_StoreField();
		void Exec_StoreFloat();
		void Exec_StoreInt0();
		void Exec_StoreInt1();
		void Exec_StoreInt2();
		void Exec_StoreInt3();
		void Exec_StoreInt4();
		void Exec_StoreInt8();
		void Exec_StoreGameVar();
		void Exec_StoreGroupVar();
		void Exec_StoreLevelVar();
		void Exec_StoreLocalVar();
		void Exec_StoreOwnerVar();
		void Exec_StoreParmVar();
		void Exec_StoreSelfVar();
		void Exec_StoreGame();
		void Exec_StoreGroup();
		void Exec_StoreLevel();
		void Exec_StoreLocal();
		void Exec_StoreOwner();
		void Exec_StoreParm();
		void Exec_StoreSelf();
		void Exec_StoreNil();
		void Exec_StoreNull();
		void Exec_StoreString();
		void Exec_StoreVector();
		void Exec_Switch();
		void Exec_UnCastBoolean();
		void Exec_UnComplement();
		void Exec_UnMinus();
		void Exec_UnDec();
		void Exec_UnInc();
		void Exec_UnSize();
		void Exec_UnTargetname();
		void Exec_VarUnNot();
		void Exec_Nop();

	private:
		ScriptClass* GetScriptClass() const;
		ScriptThread* GetThread() const;

	private:
		ScriptVM& vm;

	public:
		ScriptContext& context;
		ScriptMaster& director;
		EventSystem& eventSystem;
	};
}
