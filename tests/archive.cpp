#include "helpers/archive.h"
#include "helpers/assert.h"

#include <morfuse/Script/Listener.h>
#include <morfuse/Script/EventContext.h>

using namespace mfuse;

void WriteTest(std::ostream& stream, const version_info_t& info)
{
	Listener* inst1;
	Listener* pInst1_1, * pInst1_2, * pInst1_3;
	SafePtr<Listener> safe1_1, safe1_2, safe1_3;

	Listener* inst2;
	Listener* pInst2_1, * pInst2_2, * pInst2_3;
	SafePtr<Listener> safe2_1, safe2_2, safe2_3;

	Listener* inst3;
	Listener* pInst3;

	Listener* listeners[1000];

	{
		Archiver arc = Archiver::CreateWrite(stream, info);

		uint8_t num8 = 1;
		arc.ArchiveUInt8(num8);
		uint16_t num16 = 2;
		arc.ArchiveUInt16(num16);
		uint32_t num32 = 3;
		arc.ArchiveUInt32(num32);

		str string = "Test string";
		Archive(arc, string);

		inst1 = new Listener();
		pInst1_1 = pInst1_2 = pInst1_3 = inst1;
		safe1_1 = safe1_2 = safe1_3 = inst1;

		// archive the object
		arc.ArchiveObject(*inst1);
		// save pointers
		arc.ArchiveSafePointer(safe1_1);
		arc.ArchiveObjectPointer(pInst1_1);
		arc.ArchiveSafePointer(safe1_2);
		arc.ArchiveSafePointer(safe1_3);
		arc.ArchiveObjectPointer(pInst1_2);
		arc.ArchiveObjectPointer(pInst1_3);

		inst2 = new Listener();
		pInst2_1 = pInst2_2 = pInst2_3 = inst2;
		safe2_1 = safe2_2 = safe2_3 = inst2;

		// save pointers
		arc.ArchiveSafePointer(safe2_1);
		arc.ArchiveObjectPointer(pInst2_1);
		arc.ArchiveSafePointer(safe2_2);
		// archive the object
		arc.ArchiveObject(*inst2);
		// save pointers
		arc.ArchiveSafePointer(safe2_3);
		arc.ArchiveObjectPointer(pInst2_2);
		arc.ArchiveObjectPointer(pInst2_3);

		inst3 = new Listener();
		arc.ArchiveObject(*inst3);

		pInst3 = inst3;
		arc.ArchiveObjectPointer(pInst3);

		for (size_t i = 0; i < 1000; i++)
		{
			Listener* const l = new Listener();
			arc.ArchiveObject(*l);

			listeners[i] = l;
		}
	}

	delete inst1;
	delete inst2;
	delete inst3;

	for (size_t i = 0; i < 1000; i++)
	{
		delete listeners[i];
	}
}

void ReadTest(std::istream& stream, const version_info_t& info)
{
	Listener* inst1 = nullptr;
	Listener* pInst1_1, * pInst1_2, * pInst1_3;
	SafePtr<Listener> safe1_1, safe1_2, safe1_3;

	Listener* inst2 = nullptr;
	Listener* pInst2_1, * pInst2_2, * pInst2_3;
	SafePtr<Listener> safe2_1, safe2_2, safe2_3;

	Listener* inst3 = nullptr;
	Listener* pInst3 = nullptr;

	Listener* listeners[1000];

	{
		Archiver arc = Archiver::CreateRead(stream, info);

		uint8_t num8;
		arc.ArchiveUInt8(num8);
		assertTest(num8 == 1);
		uint16_t num16;
		arc.ArchiveUInt16(num16);
		assertTest(num16 == 2);
		uint32_t num32;
		arc.ArchiveUInt32(num32);
		assertTest(num32 == 3);

		str string;
		Archive(arc, string);
		assertTest(string == "Test string");

		// instance 1
		inst1 = new Listener();

		// archive the object
		arc.ArchiveObject(*inst1);
		// save pointers
		arc.ArchiveSafePointer(safe1_1);
		arc.ArchiveObjectPointer(pInst1_1);
		arc.ArchiveSafePointer(safe1_2);
		arc.ArchiveSafePointer(safe1_3);
		arc.ArchiveObjectPointer(pInst1_2);
		arc.ArchiveObjectPointer(pInst1_3);

		// instance 2
		inst2 = new Listener();

		// save pointers
		arc.ArchiveSafePointer(safe2_1);
		arc.ArchiveObjectPointer(pInst2_1);
		arc.ArchiveSafePointer(safe2_2);
		// archive the object
		arc.ArchiveObject(*inst2);
		// save pointers
		arc.ArchiveSafePointer(safe2_3);
		arc.ArchiveObjectPointer(pInst2_2);
		arc.ArchiveObjectPointer(pInst2_3);

		// instance 3
		inst3 = arc.ReadObject<Listener>();
		arc.ArchiveObjectPointer(pInst3);

		for (size_t i = 0; i < 1000; i++)
		{
			Listener* const l = new Listener();
			arc.ArchiveObject(*l);

			listeners[i] = l;
		}
	}

	assertTest(pInst1_1 == inst1 && pInst1_2 == inst1 && pInst1_3 == inst1);
	assertTest(safe1_1 == inst1 && safe1_2 == inst1 && safe1_3 == inst1);

	assertTest(pInst2_1 == inst2 && pInst2_2 == inst2 && pInst2_3 == inst2);
	assertTest(safe2_1 == inst2 && safe2_2 == inst2 && safe2_3 == inst2);

	assertTest(inst3);
	assertTest(pInst3 == inst3);

	delete inst1;
	delete inst2;
	delete inst3;

	for (size_t i = 0; i < 1000; i++)
	{
		delete listeners[i];
	}
}

int main()
{
	EventContext context;

	TestArchive<49357>(&WriteTest, &ReadTest);

	return 0;
}
