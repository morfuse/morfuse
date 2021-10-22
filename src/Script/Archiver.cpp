#include <morfuse/Script/Archiver.h>
#include <morfuse/Common/SafePtr.h>
#include <morfuse/Script/Class.h>

#include <iostream>

using namespace mfuse;

enum dataType_e
{
	Null,
	Byte,
	Char,
	Short,
	UShort,
	Integer,
	UInteger,
	Long,
	ULong,
	Float,
	Double,
	Boolean,
	Raw,
	Object,
	ObjectPointer,
	SafePointer,
	Position,
	Size,
	Max
};

static const char* typeNames[] =
{
	"Null",
	"Byte",
	"Char",
	"Short",
	"UShort",
	"Integer",
	"UInteger",
	"Long",
	"ULong",
	"Float",
	"Double",
	"Boolean",
	"Raw",
	"Object",
	"ObjectPointer",
	"SafePointer",
	"Position",
	"Size"
};

static constexpr unsigned int ARCHIVE_VERSION = 1;
static constexpr unsigned int ARCHIVE_NULL_POINTER = ~654321u;
static constexpr unsigned int ARCHIVE_POINTER_VALID = 0u;
static constexpr unsigned int ARCHIVE_POINTER_SELF_REFERENTIAL = ~123456u;

version_info_t::version_info_t()
	: header("MFUS")
	, version(1)
	, archiveName("Morfuse Archive")
{
}

Archiver::Archiver()
{
}

Archiver::Archiver(Archiver&& other)
	: numClassesPos(other.numClassesPos)
	, classpointerList(std::move(other.classpointerList))
	, fixupList(std::move(other.fixupList))
	, stream(other.stream)
	, archivemode(other.archivemode)
{
	other.stream = nullptr;
}

Archiver& Archiver::operator=(Archiver&& other)
{
	classpointerList = std::move(other.classpointerList);
	fixupList = std::move(other.fixupList);
	archivemode = other.archivemode;
	stream = other.stream;
	other.stream = nullptr;

	return *this;
}

Archiver::~Archiver()
{
	if (stream) {
		Close();
	}
}

void Archiver::Close()
{
	if (archivemode == archiveMode_e::Write)
	{
		const std::streampos oldPos = writeStream->tellp();
		writeStream->seekp(numClassesPos);

		uint32_t numClasses = (uint32_t)classpointerList.NumObjects();
		ArchiveUInt32(numClasses);

		writeStream->seekp(oldPos);
		writeStream->flush();
		writeStream = nullptr;
	}
	else
	{
		const size_t num = fixupList.NumObjects();
		for (uintptr_t i = 1; i <= num; i++)
		{
			const pointer_fixup_t* const fixup = fixupList.ObjectAt(i);
			void* ptr = classpointerList.ObjectAt(fixup->index);
			if (fixup->type == pointer_fixup_e::normal)
			{
				*fixup->classPtr = static_cast<AbstractClass*>(ptr);
			}
			else if (fixup->type == pointer_fixup_e::safe)
			{
				fixup->safePtr->InitSafePtr(static_cast<AbstractClass*>(ptr));
			}
			delete fixup;
		}
		fixupList.FreeObjectList();
		classpointerList.FreeObjectList();

		readStream = nullptr;
	}
}

Archiver Archiver::CreateWrite(std::ostream& streamWritePtr, const version_info_t& info)
{
	Archiver arc;
	arc.archivemode = archiveMode_e::Write;
	arc.writeStream = &streamWritePtr;

	char* header = const_cast<char*>(info.header);
	arc.WriteDataInternal(header, str::len(header));

	// save morfuse version
	uint16_t mversion = ARCHIVE_VERSION;
	arc.ArchiveUInt16(mversion);

	uint16_t version = info.version;
	arc.ArchiveUInt16(version);

	str archiveName = info.archiveName;
	::Archive(arc, archiveName);

	arc.numClassesPos = arc.writeStream->tellp();

	uint32_t numClasses = 0;
	arc.ArchiveUInt32(numClasses);

	return arc;
}

