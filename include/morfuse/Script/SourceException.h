#pragma once

#include "../Common/str.h"
#include "sourcePos.h"

#include <exception>

namespace mfuse
{
	namespace ParseException
	{
		class Base : public std::exception {};

		class ParseError : public Base
		{
		public:
			ParseError(const xstr& textValue, const xstr& msgValue, sourceLocation_t sourceLocValue);

			const char* getText() const;
			const char* getMessage() const;
			const sourceLocation_t& getSourceLoc() const;
			const char* what() const noexcept override;

		private:
			xstr text;
			xstr msg;
			sourceLocation_t sourceLoc;
		};
	}

	namespace CompileException
	{
		class Base : public std::exception {};

		class BaseSource : public Base
		{
		public:
			BaseSource(sourceLocation_t sourceLocValue);

			const sourceLocation_t& getSourceLoc() const;

		private:
			sourceLocation_t sourceLoc;
		};

		class UnknownNodeType : public Base
		{
		public:
			UnknownNodeType(uint8_t typeValue);

			uint8_t getType() const;

			const char* what() const noexcept override;

		private:
			uint8_t type;
		};

		class BreakJumpLocOverflow : public BaseSource
		{
		public:
			using BaseSource::BaseSource;

			const char* what() const noexcept override;
		};
		class ContinueJumpLocOverflow : public BaseSource
		{
		public:
			using BaseSource::BaseSource;

			const char* what() const noexcept override;
		};
		class IllegalBreak : public BaseSource
		{
		public:
			using BaseSource::BaseSource;

			const char* what() const noexcept override;
		};
		class IllegalContinue : public BaseSource
		{
		public:
			using BaseSource::BaseSource;

			const char* what() const noexcept override;
		};

		class UnknownCommand : public BaseSource
		{
		public:
			UnknownCommand(const char* cmdNameValue, sourceLocation_t sourceLocValue);

			const char* getCommandName() const;

			const char* what() const noexcept override;

		private:
			xstr cmdName;
		};

		class UnknownCommandRet : public UnknownCommand
		{
		public:
			using UnknownCommand::UnknownCommand;

			const char* what() const noexcept override;
		};

		class BadValue : public BaseSource
		{
		public:
			BadValue(uint8_t typeValue, sourceLocation_t sourceLocValue);

			uint8_t getType() const;

			const char* what() const noexcept override;

		private:
			uint8_t type;
		};

		class BadLeftValueExpectFieldArray : public BadValue
		{
		public:
			using BadValue::BadValue;

			const char* what() const noexcept override;
		};
		class BadCaseValueExpectIntString : public BadValue
		{
		public:
			using BadValue::BadValue;

			const char* what() const noexcept override;
		};
		class BadParameterLValueExpectField : public BadValue
		{
		public:
			using BadValue::BadValue;

			const char* what() const noexcept override;
		};

		class FieldBase : public BaseSource
		{
		public:
			FieldBase(const char* fieldNameValue, sourceLocation_t sourceLocValue);

			const char* getFieldName() const;

			const char* what() const noexcept override;

		private:
			xstr fieldName;
		};

		class WriteOnly : public FieldBase
		{
		public:
			using FieldBase::FieldBase;

			const char* what() const noexcept override;
		};
		class ReadOnly : public FieldBase
		{
		public:
			using FieldBase::FieldBase;

			const char* what() const noexcept override;
		};
		class NotAllowed : public FieldBase
		{
		public:
			using FieldBase::FieldBase;

			const char* what() const noexcept override;
		};

		class DuplicateLabel : public BaseSource
		{
		public:
			DuplicateLabel(const char* labelNameValue, sourceLocation_t sourceLocValue);

			const char* getLabelName() const;

			const char* what() const noexcept override;

		private:
			xstr labelName;
		};
	}
}