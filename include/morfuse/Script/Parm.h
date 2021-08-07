#pragma once

#include "Listener.h"

namespace mfuse
{
	class Parm : public Listener
	{

	public:
		MFUS_CLASS_PROTOTYPE(Parm);

		void Archive(Archiver& arc) override;

		void GetOther(Event *ev);
		void GetOwner(Event *ev);
		void GetPreviousThread(Event* ev);

	private:
		ListenerPtr other;
		ListenerPtr owner;

		// Failure variables
		bool movedone;
		bool movefail;
		bool motionfail;
		bool upperfail;
		bool sayfail;
	};
}
