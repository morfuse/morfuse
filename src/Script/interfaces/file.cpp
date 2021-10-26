#include <morfuse/Script/interfaces/file.h>

using namespace mfuse;

FileExceptions::NotFound::NotFound(const char* fnameValue)
	: fname(fnameValue)
{}

FileExceptions::NotFound::~NotFound()
{

}

const char* FileExceptions::NotFound::what() const noexcept
{
	NotFound* This = const_cast<NotFound*>(this);
	This->msg = "The specified file was not found: \"" + fname + "\"";
	return This->msg.c_str();
}

const char* FileExceptions::NotFound::getFileName() const noexcept
{
	return fname.c_str();
}
