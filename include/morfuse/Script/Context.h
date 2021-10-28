#pragma once

#include "../Global.h"
#include "../Common/MEM/BlockAlloc.h"
#include "../Common/OutputInfo.h"
#include "../Common/ThreadSingleton.h"
#include "../Common/Time.h"
#include "interfaces/file.h"
#include "Level.h"
#include "Game.h"
#include "ScriptClass.h"
#include "ScriptMaster.h"
#include "EventSystem.h"
#include "TargetList.h"
#include "EventQueue.h"
#include "EventContext.h"
#include "TrackedInstances.h"

#include <cstdint>
#include <chrono>

namespace mfuse
{
	class Game;
	class Level;
	class ScriptThread;
	class ScriptClass;
	class ScriptVM;

	template<size_t BlockSize>
	class ScriptAllocator
	{
	public:
		template<typename T>
		MEM::BlockAlloc<T, BlockSize>& GetBlock();

		template<>
		MEM::BlockAlloc<ScriptThread, BlockSize>& GetBlock<ScriptThread>()
		{
			return ScriptThread_allocator;
		}

		template<>
		MEM::BlockAlloc<ScriptClass, BlockSize>& GetBlock<ScriptClass>()
		{
			return ScriptClass_allocator;
		}

		template<>
		MEM::BlockAlloc<ScriptVM, BlockSize>& GetBlock<ScriptVM>()
		{
			return ScriptVM_allocator;
		}

	private:
		MEM::BlockAlloc<ScriptThread, BlockSize> ScriptThread_allocator;
		MEM::BlockAlloc<ScriptVM, BlockSize> ScriptVM_allocator;
		MEM::BlockAlloc<ScriptClass, BlockSize> ScriptClass_allocator;
	};
	using DefaultScriptAllocator = ScriptAllocator<MEM::DefaultBlock>;

	class ScriptSettings
	{
	public:
		ScriptSettings();

		mfuse_EXPORTS void SetDeveloperEnabled(bool value);
		mfuse_EXPORTS bool IsDeveloperEnabled() const;

	private:
		bool bDeveloper;
	};

	class ScriptInterfaces
	{
	public:
		ScriptInterfaces();

	public:
		IFileManagement* fileManagement;
	};

	class ScriptContext : public EventContext, public ThreadCastSingleton<EventContext, ScriptContext>
	{
	public:
		using ThreadCastSingleton::Get;

		mfuse_EXPORTS ScriptContext();
		mfuse_EXPORTS ~ScriptContext();

		mfuse_EXPORTS TargetList& GetTargetList();
		mfuse_EXPORTS TrackedInstances& GetTrackedInstances();
		mfuse_EXPORTS Level* GetLevel();
		mfuse_EXPORTS Game* GetGame();
		mfuse_EXPORTS ScriptMaster& GetDirector();
		mfuse_EXPORTS const ScriptMaster& GetDirector() const;

		mfuse_EXPORTS OutputInfo& GetOutputInfo();
		mfuse_EXPORTS const OutputInfo& GetOutputInfo() const;
		mfuse_EXPORTS ScriptInterfaces& GetScriptInterfaces();
		mfuse_EXPORTS const ScriptInterfaces& GetScriptInterfaces() const;

		mfuse_EXPORTS void Execute(float timeScale = 1.f);
		mfuse_EXPORTS bool IsIdle() const;

		mfuse_EXPORTS DefaultScriptAllocator& GetAllocator();
		mfuse_EXPORTS ScriptSettings& GetSettings();
		mfuse_EXPORTS const ScriptSettings& GetSettings() const;

	private:
		// put it first so instances are freed last
		TrackedInstances trackedInstances;
		Game game;
		Level level;
		TargetList targetList;
		ScriptMaster director;
		OutputInfo outputInfo;
		ScriptInterfaces interfaces;
		DefaultScriptAllocator scriptAllocator;
		ScriptSettings settings;
	};

	mfuse_TEMPLATE template class mfuse_EXPORTS ThreadCastSingleton<EventContext, ScriptContext>;
};
