#include <morfuse/Script/SimpleEntity.h>
#include <morfuse/Script/Level.h>
#include <morfuse/Script/Context.h>
#include <morfuse/Script/Archiver.h>

using namespace mfuse;

EventDef EV_SimpleEntity_GetAngle
(
	"angle",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"get the angles of the entity using just one value.\n"
	"Gets the yaw of the entity or an up and down\n"
	"direction if newAngle is[ 0 - 359 ] or - 1 or - 2",
	evType_e::Getter
);

EventDef mfuse::EV_SetAngle
(
	"angle",
	EV_DEFAULT,
	"f",
	"newAngle",
	"set the angles of the entity using just one value.\n"
	"Sets the yaw of the entity or an up and down\n"
	"direction if newAngle is[ 0 - 359 ] or - 1 or - 2",
	evType_e::Normal
);

EventDef EV_SimpleEntity_SetterAngle
(
	"angle",
	EV_DEFAULT,
	"f",
	"newAngle",
	"set the angles of the entity using just one value.\n"
	"Sets the yaw of the entity or an up and down\n"
	"direction if newAngle is[ 0 - 359 ] or - 1 or - 2",
	evType_e::Setter
);

EventDef EV_SimpleEntity_GetAngles
(
	"angles",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"get the angles of the entity.",
	evType_e::Getter
);

EventDef mfuse::EV_SetAngles
(
	"angles",
	EV_DEFAULT,
	"v[0,360][0,360][0,360]",
	"newAngles",
	"Set the angles of the entity to newAngles.",
	evType_e::Normal
);

EventDef EV_SimpleEntity_SetterAngles
(
	"angles",
	EV_DEFAULT,
	"v[0,360][0,360][0,360]",
	"newAngles",
	"Set the angles of the entity to newAngles.",
	evType_e::Setter
);

EventDef EV_SimpleEntity_GetOrigin
(
	"origin",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"entity's origin",
	evType_e::Getter
);

EventDef mfuse::EV_SetOrigin
(
	"origin",
	EV_DEFAULT,
	"v",
	"newOrigin",
	"Set the origin of the entity to newOrigin.",
	evType_e::Normal
);

EventDef EV_SimpleEntity_SetterOrigin
(
	"origin",
	EV_DEFAULT,
	"v",
	"newOrigin",
	"Set the origin of the entity to newOrigin.",
	evType_e::Setter
);

EventDef EV_SimpleEntity_GetTargetname
(
	"targetname",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"entity's targetname",
	evType_e::Getter
);

EventDef EV_SimpleEntity_SetTargetname
(
	"targetname",
	EV_DEFAULT,
	"s",
	"targetName",
	"set the targetname of the entity to targetName.",
	evType_e::Normal
);

EventDef EV_SimpleEntity_SetterTargetname
(
	"targetname",
	EV_DEFAULT,
	"s",
	"targetName",
	"set the targetname of the entity to targetName.",
	evType_e::Setter
);

EventDef EV_SimpleEntity_GetTarget
(
	"target",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"entity's target",
	evType_e::Getter
);

EventDef EV_SimpleEntity_SetTarget
(
	"target",
	EV_DEFAULT,
	"s",
	"targetname_to_target",
	"target another entity with targetname_to_target.",
	evType_e::Normal
);

EventDef EV_SimpleEntity_SetterTarget
(
	"target",
	EV_DEFAULT,
	"s",
	"targetname_to_target",
	"target another entity with targetname_to_target.",
	evType_e::Setter
);

EventDef EV_SimpleEntity_Centroid
(
	"centroid",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"entity's centroid",
	evType_e::Getter
);

EventDef EV_SimpleEntity_ForwardVector
(
	"forwardvector",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"get the forward vector of angles",
	evType_e::Getter
);

EventDef EV_SimpleEntity_LeftVector
(
	"leftvector",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"get the left vector of angles",
	evType_e::Getter
);

EventDef EV_SimpleEntity_RightVector
(
	"rightvector",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"get the right vector of angles",
	evType_e::Getter
);

EventDef EV_SimpleEntity_UpVector
(
	"upvector",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"get the up vector of angles",
	evType_e::Getter
);

SimpleEntity::SimpleEntity()
	: targetComp(*this)
{
}

SimpleEntity::~SimpleEntity()
{
}

void SimpleEntity::SimpleArchive(Archiver& arc)
{
	/*
	int index;

	Listener::Archive( arc );

	arc.ArchiveVector( &angles );

	arc.ArchiveString( &target );
	arc.ArchiveString( &targetname );

	if( targetname.length() )
	{
		if( arc.Loading() )
		{
			arc.ArchiveInteger( &index );
			world->AddTargetEntityAt( this, index );
		}
		else
		{
			index = world->GetTargetnameIndex( this );
			arc.ArchiveInteger( &index );
		}
	}
	*/

	arc.ArchiveElements((float*)angles, 3);
	targetComp.Archive(arc);
}

void SimpleEntity::Archive(Archiver& arc)
{
	SimpleEntity::SimpleArchive(arc);

	arc.ArchiveElements((float*)origin, 3);
	arc.ArchiveElements((float*)centroid, 3);
}

