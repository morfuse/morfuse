#pragma once

#include "../Common/ConstStr.h"
#include "../Common/SafePtr.h"
#include "../Common/rawchar.h"
#include "../Container/Container.h"
#include "../Container/set.h"
#include "ScriptException.h"

#include <cstdint>

namespace mfuse
{
	class Listener;
	template<class T>
	class SafePtr;

	using ConTarget = con::Container<SafePtr<Listener>>;

	class TargetList
	{
	public:
		TargetList();
		~TargetList();

		void AddListener(Listener& ent, const_str targetName);
		void RemoveListener(Listener& ent, const_str targetName);

		mfuse_EXPORTS void FreeTargetList();

		mfuse_EXPORTS Listener* GetNextTarget(const Listener* ent, const_str targetname) const;
		mfuse_EXPORTS Listener* GetTarget(const_str targetname) const;
		mfuse_EXPORTS uintptr_t GetTargetnameIndex(Listener& ent, const_str targetname) const;

		mfuse_EXPORTS ConTarget* GetExistingTargetList(const_str targetname);
		mfuse_EXPORTS const ConTarget* GetExistingConstTargetList(const_str targetname) const;
		mfuse_EXPORTS ConTarget* GetTargetList(const_str targetname);

	private:
		uintptr_t FindTarget(const ConTarget& list, const Listener* target) const;

	private:
		con::set<const_str, ConTarget> m_targetList;
	};

	namespace TargetListErrors
	{
		class Base : public ScriptExceptionBase {};

		class MultipleTargetsException : public Base, public Messageable
		{
		public:
			MultipleTargetsException(size_t numTargetsValue, const_str targetNameValue);

			size_t GetNumTargets() const noexcept;
			const_str GetTargetName() const noexcept;
			const char* what() const noexcept override;

		private:
			size_t numTargets;
			const_str targetName;
		};

		class NoTargetException : public Base, public Messageable
		{
		public:
			NoTargetException(const_str targetNameValue);

			const_str GetTargetName() const noexcept;
			const char* what() const noexcept override;

		private:
			const_str targetName;
		};
	}
}