#include <morfuse/Script/AbstractScript.h>
#include <morfuse/Script/ScriptMaster.h>
#include <morfuse/Script/Context.h>

using namespace mfuse;

AbstractScript::AbstractScript()
{
	m_ProgToSource = NULL;
	m_SourceBuffer = NULL;
	m_SourceLength = 0;
}

AbstractScript::~AbstractScript()
{
	if(m_ProgToSource) delete m_ProgToSource;
}

void AbstractScript::CreateProgToSource(size_t startLength)
{
	m_ProgToSource = new (allocator) sourcePosMap_t[startLength];
/*
	m_ProgToSource = new sourcePosMap_t;
	m_ProgToSource->getAllocator().SetAllocator(allocator);
	m_ProgToSource->getAllocator().SetAllocator(allocator);

	if(startLength) {
		m_ProgToSource->resize(startLength);
	}
*/
}

const xstr& AbstractScript::Filename() const
{
	return ScriptContext::Get().GetDirector().GetString(m_Filename);
}

const_str AbstractScript::ConstFilename() const
{
	return m_Filename;
}

void AbstractScript::AddSourcePos(uintptr_t pos, sourceLocation_t sourcePos)
{
	if (m_ProgToSource)
	{
		// simply add the location information
		//m_ProgToSource->addKeyValue(codePos) = sourcePos;
		m_ProgToSource[pos] = sourcePos;
	}
}
/*
bool AbstractScript::GetSourceAt(size_t sourcePos, xstr &sourceLine, uint32_t& column, uint32_t& line) const
{
	line = 1;
	column = 1;

	if (!m_SourceBuffer || sourcePos >= m_SourceLength)
	{
		// no source buffer or invalid source position
		return false;
	}

	uint32_t tmpColumn = 1;
	uint32_t tmpLine = 1;
	size_t posLine = 0;

	const rawchar_t *p = m_SourceBuffer;

	for (size_t i = 0; i < sourcePos && *p; i++, p++)
	{
		++tmpColumn;

		if (*p == '\n')
		{
			++tmpLine;
			tmpColumn = 1;
			posLine = i + 1;
		}
	}

	while (*p != '\0' && *p != '\n')
	{
		p++;
	}

	column = tmpColumn;
	line = tmpLine;

	const rawchar_t* lineStr = m_SourceBuffer + posLine;

	sourceLine = xstr(lineStr, p - lineStr);
	rawchar_t* stringValue = const_cast<rawchar_t*>(sourceLine.c_str());
	while(*stringValue)
	{
		// replace TAB by space
		if(*stringValue == '\t') {
			*stringValue = ' ';
		}

		++stringValue;
	}

	return true;
}
*/

bool AbstractScript::GetSourceAt(size_t pos, xstr& sourceLine, uint32_t& column, uint32_t& line) const
{
	line = 1;
	column = 1;

	if(!m_ProgToSource) {
		return false;
	}

	//const sourceLocation_t* location = m_ProgToSource->findKeyValue(sourcePos);
	const sourceLocation_t* location = &m_ProgToSource[pos];
	if(location)
	{
		column = location->column;
		line = location->line;
		return GetSourceAt(*location, sourceLine);
	}

	return false;
}

bool AbstractScript::GetSourceAt(const sourceLocation_t& sourceLoc, xstr& sourceLine) const
{
	return sourceLoc.getLine(m_SourceBuffer, m_SourceLength, sourceLine);
}

void AbstractScript::PrintSourcePos(const sourceLocation_t& sourceLoc) const
{
	xstr sourceLine;

	if (GetSourceAt(sourceLoc, sourceLine))
	{
		PrintSourcePos(sourceLine.c_str(), sourceLoc.column, sourceLoc.line);
	}
	else
	{
		ScriptContext::Get().GetOutputInfo().DPrintf("file '%s', source pos %d line %d column %d:\n", Filename().c_str(), sourceLoc.sourcePos, sourceLoc.line, sourceLoc.column);
	}
}

/*
void AbstractScript::PrintSourcePos(size_t sourcePos) const
{
	uint32_t line;
	uint32_t column;
	xstr sourceLine;

	if (GetSourceAt(sourcePos, sourceLine, column, line))
	{
		PrintSourcePos(sourceLine, column, line);
	}
	else
	{
		ScriptContext::Get().GetOutputInfo().DPrintf("file '%s', source pos %zd\n", Filename().c_str(), sourcePos);
	}
}
*/

void AbstractScript::PrintSourcePos(size_t pos) const
{
	if (!m_ProgToSource) {
		return;
	}

	//sourceLocation_t *sourceLoc = m_ProgToSource->findKeyValue(codePos);
	const sourceLocation_t* sourceLoc = &m_ProgToSource[pos];

	if (!sourceLoc) {
		return;
	}

	PrintSourcePos(*sourceLoc);
}

void AbstractScript::PrintSourcePos(const rawchar_t* sourceLine, uint32_t column, uint32_t line) const
{
	OutputInfo& outputInfo = ScriptContext::Get().GetOutputInfo();
	outputInfo.DPrintf("(%s, %d):\n%s\n", Filename().c_str(), line, sourceLine);
	outputInfo.DPrintf("%*.s^\n", column, " ");
}

const rawchar_t* AbstractScript::GetSourceBuffer() const
{
	return m_SourceBuffer;
}

uint64_t AbstractScript::GetSourceLength() const
{
	return m_SourceLength;
}

MEM::PreAllocator& AbstractScript::GetAllocator()
{
	return allocator;
}