void SimpleEntity::setOrigin(Vector origin)
{
	this->origin = origin;
	this->centroid = origin;
}

void SimpleEntity::setOriginEvent(Vector origin)
{
	setOrigin(origin);
}

void SimpleEntity::setAngles(Vector angles)
{
	this->angles = angles.AnglesMod();
}

const mfuse::Vector& SimpleEntity::getAngles() const
{
	return angles;
}

const mfuse::Vector& SimpleEntity::getOrigin() const
{
	return origin;
}

TargetComponent& SimpleEntity::GetTargetComponent()
{
	return targetComp;
}

void SimpleEntity::EventGetAngle(Event& ev)
{
	ev.AddFloat(angles[1]);
}

void SimpleEntity::EventGetAngles(Event& ev)
{
	ev.AddVector(angles);
}

void SimpleEntity::EventGetOrigin(Event& ev)
{
	ev.AddVector(origin);
}

void SimpleEntity::EventGetTargetname(Event& ev)
{
	ev.AddConstString(targetComp.GetTargetName());
}

void SimpleEntity::EventGetTarget(Event& ev)
{
	ev.AddConstString(targetComp.GetTarget());
}

void SimpleEntity::EventSetAngle(Event& ev)
{
	Vector dir;
	float angle = ev.GetFloat(1);

	dir = GetMovedir(angle);
	dir.toAngles();

	setAngles(dir);
}

void SimpleEntity::EventSetAngles(Event& ev)
{
	Vector angles;

	if (ev.NumArgs() == 1)
	{
		angles = ev.GetVector(1);
	}
	else
	{
		angles = Vector(ev.GetFloat(1), ev.GetFloat(2), ev.GetFloat(3));
	}

	setAngles(angles);
}

void SimpleEntity::EventSetOrigin(Event& ev)
{
	setOriginEvent(ev.GetVector(1));
}

void SimpleEntity::EventSetTargetname(Event& ev)
{
	targetComp.SetTargetName(ev.GetConstString(1));
}

void SimpleEntity::EventSetTarget(Event& ev)
{
	targetComp.SetTarget(ev.GetConstString(1));
}

void SimpleEntity::GetCentroid(Event& ev)
{
	ev.AddVector(centroid);
}

void SimpleEntity::GetForwardVector(Event& ev)
{
	Vector fwd;

	angles.AngleVectorsLeft(&fwd, nullptr, nullptr);
	ev.AddVector(fwd);
}

void SimpleEntity::GetLeftVector(Event& ev)
{
	Vector left;

	angles.AngleVectorsLeft(nullptr, &left, nullptr);
	ev.AddVector(left);
}

void SimpleEntity::GetRightVector(Event& ev)
{
	Vector right;

	angles.AngleVectors(nullptr, &right, nullptr);
	ev.AddVector(right);
}

void SimpleEntity::GetUpVector(Event& ev)
{
	Vector up;

	angles.AngleVectorsLeft(nullptr, nullptr, &up);
	ev.AddVector(up);
}

MFUS_CLASS_DECLARATION(Listener, SimpleEntity, nullptr)
{
	{ &EV_SimpleEntity_GetAngle,				&SimpleEntity::EventGetAngle },
	{ &EV_SimpleEntity_GetAngles,				&SimpleEntity::EventGetAngles },
	{ &EV_SimpleEntity_GetOrigin,				&SimpleEntity::EventGetOrigin },
	{ &EV_SimpleEntity_GetTargetname,			&SimpleEntity::EventGetTargetname },
	{ &EV_SimpleEntity_GetTarget,				&SimpleEntity::EventGetTarget },
	{ &EV_SimpleEntity_SetterAngle,				&SimpleEntity::EventSetAngle },
	{ &EV_SimpleEntity_SetterAngles,			&SimpleEntity::EventSetAngles },
	{ &EV_SimpleEntity_SetterOrigin,			&SimpleEntity::EventSetOrigin },
	{ &EV_SimpleEntity_SetterTargetname,		&SimpleEntity::EventSetTargetname },
	{ &EV_SimpleEntity_SetterTarget,			&SimpleEntity::EventSetTarget },
	{ &EV_SetAngle,								&SimpleEntity::EventSetAngle },
	{ &EV_SetAngles,							&SimpleEntity::EventSetAngles },
	{ &EV_SetOrigin,							&SimpleEntity::EventSetOrigin },
	{ &EV_SimpleEntity_SetTargetname,			&SimpleEntity::EventSetTargetname },
	{ &EV_SimpleEntity_SetTarget,				&SimpleEntity::EventSetTarget },
	{ &EV_SimpleEntity_Centroid,				&SimpleEntity::GetCentroid },
	{ &EV_SimpleEntity_ForwardVector,			&SimpleEntity::GetForwardVector },
	{ &EV_SimpleEntity_LeftVector,				&SimpleEntity::GetLeftVector },
	{ &EV_SimpleEntity_RightVector,				&SimpleEntity::GetRightVector },
	{ &EV_SimpleEntity_UpVector,				&SimpleEntity::GetUpVector },
	{ nullptr, nullptr }
};
