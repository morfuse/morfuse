#include <morfuse/Common/MEM/BlockAlloc.h>
#include <cassert>

struct TestStruct
{
public:
	uint32_t values[4];
};

constexpr size_t MemBlockSize = 256;
using MemAllocator = mfuse::MEM::BlockAlloc<TestStruct, MemBlockSize>;

void testCorruption(TestStruct* instArr[], uint32_t start, uint32_t end);
void testNumElements(MemAllocator& allocator, uint32_t expected);

int main(int argc, char* argv[])
{
	MemAllocator allocator;

	static constexpr uint32_t numElements = 791;
	static constexpr uint32_t numToFree = 768;
	static constexpr uint32_t numBlocks = numElements / mfuse::MEM::DefaultBlock;

	TestStruct* instArr[numElements];

	for (size_t i = 0; i < 10000; ++i)
	{
		for (uint32_t i = 0; i < numElements; ++i)
		{
			instArr[i] = new (allocator) TestStruct{ i, i * 2, i * 3, i * 4 };
		}

		testNumElements(allocator, numElements);
		testCorruption(instArr, 0, numElements);

		for (uint32_t i = 0; i < numToFree; ++i)
		{
			instArr[i]->~TestStruct();
			allocator.Free(instArr[i]);
		}

		testNumElements(allocator, numElements - numToFree);
		testCorruption(instArr, numToFree, numElements);

		for (uint32_t i = numToFree; i < numElements; ++i)
		{
			instArr[i]->~TestStruct();
			allocator.Free(instArr[i]);
		}
	}
}

void testNumElements(MemAllocator& allocator, uint32_t expected)
{
	size_t num = 0;

	mfuse::MEM::BlockAlloc_enum<TestStruct, MemBlockSize> en(allocator);
	for (TestStruct* elem = en.NextElement(); elem; elem = en.NextElement())
	{
		++num;
	};

	assert(num == expected);
	assert(allocator.Count() == expected);
}

void testCorruption(TestStruct* instArr[], uint32_t start, uint32_t end)
{
	for (uint32_t i = start; i < end; ++i)
	{
		TestStruct* ts = instArr[i];
		assert(ts->values[0] == i);
		assert(ts->values[1] == i * 2);
		assert(ts->values[2] == i * 3);
		assert(ts->values[3] == i * 4);
	}
}