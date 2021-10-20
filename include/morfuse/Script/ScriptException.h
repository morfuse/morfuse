#pragma once

#include "../Global.h"
#include "../Common/str.h"

#include <exception>
#include <utility>

namespace mfuse
{
	class ScriptExceptionBase : public std::exception
	{
	};

	class ScriptAbortExceptionBase : public std::exception
	{
	};

	class Messageable
	{
	public:
		void fill(const xstr& msg) const;
		bool filled() const noexcept;

	protected:
		const char* what() const noexcept;

	private:
		xstr msg;
	};

	class ScriptException : public ScriptExceptionBase
	{
	public:
		ScriptException(const xstr& text);

		const char* what() const noexcept override;

	public:
		xstr string;
	};

	class ScriptAbortException : public ScriptAbortExceptionBase
	{
	public:
		ScriptAbortException(const xstr& text);

		const char* what() const noexcept override;

	public:
		xstr string;
	};

	/*
	template<typename...Args>
	static constexpr void ScriptError(Args&&...args)
	{
		throw ScriptException(std::forward<Args>(args)...);
	}
	*/
};
