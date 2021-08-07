#pragma once

#include <morfuse/Common/str.h>
#include <morfuse/Common/MEM/TempAlloc.h>
#include <morfuse/Script/sourcePos.h>
#include "Location.hpp"

namespace mfuse
{
	struct yyparsedata;
	struct linked_node_t;
	/** The char type used by the parser. */
	using prchar_t = char;

	enum builtinType_e
	{
		method_game,
		method_level,
		method_local,
		method_parm,
		method_self,
		method_group,
		method_owner,
		method_field,
		method_array,
	};

	enum class statementType_e
	{
		None,
		Next,
		StatementList,
		Labeled,
		IntLabeled,
		NegIntLabeled,
		Assignment,
		If,
		IfElse,
		While,
		LogicalAnd,
		LogicalOr,
		MethodEvent,
		MethodEventExpr,
		CmdEvent,
		CmdEventExpr,
		Field,
		Listener,
		String,
		Integer,
		Float,
		Vector,
		NULLPTR,
		NIL,
		Func1Expr,
		Func2Expr,
		BoolNot,
		ArrayExpr,
		ConstArrayExpr,
		MakeArray,
		Try,
		Switch,
		Break,
		Continue,
		Do,
		PrivateLabeled,
		Define,
		max
	};

	/**
	 * Using C-union instead of C++ classes because :
	 * 1) it occupies less memory (vftable occupies at least pointer-size bytes)
	 * 2) no new allocation needed, it's all on the parsetree allocator
	 * 3) it's faster because it relies on switch-case only
	 */
	union sval_u {
		statementType_e	type;
		const char*			stringValue;
		float				floatValue;
		uint32_t			intValue;
		uint64_t			longValue;
		char				charValue;
		unsigned char		byteValue;
		unsigned char*		posValue;
		sval_u*				node;
		linked_node_t*		linkednode;
		sourceLocation_t*	sourceLocValue;

		sval_u() : longValue(0) {}
		sval_u(std::nullptr_t) : node(0) {}
		sval_u(statementType_e in) : type(in) {}
		sval_u(const char* in) : stringValue(in) {}
		sval_u(uint32_t in) : intValue(in) {}
		sval_u(uint64_t in) : longValue(in) {}
		sval_u(float in) : floatValue(in) {}
		sval_u(char in) : charValue(in) {}
		sval_u(sval_u* in) : node(in) {}
		sval_u(linked_node_t* in) : linkednode(in) {}
	};
	using sval_t = sval_u;

	struct linked_node_t {
		sval_t* prev;
		sval_t* next;
	};

	struct lengthInfo_t {
		size_t total_length;
		size_t numStrings;
		size_t numStateScripts;
		size_t numLabels;
		size_t numCases;
		size_t numCatches;
		size_t currentSwitch;

	public:
		lengthInfo_t();

		void process(statementType_e type);
	};

	struct yyexception {
		int			lineNumber;
		str			text;
		str			token;

		yyexception() { lineNumber = 0; }
	};

	class ParseTree
	{
	public:
		ParseTree();
		ParseTree(const ParseTree& other) = delete;
		ParseTree& operator=(const ParseTree& other) = delete;
		ParseTree(ParseTree&& other) = default;
		ParseTree& operator=(ParseTree&& other) = default;

		const sval_t& getRootNode() const;

		void		freeAll();
		const lengthInfo_t& lengthInfo() const;
		char*		alloc(size_t s);
		void		free(void* ptr);

		int			node_compare(void* pe1, void* pe2);

		sval_t		append_lists(sval_t val1, sval_t val2);
		sval_t		append_node(sval_t val1, sval_t val2);
		sval_t		prepend_node(sval_t val1, sval_t val2);

		sval_t		linked_list_end(sval_t val);

		sval_t		node1_(int val1);
		sval_t		node1b(int val1);
		sval_t		node_pos(location loc);
		sval_t		node_string(const char* text);

		sval_t		node0(statementType_e type);
		sval_t		node1(statementType_e type, sval_t val1);
		sval_t		node2(statementType_e type, sval_t val1, sval_t val2);
		sval_t		node3(statementType_e type, sval_t val1, sval_t val2, sval_t val3);
		sval_t		node4(statementType_e type, sval_t val1, sval_t val2, sval_t val3, sval_t val4);
		sval_t		node5(statementType_e type, sval_t val1, sval_t val2, sval_t val3, sval_t val4, sval_t val5);
		sval_t		node6(statementType_e type, sval_t val1, sval_t val2, sval_t val3, sval_t val4, sval_t val5, sval_t val6);

	public:
		MEM::TempAlloc parsetree_allocator;

		lengthInfo_t linfo;
		sval_t rootVal;
	};
};

