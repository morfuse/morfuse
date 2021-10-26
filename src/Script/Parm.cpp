#include <morfuse/Script/Parm.h>
#include <morfuse/Script/Event.h>
#include <morfuse/Script/ScriptThread.h>
#include <morfuse/Script/ScriptMaster.h>
#include <morfuse/Script/Context.h>

using namespace mfuse;

EventDef EV_Parm_GetOther
(
	"other",
	EV_DEFAULT,
	NULL,
	NULL,
	"other",
	evType_e::Normal
);

/*
EventDef EV_Parm_GetOwner
(
	"owner",
	EV_DEFAULT,
	NULL,
	NULL,
	"owner",
	evType_e::Getter
);
*/

EventDef EV_Parm_GetPreviousThread
(
	"previousthread",
	EV_DEFAULT,
	NULL,
	NULL,
	"previousthread",
	evType_e::Getter
);

void Parm::Archive(Archiver& arc)
{
	Listener::Archive(arc);

	//arc.ArchiveSafePointer(&other);
	//arc.ArchiveSafePointer(&owner);
}

void Parm::GetOther(Event& ev)
{
	ev.AddListener(other);
}

void Parm::GetOwner(Event& ev)
{
	ev.AddListener(owner);
}

void Parm::GetPreviousThread(Event& ev)
{
	ev.AddListener(ScriptContext::Get().GetDirector().PreviousThread());
}

MFUS_CLASS_DECLARATION(Listener, Parm, NULL)
{
	{ &EV_Parm_GetOther,				&Parm::GetOther },
	{ &EV_Listener_GetOwner,			&Parm::GetOwner },
	{ &EV_Parm_GetPreviousThread,		&Parm::GetPreviousThread },
	{ NULL, NULL }
};

