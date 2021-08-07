#pragma once

#include "../Common/str.h"

namespace mfuse
{
	class ScriptException
	{
	public:
		xstr string;
		int bAbort;
		int bIsForAnim;

	private:
		void CreateException(const rawchar_t*data);

	public:
		ScriptException(const xstr& text);
		ScriptException(const rawchar_t* format, ...);

		static int next_abort;
		static int next_bIsForAnim;
	};

	void Error(const char * format, ...);

#define ScriptDeprecated( function ) throw ScriptException( function ": DEPRECATED. DON'T USE IT ANYMORE" )
#define ScriptError throw ScriptException
};