Archiver Archiver::CreateRead(std::istream& streamReadPtr, const version_info_t& info)
{
	Archiver arc;
	arc.archivemode = archiveMode_e::Read;
	arc.readStream = &streamReadPtr;

	const size_t headerLen = str::len(info.header);
	char* header = new char[headerLen];

	arc.ReadDataInternal(header, headerLen);
	if (std::memcmp(header, info.header, headerLen)) {
		throw ArchiveErrors::InvalidArchiveHeader();
	}

	delete[] header;

	uint16_t mversion = 0, version = 0;
	arc.ArchiveUInt16(mversion);
	arc.ArchiveUInt16(version);

	if (mversion != ARCHIVE_VERSION && version != info.version) {
		throw ArchiveErrors::WrongVersion(mversion, version, ARCHIVE_VERSION, info.version);
	}

	str archiveName = info.archiveName;
	::Archive(arc, archiveName);

	uint32_t numClasses = 0;
	arc.ArchiveUInt32(numClasses);

	arc.classpointerList.SetNumObjects(numClasses);

	return arc;
}

void Archiver::ArchiveInt8(int8_t& num)
{
	ArchiveData(dataType_e::Char, &num, sizeof(num));
}

void Archiver::ArchiveInt16(int16_t& num)
{
	ArchiveData(dataType_e::Short, &num, sizeof(num));
}

void Archiver::ArchiveInt32(int32_t& num)
{
	ArchiveData(dataType_e::Integer, &num, sizeof(num));
}

void Archiver::ArchiveInt64(int64_t& num)
{
	ArchiveData(dataType_e::Long, &num, sizeof(num));
}

void Archiver::ArchiveUInt8(uint8_t& num)
{
	ArchiveData(dataType_e::Byte, &num, sizeof(num));
}

void Archiver::ArchiveUInt16(uint16_t& num)
{
	ArchiveData(dataType_e::UShort, &num, sizeof(num));
}

void Archiver::ArchiveUInt32(uint32_t& num)
{
	ArchiveData(dataType_e::UInteger, &num, sizeof(num));
}

void Archiver::ArchiveUInt64(uint64_t& num)
{
	ArchiveData(dataType_e::ULong, &num, sizeof(num));
}

void Archiver::ArchiveChar(char& val)
{
	ArchiveData(dataType_e::Char, &val, sizeof(val));
}

void Archiver::ArchiveSize(size_t& num)
{
	ArchiveData(dataType_e::Size, &num, sizeof(num));
}

void Archiver::ArchiveByte(uint8_t& num)
{
	ArchiveData(dataType_e::Byte, &num, sizeof(num));
}

void Archiver::ArchiveFloat(float& num)
{
	ArchiveData(dataType_e::Float, &num, sizeof(num));
}

void Archiver::ArchiveDouble(double& num)
{
	ArchiveData(dataType_e::Double, &num, sizeof(num));
}

void Archiver::ArchiveBoolean(bool& boolean)
{
	ArchiveData(dataType_e::Boolean, &boolean, sizeof(boolean));
}

void Archiver::ArchivePosition(uint32_t& pos)
{
	ArchiveData(dataType_e::Position, &pos, sizeof(pos));
}

void Archiver::ArchiveRaw(void* data, size_t size)
{
	ArchiveData(dataType_e::Raw, data, size);
}

void Archiver::ArchiveObjectPointer(void*& ptr)
{
	uint32_t index = 0;

	if (archivemode == archiveMode_e::Read)
	{
		pointer_fixup_t* fixup;
		ArchiveData(dataType_e::ObjectPointer, &index, sizeof(index));

		//
		// see if the variable was NULL
		//
		if (index == ARCHIVE_NULL_POINTER)
		{
			ptr = nullptr;
		}
		else
		{
			// init the pointer with NULL until we can fix it
			ptr = nullptr;

			fixup = new pointer_fixup_t;
			fixup->ptr = &ptr;
			fixup->index = index;
			fixup->type = pointer_fixup_e::normal;
			fixupList.AddObject(fixup);
		}
	}
	else
	{
		if (ptr)
		{
			index = (uint32_t)classpointerList.AddUniqueObject(ptr);
		}
		else
		{
			index = ARCHIVE_NULL_POINTER;
		}
		ArchiveData(dataType_e::ObjectPointer, &index, sizeof(index));
	}
}

