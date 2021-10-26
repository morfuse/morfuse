#pragma once

#include "../Global.h"
#include "ClassDef.h"
#include "../Common/AbstractClass.h"

namespace mfuse
{
	class Class : public AbstractClass
	{
		MFUS_CLASS_ABSTRACT_PROTOTYPE(Class);

	public:
		mfuse_EXPORTS static mfuse::ClassDefTemplate<Class>& staticclass();
		virtual const mfuse::ClassDef& classinfo() const;

	public:
		mfuse_EXPORTS Class();
		mfuse_EXPORTS ~Class();

		mfuse_EXPORTS virtual void Archive(Archiver& arc);

		mfuse_EXPORTS const rawchar_t* GetClassID() const;
		mfuse_EXPORTS const rawchar_t* GetClassname() const;
		mfuse_EXPORTS const rawchar_t* getSuperclass() const;

		mfuse_EXPORTS bool inheritsFrom(ClassDef* c) const;
		mfuse_EXPORTS bool inheritsFrom(const rawchar_t* name) const;
		mfuse_EXPORTS bool isInheritedBy(const rawchar_t* name) const;
		mfuse_EXPORTS bool isInheritedBy(ClassDef* c) const;
	};
};
