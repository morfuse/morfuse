#pragma once

#include "../Global.h"
#include "../Container/set.h"
#include "../Container/Container.h"
#include "../Common/str.h"
#include "Class.h"
#include "MessageableException.h"

#include <cstdint>

namespace mfuse
{
	class SafePtrBase;
	class AbstractClass;
	class ClassDef;

	enum archiveMode_e
	{
		Write,
		Read
	};

	enum pointer_fixup_e
	{
		normal,
		safe
	};

	struct pointer_fixup_t
	{
		union {
			AbstractClass** classPtr;
			SafePtrBase* safePtr;
			void** ptr;
		};
		uintptr_t index;
		pointer_fixup_e type;
	};

	struct mfuse_EXPORTS version_info_t
	{
		const char* header;
		const char* archiveName;
		unsigned int version;

		version_info_t();
	};

	class Archiver
	{
	public:
		mfuse_EXPORTS ~Archiver();

		Archiver(const Archiver& other) = delete;
		Archiver& operator=(const Archiver& other) = delete;
		Archiver(Archiver&& other);
		Archiver& operator=(Archiver&& other);

		mfuse_EXPORTS static Archiver CreateWrite(std::ostream& streamWritePtr, const version_info_t& info);
		mfuse_EXPORTS static Archiver CreateRead(std::istream& streamReadPtr, const version_info_t& info);

		mfuse_EXPORTS void ArchiveInt8(int8_t& num);
		mfuse_EXPORTS void ArchiveInt16(int16_t& num);
		mfuse_EXPORTS void ArchiveInt32(int32_t& num);
		mfuse_EXPORTS void ArchiveInt64(int64_t& unum);
		mfuse_EXPORTS void ArchiveUInt8(uint8_t& num);
		mfuse_EXPORTS void ArchiveUInt16(uint16_t& num);
		mfuse_EXPORTS void ArchiveUInt32(uint32_t& num);
		mfuse_EXPORTS void ArchiveUInt64(uint64_t& num);
		mfuse_EXPORTS void ArchiveChar(char& val);
		mfuse_EXPORTS void ArchiveSize(size_t& num);
		mfuse_EXPORTS void ArchiveByte(uint8_t& num);
		mfuse_EXPORTS void ArchiveFloat(float& num);
		mfuse_EXPORTS void ArchiveDouble(double& num);
		mfuse_EXPORTS void ArchiveBoolean(bool& boolean);
		mfuse_EXPORTS void ArchivePosition(uint32_t& pos);
		mfuse_EXPORTS void ArchiveStreamSize(std::streamsize& value);
		mfuse_EXPORTS void ArchiveRaw(void* data, size_t size);
		template<typename T> void ArchiveObjectPointer(T*& ptr);
		mfuse_EXPORTS void ArchiveObjectPointer(void*& ptr);
		mfuse_EXPORTS void ArchiveObjectPosition(void* obj);
		mfuse_EXPORTS void ArchiveSafePointer(SafePtrBase& ptr);
		mfuse_EXPORTS void ArchiveObject(Class& obj);
		mfuse_EXPORTS Class* ReadObject();
		template<typename T> T* ReadObject();

		template<typename T> void ArchiveEnum(T& enumValue);
		template<typename T> void ArchiveElements(T* elements, size_t count);

		mfuse_EXPORTS bool ObjectPositionExists(const void* obj) const noexcept;

		mfuse_EXPORTS bool Loading() const;
		mfuse_EXPORTS bool IsReading() const;
		mfuse_EXPORTS bool IsSaving() const;

	private:
		Archiver();

		void CheckRead();
		void CheckWrite();
		void Close();
		void CheckType(unsigned int type);
		unsigned int ReadType();
		void ArchiveData(unsigned int type, void* data, size_t size);
		void ReadDataInternal(void* data, size_t size);
		void WriteDataInternal(const void* data, size_t size);

		void WriteType(unsigned int type);

