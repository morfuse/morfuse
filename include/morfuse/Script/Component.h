#pragma once

#include "../Global.h"

namespace mfuse
{
	class Listener;

	/** Base component. */
	class mfuse_EXPORTS Component
	{
	public:
		Component(Listener& parentRef);
		virtual ~Component();

		Listener& Parent() const;

	private:
		Listener& parent;
	};
}