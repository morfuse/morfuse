#pragma once

#include "../Common/str.h"
#include "sourcePos.h"

#include "ScriptException.h"

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
		class mfuse_PUBLIC Base : public std::exception {};

		class mfuse_PUBLIC BaseSource : public Base
		{
		public:
			mfuse_LOCAL BaseSource(sourceLocation_t sourceLocValue);

			mfuse_EXPORTS const sourceLocation_t& getSourceLoc() const;

		private:
			sourceLocation_t sourceLoc;
		};

		class mfuse_PUBLIC UnknownNodeType : public Base
		{
		public:
			mfuse_LOCAL UnknownNodeType(uint8_t typeValue);

			mfuse_EXPORTS uint8_t getType() const;
			mfuse_EXPORTS const char* what() const noexcept override;

		private:
			uint8_t type;
		};

		class mfuse_PUBLIC BreakJumpLocOverflow : public BaseSource
		{
		public:
			using BaseSource::BaseSource;

			mfuse_EXPORTS const char* what() const noexcept override;
		};
		class mfuse_PUBLIC ContinueJumpLocOverflow : public BaseSource
		{
		public:
			using BaseSource::BaseSource;

			mfuse_EXPORTS const char* what() const noexcept override;
		};
		class mfuse_PUBLIC IllegalBreak : public BaseSource
		{
		public:
			using BaseSource::BaseSource;

			mfuse_EXPORTS const char* what() const noexcept override;
		};
		class mfuse_PUBLIC IllegalContinue : public BaseSource
		{
		public:
			using BaseSource::BaseSource;

			mfuse_EXPORTS const char* what() const noexcept override;
		};

		class mfuse_PUBLIC UnknownCommand : public BaseSource, public Messageable
		{
		public:
			mfuse_LOCAL UnknownCommand(const char* cmdNameValue, sourceLocation_t sourceLocValue);

			mfuse_EXPORTS const char* getCommandName() const;
			mfuse_EXPORTS const char* what() const noexcept override;

		protected:
			const xstr& getPrivateCommandName() const;

		private:
			xstr cmdName;
		};

		class mfuse_PUBLIC UnknownCommandRet : public UnknownCommand
		{
		public:
			using UnknownCommand::UnknownCommand;

			mfuse_EXPORTS const char* what() const noexcept override;
		};

		class mfuse_PUBLIC BadValue : public BaseSource
		{
		public:
			mfuse_LOCAL BadValue(uint8_t typeValue, sourceLocation_t sourceLocValue);

			mfuse_EXPORTS uint8_t getType() const;
			mfuse_EXPORTS const char* what() const noexcept override;

		private:
			uint8_t type;
		};

		class mfuse_PUBLIC BadLeftValueExpectFieldArray : public BadValue
		{
		public:
			using BadValue::BadValue;

			mfuse_EXPORTS const char* what() const noexcept override;
		};
		class mfuse_PUBLIC BadCaseValueExpectIntString : public BadValue
		{
		public:
			using BadValue::BadValue;

			mfuse_EXPORTS const char* what() const noexcept override;
		};
		class mfuse_PUBLIC BadParameterLValueExpectField : public BadValue
		{
		public:
			using BadValue::BadValue;

			mfuse_EXPORTS const char* what() const noexcept override;
		};

		class mfuse_PUBLIC FieldBase : public BaseSource
		{
		public:
			mfuse_LOCAL FieldBase(const char* fieldNameValue, sourceLocation_t sourceLocValue);

			mfuse_EXPORTS const char* getFieldName() const;
			mfuse_EXPORTS const char* what() const noexcept override;

		private:
			xstr fieldName;
		};

		class mfuse_PUBLIC WriteOnly : public FieldBase
		{
		public:
			using FieldBase::FieldBase;

			mfuse_EXPORTS const char* what() const noexcept override;
		};
		class mfuse_PUBLIC ReadOnly : public FieldBase
		{
		public:
			using FieldBase::FieldBase;

			mfuse_EXPORTS const char* what() const noexcept override;
		};
		class mfuse_PUBLIC NotAllowed : public FieldBase
		{
		public:
			using FieldBase::FieldBase;

			mfuse_EXPORTS const char* what() const noexcept override;
		};

		class mfuse_PUBLIC DuplicateLabel : public BaseSource
		{
		public:
			mfuse_LOCAL DuplicateLabel(const char* labelNameValue, sourceLocation_t sourceLocValue);

			mfuse_EXPORTS const char* getLabelName() const;
			mfuse_EXPORTS const char* what() const noexcept override;

		private:
			xstr labelName;
		};
	}
}