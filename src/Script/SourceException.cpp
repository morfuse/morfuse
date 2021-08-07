#include <morfuse/Script/SourceException.h>

using namespace mfuse;

ParseException::ParseError::ParseError(const xstr& textValue, const xstr& msgValue, sourceLocation_t sourceLocValue)
	: text(textValue)
	, msg(msgValue)
	, sourceLoc(sourceLocValue)
{
}

const char* ParseException::ParseError::getText() const
{
	return text.c_str();
}

const char* ParseException::ParseError::getMessage() const
{
	return msg.c_str();
}

const sourceLocation_t& ParseException::ParseError::getSourceLoc() const
{
	return sourceLoc;
}

const char* ParseException::ParseError::what() const noexcept
{
	return msg.c_str();
}

CompileException::BaseSource::BaseSource(sourceLocation_t sourceLocValue)
	: sourceLoc(sourceLocValue)
{
}

const mfuse::sourceLocation_t& CompileException::BaseSource::getSourceLoc() const
{
	return sourceLoc;
}

CompileException::UnknownNodeType::UnknownNodeType(uint8_t typeValue)
	: type(typeValue)
{
}

uint8_t CompileException::UnknownNodeType::getType() const
{
	return type;
}

const char* CompileException::UnknownNodeType::what() const noexcept
{
	return "unknown type";
}

CompileException::UnknownCommand::UnknownCommand(const char* cmdNameValue, sourceLocation_t sourceLocValue)
	: BaseSource(sourceLocValue)
	, cmdName(cmdNameValue)
{
}

const char* CompileException::UnknownCommand::getCommandName() const
{
	return cmdName.c_str();
}

const char* CompileException::UnknownCommand::what() const noexcept
{
	return "unknown command";
}

const char* CompileException::UnknownCommandRet::what() const noexcept
{
	return "unknown return command";
}

CompileException::BadValue::BadValue(uint8_t typeValue, sourceLocation_t sourceLocValue)
	: BaseSource(sourceLocValue)
	, type(typeValue)
{
}

uint8_t CompileException::BadValue::getType() const
{
	return type;
}

const char* CompileException::BadValue::what() const noexcept
{
	return "bad value";
}

const char* CompileException::BadLeftValueExpectFieldArray::what() const noexcept
{
	return "bad lvalue (expected field or array)";
}

const char* CompileException::BadCaseValueExpectIntString::what() const noexcept
{
	return "bad case value (expected integer or string)";
}

const char* CompileException::BadParameterLValueExpectField::what() const noexcept
{
	return "bad parameter lvalue (expected field)";
}

CompileException::FieldBase::FieldBase(const char* fieldNameValue, sourceLocation_t sourceLocValue)
	: BaseSource(sourceLocValue)
	, fieldName(fieldNameValue)
{
}

const char* CompileException::FieldBase::getFieldName() const
{
	return fieldName.c_str();
}

const char* CompileException::FieldBase::what() const noexcept
{
	return "bad field";
}

const char* CompileException::WriteOnly::what() const noexcept
{
	return "cannot get a write-only variable";
}

const char* CompileException::ReadOnly::what() const noexcept
{
	return "cannot get a read-only variable";
}

const char* CompileException::NotAllowed::what() const noexcept
{
	return "built-in field not allowed";
}

CompileException::DuplicateLabel::DuplicateLabel(const char* labelNameValue, sourceLocation_t sourceLocValue)
	: BaseSource(sourceLocValue)
	, labelName(labelNameValue)
{
}

const char* CompileException::DuplicateLabel::getLabelName() const
{
	return labelName.c_str();
}

const char* CompileException::DuplicateLabel::what() const noexcept
{
	return "duplicate label";
}

const char* CompileException::BreakJumpLocOverflow::what() const noexcept
{
	return "increase BREAK_JUMP_LOCATION_COUNT and recompile";
}

const char* CompileException::ContinueJumpLocOverflow::what() const noexcept
{
	return "increase CONTINUE_JUMP_LOCATION_COUNT and recompile";
}

const char* CompileException::IllegalBreak::what() const noexcept
{
	return "illegal break";
}

const char* CompileException::IllegalContinue::what() const noexcept
{
	return "illegal continue";
}