void Archiver::ArchiveObjectPosition(void* obj)
{
	uint32_t index = 0;

	if (archivemode == archiveMode_e::Read)
	{
		ArchivePosition(index);
		classpointerList.AddObjectAt(index, obj);
	}
	else
	{
		index = (uint32_t)classpointerList.AddUniqueObject(obj);
		ArchivePosition(index);
	}
}

void Archiver::ArchiveSafePointer(SafePtrBase& ptr)
{
	uint32_t index = 0;

	if (archivemode == archiveMode_e::Read)
	{
		pointer_fixup_t* fixup;

		ArchiveData(dataType_e::SafePointer, &index, sizeof(index));

		//
		// see if the variable was NULL
		//
		if (index == ARCHIVE_NULL_POINTER)
		{
			ptr.InitSafePtr(nullptr);
		}
		else
		{
			// init the pointer with NULL until we can fix it
			ptr.InitSafePtr(nullptr);

			// Add new fixup
			fixup = new pointer_fixup_t;
			fixup->safePtr = &ptr;
			fixup->index = index;
			fixup->type = pointer_fixup_e::safe;
			fixupList.AddObject(fixup);
		}
	}
	else
	{
		if (ptr.Pointer())
		{
			AbstractClass* obj;

			obj = ptr.Pointer();
			index = (uint32_t)classpointerList.AddUniqueObject(obj);
		}
		else
		{
			index = ARCHIVE_NULL_POINTER;
		}
		ArchiveData(dataType_e::SafePointer, &index, sizeof(index));
	}
}

void Archiver::ArchiveObject(Class& obj)
{
	if (archivemode == archiveMode_e::Read)
	{
		const unsigned int type = ReadType();
		if (type != dataType_e::Object)
		{
			throw ArchiveErrors::TypeError(dataType_e::Object, type);
		}

		const std::streamsize size = ReadStreamSize();

		str classname;
		Archive(*this, classname);

		const ClassDef* const cls = ClassDef::GetClass(classname.c_str());
		if (!cls)
		{
			throw ArchiveErrors::InvalidClass(classname);
		}

		if (obj.classinfo() != cls)
		{
			throw ArchiveErrors::ObjectClassError(classname, obj.GetClassname());
		}

		uint32_t index;
		ArchiveUInt32(index);
		const std::streampos objstart = readStream->tellg();
		// archive the class instance
		obj.Archive(*this);

		const std::streampos endpos = readStream->tellg();

		// make sure that it has read the entire object
		if ((endpos - objstart) > size)
		{
			throw ArchiveErrors::ReadPastEndObject();
		}
		else if ((endpos - objstart) < size)
		{
			throw ArchiveErrors::NotReadEntireDataObject();
		}

		//
		// register this pointer with our list
		//
		classpointerList.AddObjectAt(index, static_cast<void*>(&obj));
	}
	else
	{
		WriteType(dataType_e::Object);

		const std::streampos sizepos = writeStream->tellp();
		std::streamsize size = 0;
		WriteStreamSize(size);

		str classname = obj.Class::GetClassname();
		Archive(*this, classname);

		// write out pointer index for this class pointer
		uint32_t index = (uint32_t)classpointerList.AddUniqueObject(static_cast<void*>(&obj));
		ArchiveUInt32(index);

		const std::streampos objstart = writeStream->tellp();
		obj.Archive(*this);

		const std::streampos endpos = writeStream->tellp();

		// archive the object size
		size = endpos - objstart;

		writeStream->seekp(sizepos);
		WriteStreamSize(size);
		writeStream->seekp(endpos);
	}
}

