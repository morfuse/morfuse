#include <morfuse/Common/membuf.h>
#include <morfuse/Script/Archiver.h>

template<typename Stream>
using ArchiveTestHandler = void (*)(Stream& stream, const mfuse::version_info_t& info);

template<size_t maxSize>
inline void TestArchive(ArchiveTestHandler<std::ostream> WriteHandler, ArchiveTestHandler<std::istream> ReadHandler)
{
	char buf[maxSize];
	mfuse::omemstream streamWrite(buf, sizeof(buf));
	mfuse::imemstream streamRead(buf, sizeof(buf));

	mfuse::version_info_t info;
	info.header = "TEST";
	info.version = 1;
	info.archiveName = "Morfuse test archive";
	WriteHandler(streamWrite, info);
	ReadHandler(streamRead, info);
}
