#include "parsetree.h"

#include <morfuse/Common/MEM/TempAlloc.h>
#include <morfuse/Common/ConstStr.h>
#include <morfuse/Script/ScriptOpcodes.h>
#include <cstring>

namespace mfuse
{
	class StateScript;
}

using namespace mfuse;

static sval_t node_none{ statementType_e::None };

enum statement_alloc_e {
	none,
	label,
	stateScript,
	caseLabel,
	trycatch
};

struct statement_t {
	uint8_t oplen;
	bool allocString;
	statement_alloc_e allocationtype;
};

ParseTree::ParseTree()
{
}

const sval_t& ParseTree::getRootNode() const
{
	return rootVal;
}

const lengthInfo_t& ParseTree::lengthInfo() const
{
	return linfo;
}

sval_t ParseTree::node1_(int val1)
{
	sval_t val;

	val.intValue = val1;

	return val;
}

sval_t ParseTree::node1b(int val1)
{
	sval_t val;

	val.byteValue = val1;

	return val;
}

mfuse::sval_t ParseTree::node_pos(location loc)
{
	sval_t val;

	val.sourceLocValue = (sourceLocation_t*)alloc(sizeof(sourceLocation_t), alignof(sourceLocation_t));
	val.sourceLocValue->sourcePos = loc.sourcePos;
	val.sourceLocValue->line = loc.begin.line;
	val.sourceLocValue->column = loc.begin.column;

	return val;
}

sval_t ParseTree::node_string(const char* text)
{
	sval_t val;

	val.stringValue = text;

	return val;
}

sval_t ParseTree::node0(statementType_e type)
{
	if (type == statementType_e::None)
	{
		// memory optimization
		return &node_none;
	}
	else
	{
		sval_t* const node = (sval_t*)alloc(sizeof(sval_t) * 1, alignof(sval_t));
		node->type = type;
		return node;
	}
}

sval_t ParseTree::node1(statementType_e type, sval_t val1)
{
	sval_t* const node = (sval_t*)alloc(sizeof(sval_t) * 2, alignof(sval_t));

	node[0].type = type;
	node[1] = val1;

	return node;
}

sval_t ParseTree::node2(statementType_e type, sval_t val1, sval_t val2)
{
	sval_t* const node = (sval_t*)alloc(sizeof(sval_t) * 3, alignof(sval_t));

	node[0].type = type;
	node[1] = val1;
	node[2] = val2;

	return node;
}

sval_t ParseTree::node3(statementType_e type, sval_t val1, sval_t val2, sval_t val3)
{
	sval_t* const node = (sval_t*)alloc(sizeof(sval_t) * 4, alignof(sval_t));

	node[0].type = type;
	node[1] = val1;
	node[2] = val2;
	node[3] = val3;

	return node;
}

sval_t ParseTree::node4(statementType_e type, sval_t val1, sval_t val2, sval_t val3, sval_t val4)
{
	sval_t* const node = (sval_t*)alloc(sizeof(sval_t) * 5, alignof(sval_t));

	node[0].type = type;
	node[1] = val1;
	node[2] = val2;
	node[3] = val3;
	node[4] = val4;

	return node;
}

sval_t ParseTree::node5(statementType_e type, sval_t val1, sval_t val2, sval_t val3, sval_t val4, sval_t val5)
{
	sval_t* const node = (sval_t*)alloc(sizeof(sval_t) * 6, alignof(sval_t));

	node[0].type = type;
	node[1] = val1;
	node[2] = val2;
	node[3] = val3;
	node[4] = val4;
	node[5] = val5;

	return node;
}

sval_t ParseTree::node6(statementType_e type, sval_t val1, sval_t val2, sval_t val3, sval_t val4, sval_t val5, sval_t val6)
{
	sval_t* const node = (sval_t*)alloc(sizeof(sval_t) * 7, alignof(sval_t));

	node[0].type = type;
	node[1] = val1;
	node[2] = val2;
	node[3] = val3;
	node[4] = val4;
	node[5] = val5;
	node[6] = val6;

	return node;
}

sval_t ParseTree::linked_list_end(sval_t val)
{
	sval_t* const node = (sval_t*)alloc(sizeof(sval_t) * 2, alignof(sval_t));

	// set the prev value of the node
	// and also nullify the next value of the node
	node[0] = val;
	node[1].node = nullptr;
	sval_t* const end = (sval_t*)alloc(sizeof(sval_t) * 2, alignof(sval_t));
	end[0].node = node;
	end[1].node = node;

	/*
	linked_node_t* const node = (linked_node_t*)alloc(sizeof(linked_node_t));

	// set the prev value of the node
	// and also nullify the next value of the node
	node->prev = val.node;
	node->next = nullptr;
	linked_node_t* const end = (linked_node_t*)alloc(sizeof(linked_node_t));
	end->prev = sval_t(node).node;
	end->next = sval_t(node).node;
	*/

	return end;
}

sval_t ParseTree::prepend_node(sval_t val1, sval_t val2)
{
	sval_t* const node = (sval_t*)alloc(sizeof(sval_t) * 2, alignof(sval_t));

	// set the first and the second node
	node[0].node = val1.node;
	node[1].node = val2.node->node;
	val2.node->node = node;
	return node;
}

sval_t ParseTree::append_node(sval_t val1, sval_t val2)
{
	sval_t* const node = (sval_t*)alloc(sizeof(sval_t) * 2, alignof(sval_t));

	node[0].node = val2.node;
	node[1].node = nullptr;

	val1.node[1].node[1].node = node;
	val1.node[1].node = node;

	/*
	linked_node_t* const node = (linked_node_t*)alloc(sizeof(linked_node_t));

	node->prev = val2.node;
	node->next = nullptr;

	val1.linkednode->next->linkednode->next = (sval_t*)node;
	val1.linkednode->next = (sval_t*)node;
	*/

	return val1.node;
}

sval_t ParseTree::ParseTree::append_lists(sval_t val1, sval_t val2)
{
	val1.node[1].node[1] = val2.node[0];
	val1.node[1] = val2.node[1];

	return val1.node;
}

char* ParseTree::alloc(size_t s)
{
	return (char*)parsetree_allocator.Alloc(s);
}

char* ParseTree::alloc(size_t s, size_t alignment)
{
	return (char*)parsetree_allocator.Alloc(s, alignment);
}

void ParseTree::free(void* ptr)
{
	parsetree_allocator.Free(ptr);
}

void ParseTree::freeAll()
{
	parsetree_allocator.FreeAll();

	/*
	if( showopcodes->integer )
	{
		glbs.DPrintf( "%d bytes freed\n", parsedata.total_length );
	}
	*/
}

lengthInfo_t::lengthInfo_t()
	: total_length(0)
	, numStrings(0)
	, numStateScripts(0)
	, numLabels(0)
	, numCases(0)
	, numCatches(0)
{
	total_length = 0;
	numStrings = 0;
}
