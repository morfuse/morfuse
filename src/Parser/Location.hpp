#pragma once

#include <ostream>

namespace mfuse
{
	class position
	{
	public:
		position()
			: line(1)
			, column(0)
		{}

	public:
		uint32_t line;
		uint16_t column;
	};
	class location
	{
	public:
		location()
			: sourcePos(0)
			, lineSourcePos(0)
			, lineno(0)
		{}
	public:
		uint32_t sourcePos;
		uint32_t lineSourcePos;
		uint32_t lineno;
		position begin;
		position end;
	};

	template <typename YYChar>
	std::basic_ostream<YYChar>&
		operator<< (std::basic_ostream<YYChar>& ostr, const location&)
	{
		return ostr;
	}
}