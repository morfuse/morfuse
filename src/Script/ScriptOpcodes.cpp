#include <morfuse/Script/ScriptOpcodes.h>
#include <morfuse/Common/ConstStr.h>
#include <morfuse/Common/short3.h>
#include <morfuse/Common/Vector.h>

namespace mfuse
{
	class StateScript;

static opcode_t OpcodeInfo[] =
{
		{ "OPCODE_EOF",								0,								0,			false },
		{ "OPCODE_BOOL_JUMP_FALSE4",				1 + sizeof(op_offset_t),		-1,			false },
		{ "OPCODE_BOOL_JUMP_TRUE4",					1 + sizeof(op_offset_t),		-1,			false },
		{ "OPCODE_VAR_JUMP_FALSE4",					1 + sizeof(op_offset_t),		-1,			false },
		{ "OPCODE_VAR_JUMP_TRUE4",					1 + sizeof(op_offset_t),		-1,			false },

		{ "OPCODE_BOOL_LOGICAL_AND",				1 + sizeof(op_offset_t),		-1,			false },
		{ "OPCODE_BOOL_LOGICAL_OR",					1 + sizeof(op_offset_t),		-1,			false },
		{ "OPCODE_VAR_LOGICAL_AND",					1 + sizeof(op_offset_t),		-1,			false },
		{ "OPCODE_VAR_LOGICAL_OR",					1 + sizeof(op_offset_t),		-1,			false },

		{ "OPCODE_BOOL_TO_VAR",						0,								0,			false },

		{ "OPCODE_JUMP4",							1 + sizeof(op_offset_t),		0,			false },
		{ "OPCODE_JUMP_BACK4",						1 + sizeof(op_offset_t),		0,			false },

		{ "OPCODE_STORE_INT0",						1,								1,			false },
		{ "OPCODE_STORE_INT1",						1 + sizeof(int8_t),				1,			false },
		{ "OPCODE_STORE_INT2",						1 + sizeof(int16_t),			1,			false },
		{ "OPCODE_STORE_INT3",						1 + sizeof(short3),				1,			false },
		{ "OPCODE_STORE_INT4",						1 + sizeof(int32_t),			1,			false },
		{ "OPCODE_STORE_INT4",						1 + sizeof(int64_t),			1,			false },

		{ "OPCODE_BOOL_STORE_FALSE",				1,								1,			false },
		{ "OPCODE_BOOL_STORE_TRUE",					1,								1,			false },

		{ "OPCODE_STORE_STRING",					1 + sizeof(op_name_t),			1,			false },
		{ "OPCODE_STORE_FLOAT",						1 + sizeof(float),				1,			false },
		{ "OPCODE_STORE_VECTOR",					1 + sizeof(Vector),				1,			false },
		{ "OPCODE_CALC_VECTOR",						1,								-2,			false },
		{ "OPCODE_STORE_NULL",						1,								1,			false },
		{ "OPCODE_STORE_NIL",						1,								1,			false },

		{ "OPCODE_EXEC_CMD0",						1 + sizeof(op_ev_t),					0,			true },
		{ "OPCODE_EXEC_CMD1",						1 + sizeof(op_ev_t),					-1,			true },
		{ "OPCODE_EXEC_CMD2",						1 + sizeof(op_ev_t),					-2,			true },
		{ "OPCODE_EXEC_CMD3",						1 + sizeof(op_ev_t),					-3,			true },
		{ "OPCODE_EXEC_CMD4",						1 + sizeof(op_ev_t),					-4,			true },
		{ "OPCODE_EXEC_CMD5",						1 + sizeof(op_ev_t),					-5,			true },
		{ "OPCODE_EXEC_CMD_COUNT1",					1 + sizeof(op_ev_t) + sizeof(op_parmNum_t),	-128,		true },

		{ "OPCODE_EXEC_CMD_METHOD0",				1 + sizeof(op_ev_t),					-1,			true },
		{ "OPCODE_EXEC_CMD_METHOD1",				1 + sizeof(op_ev_t),					-2,			true },
		{ "OPCODE_EXEC_CMD_METHOD2",				1 + sizeof(op_ev_t),					-3,			true },
		{ "OPCODE_EXEC_CMD_METHOD3",				1 + sizeof(op_ev_t),					-4,			true },
		{ "OPCODE_EXEC_CMD_METHOD4",				1 + sizeof(op_ev_t),					-5,			true },
		{ "OPCODE_EXEC_CMD_METHOD5",				1 + sizeof(op_ev_t),					-6,			true },
		{ "OPCODE_EXEC_CMD_METHOD_COUNT1",			1 + sizeof(op_ev_t) + sizeof(op_parmNum_t),	-128,		true },

		{ "OPCODE_EXEC_METHOD0",					1 + sizeof(op_ev_t),						0,			true },
		{ "OPCODE_EXEC_METHOD1",					1 + sizeof(op_ev_t),						-1,			true },
		{ "OPCODE_EXEC_METHOD2",					1 + sizeof(op_ev_t),						-2,			true },
		{ "OPCODE_EXEC_METHOD3",					1 + sizeof(op_ev_t),						-3,			true },
		{ "OPCODE_EXEC_METHOD4",					1 + sizeof(op_ev_t),						-4,			true },
		{ "OPCODE_EXEC_METHOD5",					1 + sizeof(op_ev_t),						-5,			true },
		{ "OPCODE_EXEC_METHOD_COUNT1",				1 + sizeof(op_ev_t) + sizeof(op_parmNum_t),	-128,		true },

		{ "OPCODE_LOAD_GAME_VAR",					1 + sizeof(op_name_t) + sizeof(op_evName_t),		-1,			false },
		{ "OPCODE_LOAD_LEVEL_VAR",					1 + sizeof(op_name_t) + sizeof(op_evName_t),		-1,			false },
		{ "OPCODE_LOAD_LOCAL_VAR",					1 + sizeof(op_name_t) + sizeof(op_evName_t),		-1,			false },
		{ "OPCODE_LOAD_PARM_VAR",					1 + sizeof(op_name_t) + sizeof(op_evName_t),		-1,			false },
		{ "OPCODE_LOAD_SELF_VAR",					1 + sizeof(op_name_t) + sizeof(op_evName_t),		-1,			false },
		{ "OPCODE_LOAD_GROUP_VAR",					1 + sizeof(op_name_t) + sizeof(op_evName_t),		-1,			false },
		{ "OPCODE_LOAD_OWNER_VAR",					1 + sizeof(op_name_t) + sizeof(op_evName_t),		-1,			false },
		{ "OPCODE_LOAD_FIELD_VAR",					1 + sizeof(op_name_t) + sizeof(op_evName_t),		-2,			false },
		{ "OPCODE_LOAD_ARRAY_VAR",					1,								-3,			false },
		{ "OPCODE_LOAD_CONST_ARRAY1",				1 + sizeof(op_arrayParmNum_t),	-128,		false },

		{ "OPCODE_STORE_FIELD_REF",					1 + sizeof(op_name_t),			0,			false },
		{ "OPCODE_STORE_ARRAY_REF",					1,								-1,			false },

		{ "OPCODE_MARK_STACK_POS",					1,								0,			false },

		{ "OPCODE_STORE_PARAM",						1,								1,			false },

		{ "OPCODE_RESTORE_STACK_POS",				1,								0,			false },

		{ "OPCODE_LOAD_STORE_GAME_VAR",				1 + sizeof(op_name_t) + sizeof(op_evName_t),			0,			false },
		{ "OPCODE_LOAD_STORE_LEVEL_VAR",			1 + sizeof(op_name_t) + sizeof(op_evName_t),			0,			false },
		{ "OPCODE_LOAD_STORE_LOCAL_VAR",			1 + sizeof(op_name_t) + sizeof(op_evName_t),			0,			false },
		{ "OPCODE_LOAD_STORE_PARM_VAR",				1 + sizeof(op_name_t) + sizeof(op_evName_t),			0,			false },
		{ "OPCODE_LOAD_STORE_SELF_VAR",				1 + sizeof(op_name_t) + sizeof(op_evName_t),			0,			false },
		{ "OPCODE_LOAD_STORE_GROUP_VAR",			1 + sizeof(op_name_t) + sizeof(op_evName_t),			0,			false },
		{ "OPCODE_LOAD_STORE_OWNER_VAR",			1 + sizeof(op_name_t) + sizeof(op_evName_t),			0,			false },

		{ "OPCODE_STORE_GAME_VAR",					1 + sizeof(op_name_t) + sizeof(op_evName_t),			1,			false },
		{ "OPCODE_STORE_LEVEL_VAR",					1 + sizeof(op_name_t) + sizeof(op_evName_t),			1,			false },
		{ "OPCODE_STORE_LOCAL_VAR",					1 + sizeof(op_name_t) + sizeof(op_evName_t),			1,			false },
		{ "OPCODE_STORE_PARM_VAR",					1 + sizeof(op_name_t) + sizeof(op_evName_t),			1,			false },
		{ "OPCODE_STORE_SELF_VAR",					1 + sizeof(op_name_t) + sizeof(op_evName_t),			1,			false },
		{ "OPCODE_STORE_GROUP_VAR",					1 + sizeof(op_name_t) + sizeof(op_evName_t),			1,			false },
		{ "OPCODE_STORE_OWNER_VAR",					1 + sizeof(op_name_t) + sizeof(op_evName_t),			1,			false },
		{ "OPCODE_STORE_FIELD",						1 + sizeof(op_name_t) + sizeof(op_evName_t),			0,			false },
		{ "OPCODE_STORE_ARRAY",						1,								-1,			false },
		{ "OPCODE_STORE_GAME",						1,								1,			false },
		{ "OPCODE_STORE_LEVEL",						1,								1,			false },
		{ "OPCODE_STORE_LOCAL",						1,								1,			false },
		{ "OPCODE_STORE_PARM",						1,								1,			false },
		{ "OPCODE_STORE_SELF",						1,								1,			false },
		{ "OPCODE_STORE_GROUP",						1,								1,			false },
		{ "OPCODE_STORE_OWNER",						1,								1,			false },

		{ "OPCODE_BIN_BITWISE_AND",					1,								-1,			false },
		{ "OPCODE_BIN_BITWISE_OR",					1,								-1,			false },
		{ "OPCODE_BIN_BITWISE_EXCL_OR",				1,								-1,			false },
		{ "OPCODE_BIN_EQUALITY",					1,								-1,			false },
		{ "OPCODE_BIN_INEQUALITY",					1,								-1,			false },
		{ "OPCODE_BIN_LESS_THAN",					1,								-1,			false },
		{ "OPCODE_BIN_GREATER_THAN",				1,								-1,			false },
		{ "OPCODE_BIN_LESS_THAN_OR_EQUAL",			1,								-1,			false },
		{ "OPCODE_BIN_GREATER_THAN_OR_EQUAL",		1,								-1,			false },
		{ "OPCODE_BIN_PLUS",						1,								-1,			false },
		{ "OPCODE_BIN_MINUS",						1,								-1,			false },
		{ "OPCODE_BIN_MULTIPLY",					1,								-1,			false },
		{ "OPCODE_BIN_DIVIDE",						1,								-1,			false },
		{ "OPCODE_BIN_PERCENTAGE",					1,								-1,			false },

		{ "OPCODE_UN_MINUS",						1,								0,			false },
		{ "OPCODE_UN_COMPLEMENT",					1,								0,			false },
		{ "OPCODE_UN_TARGETNAME",					1,								0,			false },
		{ "OPCODE_BOOL_UN_NOT",						1,								0,			false },
		{ "OPCODE_VAR_UN_NOT",						1,								0,			false },
		{ "OPCODE_UN_CAST_BOOLEAN",					1,								0,			false },
		{ "OPCODE_UN_INC",							1,								0,			false },
		{ "OPCODE_UN_DEC",							1,								0,			false },
		{ "OPCODE_UN_SIZE",							1,								0,			false },

		{ "OPCODE_SWITCH",							1 + sizeof(StateScript*),		-1,			false },

		{ "OPCODE_FUNC",							11,								-128,		true },

		{ "OPCODE_NOP",								1,								0,			false },

		{ "OPCODE_BIN_SHIFT_LEFT",					1,								-1,			false },
		{ "OPCODE_BIN_SHIFT_RIGHT",					1,								-1,			false },

		{ "OPCODE_END",								1,								-1,			false },
		{ "OPCODE_RETURN",							1,								-1,			false },
};

static const char *aszVarGroupNames[] =
{
	"game",
	"level",
	"local",
	"parm",
	"self"
};
};

const char* mfuse::VarGroupName(uint8_t iVarGroup)
{
	return aszVarGroupNames[iVarGroup];
}

const char* mfuse::OpcodeName(opval_t opcode)
{
	return OpcodeInfo[opcode].opcodename;
}

size_t mfuse::OpcodeLength(opval_t opcode)
{
	return OpcodeInfo[opcode].opcodelength;
}

int mfuse::OpcodeVarStackOffset(opval_t opcode)
{
	return OpcodeInfo[opcode].opcodestackoffset;
}

bool mfuse::IsExternalOpcode(opval_t opcode)
{
	return OpcodeInfo[opcode].isexternal;
}