Class* Archiver::ReadObject()
{
	const unsigned int type = ReadType();
	if (type != dataType_e::Object)
	{
		throw ArchiveErrors::TypeError(dataType_e::Object, type);
	}

	const std::streamsize size = ReadStreamSize();

	str classname;
	Archive(*this, classname);

	const ClassDef* const cls = ClassDef::GetClass(classname.c_str());
	if (!cls)
	{
		throw ArchiveErrors::InvalidClass(classname);
	}

	uint32_t index;
	ArchiveUInt32(index);
	const std::streampos objstart = readStream->tellg();

	Class* const obj = cls->createInstance();
	if (!obj)
	{
		throw ArchiveErrors::ObjectInstanceFailed(classname);
	}

	// archive the class instance
	obj->Archive(*this);

	const std::streampos endpos = readStream->tellg();

	// make sure that it has read the entire object
	if ((endpos - objstart) > size)
	{
		throw ArchiveErrors::ReadPastEndObject();
	}
	else if ((endpos - objstart) < size)
	{
		throw ArchiveErrors::NotReadEntireDataObject();
	}

	//
	// register this pointer with our list
	//
	classpointerList.AddObjectAt(index, static_cast<void*>(obj));

	return obj;
}

void Archiver::ArchiveData(unsigned int type, void* data, size_t size)
{
	if (archivemode == archiveMode_e::Read)
	{
		CheckType(type);
		ReadDataInternal((char*)data, size);
	}
	else
	{
		WriteType(type);
		WriteDataInternal((char*)data, size);
	}
}

void Archiver::WriteDataInternal(const void* data, size_t size)
{
	CheckWrite();
	writeStream->write(static_cast<const char*>(data), size);
}

void Archiver::ReadDataInternal(void* data, size_t size)
{
	CheckRead();
	readStream->read(static_cast<char*>(data), size);
}

bool Archiver::Loading() const
{
	return archivemode == archiveMode_e::Read;
}

bool Archiver::IsReading() const
{
	return archivemode == archiveMode_e::Read;
}

bool Archiver::IsSaving() const
{
	return archivemode == archiveMode_e::Write;
}

void Archiver::CheckType(unsigned int type)
{
	assert(type >= 0 && type < dataType_e::Max);
	const unsigned int t = ReadType();
	if (t != type)
	{
		throw ArchiveErrors::TypeError(type, t);
	}
}

unsigned int Archiver::ReadType()
{
	unsigned int type;
	ReadDataInternal(&type, sizeof(type));

	return type;
}

void Archiver::WriteType(unsigned int type)
{
	WriteDataInternal(&type, sizeof(type));
}

std::streamsize Archiver::ReadStreamSize()
{
	std::streamsize s;
	ReadDataInternal(&s, sizeof(s));
	return s;
}

void Archiver::WriteStreamSize(std::streamsize size)
{
	WriteDataInternal(&size, sizeof(size));
}

bool Archiver::ObjectPositionExists(const void* obj) const noexcept
{
	return classpointerList.IndexOfObject(const_cast<void*>(obj)) != 0;
}

void Archiver::CheckRead()
{
	if (archivemode != archiveMode_e::Read || !readStream) {
		throw ArchiveErrors::MissingReadStream();
	}

	if (!readStream->good()) {
		throw ArchiveErrors::ReadStreamFail();
	}
}

void Archiver::CheckWrite()
{
	if (archivemode != archiveMode_e::Write || !writeStream) {
		throw ArchiveErrors::MissingWriteStream();
	}

	if (!writeStream->good()) {
		throw ArchiveErrors::WriteStreamFail();
	}
}

const char* ArchiveErrors::InvalidArchiveHeader::what() const noexcept
{
	return "Archive has bad header.";
}

ArchiveErrors::WrongVersion::WrongVersion(uint32_t engineVersionVal, uint32_t versionVal, uint32_t expectedEngineVersionVal, uint32_t expectedVersionVal)
	: engineVersion(engineVersionVal)
	, version(versionVal)
	, expectedEngineVersion(expectedEngineVersionVal)
	, expectedVersion(expectedVersionVal)
{
}

