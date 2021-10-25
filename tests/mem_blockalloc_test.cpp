#include "helpers/assert.h"

#include <morfuse/Common/MEM/BlockAlloc.h>

struct TestStruct
{
public:
	uint32_t values[4];
};

void testCorruption(TestStruct* instArr[], uint32_t start, uint32_t end);

template<size_t blockSize>
void testNumElements(mfuse::MEM::BlockAlloc<TestStruct, blockSize>& allocator, uint32_t expected);

constexpr uint32_t getIndex(const uint32_t i, const uint32_t j)
{
	return (i + j) * (j + 1);
}

template<size_t numElements, size_t numToFree, size_t blockSize, size_t iterations = 1000>
void testMemory()
{
	mfuse::MEM::BlockAlloc<TestStruct, blockSize> allocator;

	static constexpr uint32_t numBlocks = numElements / mfuse::MEM::DefaultBlock;

	TestStruct* instArr[numElements];

	for (size_t i = 0; i < iterations; ++i)
	{
		for (uint32_t i = 0; i < numElements; ++i)
		{
			instArr[i] = new (allocator) TestStruct
			{
				getIndex(i, 1),
				getIndex(i, 2),
				getIndex(i, 3),
				getIndex(i, 4)
			};
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
		
		assertTest(allocator.Count() == 0);
		assertTest(allocator.BlockCount() == 1);
	}

	for (uint32_t i = 0; i < numElements; ++i)
	{
		instArr[i] = new (allocator) TestStruct
		{
			getIndex(i, 1),
			getIndex(i, 2),
			getIndex(i, 3),
			getIndex(i, 4)
		};
	}

	testNumElements(allocator, numElements);
	testCorruption(instArr, 0, numElements);

	allocator.FreeAll();

	assertTest(allocator.Count() == 0);
	assertTest(allocator.BlockCount() == 0);
}

int main(int argc, char* argv[])
{
	testMemory<791, 768, 256, 2000>();
	testMemory<1536, 1024, 1024, 1500>();
	testMemory<4087, 3455, 65537, 1000>();
	testMemory<16840, 9564, 131072, 1000>();
}

template<size_t blockSize>
inline void testNumElements(mfuse::MEM::BlockAlloc<TestStruct, blockSize>& allocator, uint32_t expected)
{
	size_t num = 0;

	mfuse::MEM::BlockAlloc_enum<TestStruct, blockSize> en(allocator);
	for (TestStruct* elem = en.NextElement(); elem; elem = en.NextElement())
	{
		++num;
	};

	assertTest(num == expected);
	assertTest(allocator.Count() == expected);
}

inline void testCorruption(TestStruct* instArr[], uint32_t start, uint32_t end)
{
	for (uint32_t i = start; i < end; ++i)
	{
		TestStruct* ts = instArr[i];
		assertTest(ts->values[0] == getIndex(i, 1));
		assertTest(ts->values[1] == getIndex(i, 2));
		assertTest(ts->values[2] == getIndex(i, 3));
		assertTest(ts->values[3] == getIndex(i, 4));
	}
}