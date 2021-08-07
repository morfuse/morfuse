#pragma once

#include "Listener.h"
#include "../Common/Vector.h"
#include "../Common/str.h"

namespace mfuse
{
	extern EventDef EV_SetAngles;
	extern EventDef EV_SetAngle;
	extern EventDef EV_SetOrigin;

	// entity subclass
#define EF_ENTITY				(1<<0)
#define EF_ANIMATE				(1<<1)
#define EF_SENTIENT				(1<<2)
#define EF_PLAYER				(1<<3)
#define EF_ACTOR				(1<<4)
#define EF_ITEM					(1<<5)
#define EF_INVENTORYITEM		(1<<6)
#define EF_WEAPON				(1<<7)
#define EF_PROJECTILE			(1<<8)
#define EF_DOOR					(1<<9)
#define EF_CAMERA				(1<<10)
#define EF_VEHICLE				(1<<11)
#define EF_VEHICLETANK			(1<<12)
#define EF_VEHICLETURRET		(1<<13)
#define EF_TURRET				(1<<14)
#define EF_PATHNODE				(1<<15)
#define EF_WAYPOINT				(1<<16)
#define EF_TEMPWAYPOINT			(1<<17)
#define EF_VEHICLEPOINT			(1<<18)
#define EF_SPLINEPATH			(1<<19)
#define EF_CRATEOBJECT			(1<<20)
#define EF_BOT					(1<<21)

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

		const Vector& getOrigin() const;
		void SetTarget(const xstr& target);
		void SetTargetName(const xstr& targetname);
		const xstr& Target();
		const xstr& TargetName();

		SimpleEntity *Next(void);

		int IsSubclassOfEntity(void) const;
		int IsSubclassOfAnimate(void) const;
		int IsSubclassOfSentient(void) const;
		int IsSubclassOfPlayer(void) const;
		int IsSubclassOfActor(void) const;
		int IsSubclassOfItem(void) const;
		int IsSubclassOfInventoryItem(void) const;
		int IsSubclassOfWeapon(void) const;
		int IsSubclassOfProjectile(void) const;
		int IsSubclassOfDoor(void) const;
		int IsSubclassOfCamera(void) const;
		int IsSubclassOfVehicle(void) const;
		int IsSubclassOfVehicleTank(void) const;
		int IsSubclassOfVehicleTurretGun(void) const;
		int IsSubclassOfTurretGun(void) const;
		int IsSubclassOfPathNode(void) const;
		int IsSubclassOfWaypoint(void) const;
		int IsSubclassOfTempWaypoint(void) const;
		int IsSubclassOfVehiclePoint(void) const;
		int IsSubclassOfSplinePath(void) const;
		int IsSubclassOfCrateObject(void) const;
		int IsSubclassOfBot(void) const;

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

		// Flag (used to set a class flag)
		entflags_t entflags;

		// Used by scripts
		xstr target;
		xstr targetname;

		// Centered origin based on mins/maxs
		Vector centroid;
	};
};
