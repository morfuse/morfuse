#include <morfuse/Script/sourcePos.h>
#include <morfuse/Common/str.h>

#include <iomanip>

using namespace mfuse;

sourceLocation_t::sourceLocation_t()
	: sourcePos(0)
	, line(1)
	, column(1)
{};

sourceLocation_t::sourceLocation_t(uint8_t val)
{
	if (val == -1)
	{
		sourcePos = -1;
		line = -1;
		column = 0;
	}
}

bool sourceLocation_t::getLine(const rawchar_t* sourceBuffer, size_t sourceLength, str& sourceLine) const
{
	const rawchar_t* lineStr = sourceBuffer + sourcePos - column;

	if (lineStr < sourceBuffer || lineStr >= sourceBuffer + sourceLength)
	{
		// prevent overflow
		return false;
	}

	const rawchar_t* p = lineStr;

	while (*p && *p != '\n') {
		++p;
	}

	sourceLine = str(lineStr, p - lineStr);
	rawchar_t* stringValue = const_cast<rawchar_t*>(sourceLine.c_str());
	while (*stringValue)
	{
		// replace TAB by space
		if (*stringValue == '\t') {
			*stringValue = ' ';
		}

		++stringValue;
	}

	return true;
}

void mfuse::printSourcePos(const sourceLocation_t& sourcePos, const rawchar_t* scriptName, const rawchar_t* sourceBuffer, size_t sourceLength, std::ostream& out)
{
	str sourceLine;
	if(sourcePos.getLine(sourceBuffer, sourceLength, sourceLine))
	{
		out << "(" << scriptName << ", " << sourcePos.line << "):\n";
		out << sourceLine.c_str() << std::endl;
		out << std::setw(sourcePos.column + 2) << std::setfill(' ')
			<< "^\n"
			<< std::setw(0);
	}
}
