#include "helpers/test.h"
#include "helpers/assert.h"

using namespace mfuse;

const char scriptData[] =
"main:\n"
"local.haystack = \"GCATCGCAGAGAGTATACAGTACG\"\n"
"local.needle = \"GCAGAGAG\"\n"
"local.result = \"\"\n"
"println(\">>>>>>>>>>>>>>>>>>>>>>>> Start Testing KMPstrstr\")\n"
"local.result = waitthread KMPstrstr local.haystack local.needle\n"
"if !(local.result) {}\n"
"else {\n"
"    println(\">>>>>>>>>>>>>>>>>>>>>>>> Result is: \" + local.result)\n"
"}\n"
"println(\">>>>>>>>>>>>>>>>>>>>>>>> End Testing KMPstrstr\")\n"
"end\n"
"KMPstrstr local.string local.sub:\n"
"local.strlen = local.string.size\n"
"local.sublen = local.sub.size\n"
"local.result = NIL\n"
"local.dfa[0] = -1\n"
"local.subptr = 0\n"
"local.dfaptr = -1\n"
"while (local.subptr < local.sublen) {\n"
"    while (local.dfaptr > -1 && local.sub[local.subptr] != local.sub[local.dfaptr]) {\n"
"        local.dfaptr = local.dfa[local.dfaptr]\n"
"    }\n"
"    local.subptr++\n"
"        local.dfaptr++\n"
"        if (local.dfa[local.subptr] == local.dfa[local.dfaptr]) {\n"
"            local.dfa[local.subptr] = local.dfa[local.dfaptr]\n"
"        }\n"
"        else {\n"
"            local.dfa[local.subptr] = local.dfaptr\n"
"        }\n"
"}\n"
"local.strptr = 0\n"
"local.subptr = 0\n"
"while (local.strptr < local.strlen) {\n"
"    while (local.subptr > -1 && local.sub[local.subptr] != local.string[local.strptr]) {\n"
"        local.subptr = local.dfa[local.subptr]\n"
"    }\n"
"    local.subptr++\n"
"        local.strptr++\n"
"        if (local.subptr >= local.sublen) {\n"
"            if (local.result == NIL) {\n"
"                local.result = local.strptr - local.subptr\n"
"            }\n"
"            else {\n"
"                local.result = local.result::(local.strptr - local.subptr)\n"
"            }\n"
"            local.subptr = local.dfa[local.subptr]\n"
"        }\n"
"}\n"
"end(local.result)";

int main()
{
	ScriptContext context;
	setupContext(context);

	ScriptMaster& director = context.GetDirector();

	const ProgramScript* const script = compile(context.GetDirector(), "kmp", scriptData);
	director.ExecuteThread(script);

	return 0;
}
