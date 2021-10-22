#pragma once

#include "Class.h"
#include "../Container/set.h"
#include "../Common/MEM/PreAllocator.h"
#include "sourcePos.h"
#include "ScriptOpcodes.h"

namespace mfuse
{
	struct sourceinfo_t {
		uint32_t sourcePos;
		uint32_t column;
		uint32_t line;
	};

	using sourcePosMap_t = sourceLocation_t; //con_set<const opval_t*, sourceLocation_t, MEM_ChildPreAllocator_set>;

	class AbstractScript : public AbstractClass
	{
	public:
		AbstractScript();
		virtual ~AbstractScript();

		void CreateProgToSource(size_t startLength = 0);

		mfuse_EXPORTS const_str Filename() const;
		//bool GetSourceAt(size_t sourcePos, str& sourceLine, uint32_t& column, uint32_t& line) const;
		bool GetSourceAt(size_t pos, str& sourceLine, uint32_t& column, uint32_t& line) const;
		bool GetSourceAt(const sourceLocation_t& sourceLoc, str& sourceLine) const;
		void AddSourcePos(uintptr_t pos, sourceLocation_t sourcePos);
		void PrintSourcePos(std::ostream& out, const sourceLocation_t& sourcePos) const;
		//void PrintSourcePos(size_t sourcePos) const;
		void PrintSourcePos(std::ostream& out, size_t pos) const;
		void PrintSourcePos(std::ostream& out, const rawchar_t* sourceLine, uint32_t column, uint32_t line) const;
		const rawchar_t* GetSourceBuffer() const;
		uint64_t GetSourceLength() const;

		MEM::PreAllocator& GetAllocator();

	protected:
		MEM::PreAllocator allocator;
		size_t m_SourceLength;
		sourcePosMap_t* m_ProgToSource;
		const char* m_SourceBuffer;
		const_str m_Filename;
	};
}
