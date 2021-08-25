#pragma once

#include "../Common/str.h"

#include <utility>

namespace mfuse
{
	class ScriptException
	{
	public:
		ScriptException(const xstr& text);
		ScriptException(const rawchar_t* format, ...);

	public:
		xstr string;
	};

	class ScriptAbortException : public ScriptException
	{
	public:
		ScriptAbortException(const xstr& text);
		ScriptAbortException(const rawchar_t* text);
	};

	template<typename...Args>
	static constexpr void ScriptError(Args&&...args)
	{
		throw ScriptException(std::forward<Args>(args)...);
	}
};