		std::streamsize ReadStreamSize();
		void WriteStreamSize(std::streamsize size);

	private:
		std::streampos numClassesPos;
		con::Container<void*> classpointerList;
		con::Container<pointer_fixup_t*> fixupList;
		union {
			std::istream* readStream;
			std::ostream* writeStream;
			void* stream;
		};
		archiveMode_e archivemode;
	};

	template<typename T>
	void Archiver::ArchiveObjectPointer(T*& ptr)
	{
		ArchiveObjectPointer((void*&)ptr);
	}

	template<typename T>
	T* Archiver::ReadObject()
	{
		const ClassDef& cls = T::staticclass();

		T* instance = static_cast<T*>(cls.createInstance());
		ArchiveObject(*instance);
		return instance;
	}

	template<typename T>
	void Archiver::ArchiveEnum(T& enumValue)
	{
		if (Loading())
		{
			uint8_t byteValue;
			ArchiveByte(byteValue);
			enumValue = T(byteValue);
		}
		else
		{
			uint8_t byteValue = (uint8_t)enumValue;
			ArchiveByte(byteValue);
		}
	}

	template<typename T>
	void Archiver::ArchiveElements(T* elements, size_t count)
	{
		for (size_t i = 0; i < count; i++)
		{
			ArchiveRaw(elements, sizeof(*elements) * count);
		}
	}

	namespace ArchiveErrors
	{
		class Base : std::exception {};

		class InvalidArchiveHeader : public Base
		{
		public:
			const char* what() const noexcept override;
		};

		class WrongVersion : public Base, public Messageable
		{
		public:
			WrongVersion(uint32_t engineVersionVal, uint32_t versionVal, uint32_t expectedEngineVersionVal, uint32_t expectedVersionVal);

			uint32_t GetEngineVersion() const;
			uint32_t GetVersion() const;
			uint32_t GetExpectedEngineVersion() const;
			uint32_t GetExpectedVersion() const;
			const char* what() const noexcept override;

		private:
			uint32_t engineVersion;
			uint32_t version;
			uint32_t expectedEngineVersion;
			uint32_t expectedVersion;
		};

		class MissingReadStream : public Base
		{
		public:
			const char* what() const noexcept override;
		};

		class ReadStreamFail : public Base
		{
		public:
			const char* what() const noexcept override;
		};

		class MissingWriteStream : public Base
		{
		public:
			const char* what() const noexcept override;
		};

		class WriteStreamFail : public Base
		{
		public:
			const char* what() const noexcept override;
		};

		class TypeError : public Base, public Messageable
		{
		public:
			TypeError(unsigned int expectedTypeVal, unsigned int typeVal);

			unsigned int GetExpectedType() const;
			unsigned int GetType() const;
			const char* what() const noexcept override;

		private:
			unsigned int expectedType;
			unsigned int type;
		};

		class InvalidClass : public Base, public Messageable
		{
		public:
			InvalidClass(const str& classNameRef);

			const str& GetClassName();
			const char* what() const noexcept override;

		private:
			str className;
		};

		class ObjectInstanceFailed : public Base, public Messageable
		{
		public:
			ObjectInstanceFailed(const str& classNameRef);

			const str& GetClassName();
			const char* what() const noexcept override;

		private:
			str className;
		};

		class ObjectClassError : public Base, public Messageable
		{
		public:
			ObjectClassError(const str& classNameRef, const rawchar_t* expectedClassNamePtr);

			const rawchar_t* GetExpectedClass() const;
			const str& GetClassName() const;
			const char* what() const noexcept override;

		private:
			str className;
			const rawchar_t* expectedClassName;
		};

		class ReadPastEndObject : public Base, public Messageable
		{
		public:
			const char* what() const noexcept override;
		};

		class NotReadEntireDataObject : public Base, public Messageable
		{
		public:
			const char* what() const noexcept override;
		};
	}
}
