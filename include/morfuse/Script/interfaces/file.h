#pragma once

#include "../../Global.h"
#include "../ScriptException.h"
#include "../../Common/str.h"

#include <istream>

namespace mfuse
{
	class IFile
	{
	public:
		virtual ~IFile() = default;

		/**
		 * Return the stream associated with the file.
		 */
		virtual std::istream& getStream() noexcept = 0;
	};

	class IFileManagement
	{
	public:
		virtual IFile* OpenFile(const char* fname) = 0;
		virtual void CloseFile(IFile* file) noexcept = 0;
	};

	namespace FileExceptions
	{
		class Base : public ScriptExceptionBase {};

		/**
		 * The specified file was not found.
		 */
		class mfuse_PUBLIC NotFound : public Base
		{
		public:
			mfuse_EXPORTS NotFound(const char* fname);
			mfuse_EXPORTS ~NotFound();

			mfuse_EXPORTS const char* what() const noexcept override;
			mfuse_EXPORTS const char* getFileName() const noexcept;

		private:
			xstr fname;
			xstr msg;
		};
	}
}
