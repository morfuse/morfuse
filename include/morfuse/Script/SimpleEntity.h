#pragma once

#include "Listener.h"
#include "../Common/Vector.h"
#include "../Common/str.h"
#include "Components/TargetComponent.h"

namespace mfuse
{
	extern EventDef EV_SetAngles;
	extern EventDef EV_SetAngle;
	extern EventDef EV_SetOrigin;

	typedef unsigned int entflags_t;

	class Game;
	class Level;
	class World;

	class SimpleEntity;
	typedef SafePtr< SimpleEntity > SimpleEntityPtr;

	class SimpleEntity : public Listener
	{
		MFUS_CLASS_PROTOTYPE(SimpleEntity);

	public:
		SimpleEntity();
		~SimpleEntity();

		void SimpleArchive(Archiver& arc);
		void Archive(Archiver& arc) override;

		virtual void setOrigin(Vector origin);
		virtual void setOriginEvent(Vector origin);
		virtual void setAngles(Vector angles);

		const Vector& getAngles() const;
		const Vector& getOrigin() const;
		TargetComponent& GetTargetComponent();

		void EventGetAngle(Event *ev);
		void EventGetAngles(Event *ev);
		void EventGetOrigin(Event *ev);
		void EventGetTargetname(Event *ev);
		void EventGetTarget(Event *ev);

		void EventSetAngle(Event *ev);
		void EventSetAngles(Event *ev);
		void EventSetOrigin(Event *ev);
		void EventSetTargetname(Event *ev);
		void EventSetTarget(Event *ev);

		void GetCentroid(Event *ev);

		void GetForwardVector(Event *ev);
		void GetLeftVector(Event *ev);
		void GetRightVector(Event *ev);
		void GetUpVector(Event *ev);

	private:
		// Base coord variable
		Vector origin;
		Vector angles;

		// Used by scripts
		TargetComponent targetComp;
		str target;

		// Centered origin based on mins/maxs
		Vector centroid;
	};
};
