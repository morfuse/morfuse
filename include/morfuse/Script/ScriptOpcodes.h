#pragma once

#include <cstdint>
#include <cstddef>

namespace mfuse
{
    /** Should stay a 8-byte value. */
    using opval_t = uint8_t;
    /** Store an index in the string table. */
    using op_name_t = uint32_t;
    /** Event number. */
    using op_ev_t = uint32_t;
    /** Index in the event name table. */
    using op_evName_t = uint32_t;
    /** Jump offset. */
    using op_offset_t = uint32_t;
    /** Parameter count. */
    using op_parmNum_t = uint8_t;
    /** Parameter count of const array. */
    using op_arrayParmNum_t = uint16_t;

    struct opcode_t {
        const char *opcodename;
        uint32_t opcodelength;
        short opcodestackoffset;
        bool isexternal;
    };

    struct opcode_info_t {
        opval_t opcode;
        int8_t VarStackOffset;
    };

    enum opcode_e
    {
        OP_DONE,
        OP_BOOL_JUMP_FALSE4,
        OP_BOOL_JUMP_TRUE4,
        OP_VAR_JUMP_FALSE4,
        OP_VAR_JUMP_TRUE4,

        OP_BOOL_LOGICAL_AND,
        OP_BOOL_LOGICAL_OR,
        OP_VAR_LOGICAL_AND,
        OP_VAR_LOGICAL_OR,

        OP_BOOL_TO_VAR,

        OP_JUMP4,
        OP_JUMP_BACK4,

        OP_STORE_INT0,
        OP_STORE_INT1,
        OP_STORE_INT2,
        OP_STORE_INT3,
        OP_STORE_INT4,
        OP_STORE_INT8,

        OP_BOOL_STORE_FALSE,
        OP_BOOL_STORE_TRUE,

        OP_STORE_STRING,
        OP_STORE_FLOAT,
        OP_STORE_VECTOR,
        OP_CALC_VECTOR,
        OP_STORE_NULL,
        OP_STORE_NIL,

        /** Exec script thread command. */
        OP_EXEC_CMD0,
        OP_EXEC_CMD1,
        OP_EXEC_CMD2,
        OP_EXEC_CMD3,
        OP_EXEC_CMD4,
        OP_EXEC_CMD5,
        OP_EXEC_CMD_COUNT1,

        /** Exec normal listener command. */
        OP_EXEC_CMD_METHOD0,
        OP_EXEC_CMD_METHOD1,
        OP_EXEC_CMD_METHOD2,
        OP_EXEC_CMD_METHOD3,
        OP_EXEC_CMD_METHOD4,
        OP_EXEC_CMD_METHOD5,
        OP_EXEC_CMD_METHOD_COUNT1,

        /** Exec listener command with return. */
        OP_EXEC_METHOD0,
        OP_EXEC_METHOD1,
        OP_EXEC_METHOD2,
        OP_EXEC_METHOD3,
        OP_EXEC_METHOD4,
        OP_EXEC_METHOD5,
        OP_EXEC_METHOD_COUNT1,

        OP_LOAD_GAME_VAR,
        OP_LOAD_LEVEL_VAR,
        OP_LOAD_LOCAL_VAR,
        OP_LOAD_PARM_VAR,
        OP_LOAD_SELF_VAR,
        OP_LOAD_GROUP_VAR,
        OP_LOAD_OWNER_VAR,
        OP_LOAD_FIELD_VAR,
        OP_LOAD_ARRAY_VAR,
        OP_LOAD_CONST_ARRAY1,

        OP_STORE_FIELD_REF,
        OP_STORE_ARRAY_REF,

        OP_MARK_STACK_POS,

        OP_STORE_PARAM,

        OP_RESTORE_STACK_POS,

        OP_LOAD_STORE_GAME_VAR,
        OP_LOAD_STORE_LEVEL_VAR,
        OP_LOAD_STORE_LOCAL_VAR,
        OP_LOAD_STORE_PARM_VAR,
        OP_LOAD_STORE_SELF_VAR,
        OP_LOAD_STORE_GROUP_VAR,
        OP_LOAD_STORE_OWNER_VAR,

        OP_STORE_GAME_VAR,
        OP_STORE_LEVEL_VAR,
        OP_STORE_LOCAL_VAR,
        OP_STORE_PARM_VAR,
        OP_STORE_SELF_VAR,
        OP_STORE_GROUP_VAR,
        OP_STORE_OWNER_VAR,
        OP_STORE_FIELD,
        OP_STORE_ARRAY,
        OP_STORE_GAME,
        OP_STORE_LEVEL,
        OP_STORE_LOCAL,
        OP_STORE_PARM,
        OP_STORE_SELF,
        OP_STORE_GROUP,
        OP_STORE_OWNER,

        OP_BIN_BITWISE_AND,
        OP_BIN_BITWISE_OR,
        OP_BIN_BITWISE_EXCL_OR,
        OP_BIN_EQUALITY,
        OP_BIN_INEQUALITY,
        OP_BIN_LESS_THAN,
        OP_BIN_GREATER_THAN,
        OP_BIN_LESS_THAN_OR_EQUAL,
        OP_BIN_GREATER_THAN_OR_EQUAL,
        OP_BIN_PLUS,
        OP_BIN_MINUS,
        OP_BIN_MULTIPLY,
        OP_BIN_DIVIDE,
        OP_BIN_PERCENTAGE,

        OP_UN_MINUS,
        OP_UN_COMPLEMENT,
        OP_UN_TARGETNAME,
        OP_BOOL_UN_NOT,
        OP_VAR_UN_NOT,
        OP_UN_CAST_BOOLEAN,
        OP_UN_INC,
        OP_UN_DEC,
        OP_UN_SIZE,

        OP_SWITCH,

        OP_FUNC,

        OP_NOP,

        OP_BIN_SHIFT_LEFT,
        OP_BIN_SHIFT_RIGHT,

        OP_END,
        OP_RETURN,

        OP_PREVIOUS,
        OP_MAX
    };

    /*
    struct programTable_t
    {
        opval_t* opcodes;
        offset_t* jumpTable;
        uint8_t* i8Table;
        uint16_t* i16Table;
        short3* i24Table;
        uint32_t* i32Table;
        uint64_t* i64Table;
        float* fltTable;
        op_ev_t* evTable;
        op_parmNum_t* parmTable;
    };
    */
    const char* VarGroupName(uint8_t iVarGroup);
    const char* OpcodeName(opval_t opcode);
    size_t OpcodeLength(opval_t opcode);
    int OpcodeVarStackOffset(opval_t opcode);
    bool IsExternalOpcode(opval_t opcode);
};