uint32_t ArchiveErrors::WrongVersion::GetEngineVersion() const
{
	return engineVersion;
}

uint32_t ArchiveErrors::WrongVersion::GetExpectedEngineVersion() const
{
	return expectedEngineVersion;
}

uint32_t ArchiveErrors::WrongVersion::GetVersion() const
{
	return version;
}

uint32_t ArchiveErrors::WrongVersion::GetExpectedVersion() const
{
	return expectedVersion;
}

const char* ArchiveErrors::WrongVersion::what() const noexcept
{
	if (!filled()) {
		fill("Wrong archive version. Got engine version " + xstr(engineVersion) + " and program version " + xstr(version) + ". "
			"(expected " + xstr(expectedEngineVersion) + " and " + xstr(expectedVersion));
	}

	return Messageable::what();
}

ArchiveErrors::TypeError::TypeError(unsigned int expectedTypeVal, unsigned int typeVal)
	: expectedType(expectedTypeVal)
	, type(typeVal)
{
}

unsigned int ArchiveErrors::TypeError::GetExpectedType() const
{
	return expectedType;
}

unsigned int ArchiveErrors::TypeError::GetType() const
{
	return type;
}

const char* ArchiveErrors::TypeError::what() const noexcept
{
	if (!filled())
	{
		if (type < dataType_e::Max)
		{
			fill("Expecting " + xstr(typeNames[expectedType]) + ", found " + typeNames[type]);
		}
		else
		{
			fill("Expecting " + xstr(typeNames[expectedType]) + ", found " + xstr(type) + " (unknown type)");
		}
	}

	return Messageable::what();
}

ArchiveErrors::InvalidClass::InvalidClass(const str& classNameRef)
	: className(classNameRef)
{
}

const str& ArchiveErrors::InvalidClass::GetClassName()
{
	return className;
}

const char* ArchiveErrors::InvalidClass::what() const noexcept
{
	if (!filled()) {
		fill("Invalid class '" + className + "'");
	}

	return Messageable::what();
}

ArchiveErrors::ObjectClassError::ObjectClassError(const str& classNameRef, const rawchar_t* expectedClassNamePtr)
	: className(classNameRef)
	, expectedClassName(expectedClassNamePtr)
{
}

const rawchar_t* ArchiveErrors::ObjectClassError::GetExpectedClass() const
{
	return expectedClassName;
}

const str& ArchiveErrors::ObjectClassError::GetClassName() const
{
	return className;
}

const char* ArchiveErrors::ObjectClassError::what() const noexcept
{
	if (!filled()) {
		fill("Archive has '" + className + "'" + " object, but was expecting a '" + expectedClassName + "' object.");
	}

	return Messageable::what();
}

const char* ArchiveErrors::ReadPastEndObject::what() const noexcept
{
	return "Object read past end of object's data";
}

const char* ArchiveErrors::NotReadEntireDataObject::what() const noexcept
{
	return "Object didn't read entire data from file";
}

ArchiveErrors::ObjectInstanceFailed::ObjectInstanceFailed(const str& classNameRef)
	: className(classNameRef)
{
}

const str& ArchiveErrors::ObjectInstanceFailed::GetClassName()
{
	return className;
}

const char* ArchiveErrors::ObjectInstanceFailed::what() const noexcept
{
	if (!filled()) {
		fill("Failed to on new instance of class '" + className + "'");
	}

	return Messageable::what();
}

const char* ArchiveErrors::MissingReadStream::what() const noexcept
{
	return "Couldn't read data as there is no read stream";
}

const char* ArchiveErrors::ReadStreamFail::what() const noexcept
{
	return "Failure in read stream";
}

const char* ArchiveErrors::MissingWriteStream::what() const noexcept
{
	return "Couldn't write data as there is no write stream";
}

const char* ArchiveErrors::WriteStreamFail::what() const noexcept
{
	return "Failure in write stream";
}
