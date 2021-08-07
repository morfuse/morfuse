#pragma once

#include "../../Utilities/SharedPtr.h"

namespace mfuse
{
	class IFile
	{
	public:
		virtual ~IFile() = default;


	};
	using IFilePtr = SharedPtr<IFile>;

	class IFileManagement
	{
	public:
		virtual IFilePtr OpenFile(const char* fname) = 0;
	};
}
