#pragma once

#include "Class.h"
#include "../Container/set.h"
#include "../Container/Container.h"
#include "../Common/MEM/DefaultAlloc.h"
#include "../Common/MEM/PreAllocator.h"
#include "../Common/rawchar.h"
#include "StringResolvable.h"
#include "ScriptException.h"
#include "ScriptOpcodes.h"

namespace mfuse
{
	class ProgramScript;

	struct script_label_t {
		/** Pointer to the actual code position. */
		const opval_t* codepos;

		/** The label name the codepos is in. */
		//const_str key;

		/** True if the label is in a private part. */
		bool isprivate;
	};
	using labelMap = con::set<const_str, script_label_t, Hash<const_str>, EqualTo<const_str>, MEM::ChildPreAllocator_set>;

	class StateScript : public Class
	{
	public:
		StateScript(ProgramScript* inParent);
		~StateScript();

		void Archive(Archiver& arc) override;

		bool AddLabel(const_str label, const opval_t* pos, bool private_section = false);
		const script_label_t* FindLabel(const_str label) const;
		void Reserve(size_t count);

		ProgramScript* GetParent() const;

	private:
		/** The list of labels the StateScript contains. */
		labelMap label_list;
		//Container<script_label_t*, , MEM::ChildPreAllocator> reverse_label_list;

		/** The parent game script. */
		ProgramScript* m_Parent;
	};

	namespace StateScriptErrors
	{
		class Base : public ScriptExceptionBase {};

		class mfuse_PUBLIC LabelNotFound : public Base, public Messageable
		{
		public:
			mfuse_LOCAL LabelNotFound(const StringResolvable& label, const StringResolvable& fileName);
			mfuse_LOCAL virtual ~LabelNotFound();

			mfuse_EXPORTS const StringResolvable& GetLabel() const noexcept;
			mfuse_EXPORTS const StringResolvable& GetFileName() const noexcept;
			mfuse_LOCAL const char* what() const noexcept override;

		private:
			StringResolvable fileName;
			StringResolvable label;
		};
	};
}
