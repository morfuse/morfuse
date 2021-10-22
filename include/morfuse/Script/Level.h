#pragma once

#include "Listener.h"
#include "../Common/Vector.h"
#include "../Common/str.h"

namespace mfuse
{
	class SimpleArchivedEntity;

	class Level : public Listener
	{
		MFUS_CLASS_PROTOTYPE(Level);

	public:
		Level();

		const str& GetCurrentScript() const;
		void SetCurrentScript(const rawchar_t* name);

	private:
		str currentScript;
	};
};
