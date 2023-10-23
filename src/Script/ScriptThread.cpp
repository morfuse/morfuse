#include <morfuse/Common/ConstStr.h>
#include <morfuse/Script/ScriptThread.h>
#include <morfuse/Script/ScriptClass.h>
#include <morfuse/Script/ScriptVM.h>
#include <morfuse/Script/ScriptException.h>
#include <morfuse/Script/Level.h>
#include <morfuse/Script/SimpleEntity.h>
#include <morfuse/Script/EventQueue.h>
#include <morfuse/Script/EventSystem.h>
#include <morfuse/Script/Context.h>
#include <morfuse/Script/ScriptMaster.h>
#include <morfuse/Script/PredefinedString.h>
#include <morfuse/Script/Archiver.h>
#include <morfuse/Script/SpawnArgs.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

namespace mfuse
{
EventDef EV_ScriptThread_Abs
(
	"abs",
	EV_DEFAULT,
	"f",
	"arg",
	"Absolute value of int or float",
	evType_e::Return
);

EventDef EV_ScriptThread_AnglesPointAt
(
	"angles_pointat",
	EV_DEFAULT,
	"eee",
	"parent_entity entity target_entity",
	"Returns the angles that points at the target_entity given the base orientation of the parent_entity and the position of the entity.",
	evType_e::Return
);

EventDef EV_ScriptThread_AnglesToForward
(
	"angles_toforward",
	EV_DEFAULT,
	"v",
	"angles",
	"Returns the forward vector of the specified angles",
	evType_e::Return
);

EventDef EV_ScriptThread_AnglesToLeft
(
	"angles_toleft",
	EV_DEFAULT,
	"v",
	"angles",
	"Returns the left vector of the specified angles",
	evType_e::Return
);

EventDef EV_ScriptThread_AnglesToUp
(
	"angles_toup",
	EV_DEFAULT,
	"v",
	"angles",
	"Returns the up vector of the specified angles",
	evType_e::Return
);

EventDef EV_ScriptThread_Assert
(
	"assert",
	EV_DEFAULT,
	"b",
	"value",
	"Assert if value is 0.",
	evType_e::Normal
);

EventDef EV_ScriptThread_CastBoolean
(
	"bool",
	EV_DEFAULT,
	"i",
	"value",
	"Casts value to a bool.",
	evType_e::Return
);

EventDef EV_ScriptThread_CastFloat
(
	"float",
	EV_DEFAULT,
	"i",
	"value",
	"Casts value to a float.",
	evType_e::Return
);

EventDef EV_ScriptThread_CastInt
(
	"int",
	EV_DEFAULT,
	"i",
	"value",
	"Casts value to an int.",
	evType_e::Return
);

EventDef EV_ScriptThread_CastString
(
	"string",
	EV_DEFAULT,
	"i",
	"value",
	"Casts value to a string.",
	evType_e::Return
);

EventDef EV_ScriptThread_CreateListener
(
	"CreateListener",
	EV_DEFAULT,
	NULL,
	NULL,
	"Creates a Listener instance.",
	evType_e::Return
);

EventDef EV_ScriptThread_End
(
	"end",
	EV_DEFAULT,
	NULL,
	NULL,
	"Ends the script",
	evType_e::Normal
);

EventDef EV_ScriptThread_Timeout
(
	"timeout",
	EV_DEFAULT,
	"f",
	"time",
	"specifies script timeout time",
	evType_e::Normal
);

EventDef EV_ScriptThread_Error
(
	"error",
	EV_DEFAULT,
	NULL,
	NULL,
	"Ends the script",
	evType_e::Normal
);

EventDef EV_ScriptThread_GetSelf
(
	"self",
	EV_DEFAULT,
	NULL,
	NULL,
	"self",
	evType_e::Getter
);

EventDef EV_ScriptThread_Goto
(
	"goto",
	EV_DEFAULT,
	"s",
	"label",
	"Goes to the specified label.",
	evType_e::Normal
);

EventDef EV_ScriptThread_MPrint
(
	"mprint",
	EV_DEFAULT,
	"s",
	"string",
	"Prints a string.",
	evType_e::Normal
);

EventDef EV_ScriptThread_MPrintln
(
	"mprintln",
	EV_DEFAULT,
	"s",
	"string",
	"Prints a string. followed by a newline",
	evType_e::Normal
);

EventDef EV_ScriptThread_Print
(
	"print",
	EV_DEFAULT,
	"s",
	"string",
	"Prints a string.",
	evType_e::Normal
);

EventDef EV_ScriptThread_Println
(
	"println",
	EV_DEFAULT,
	"s",
	"string",
	"Prints a string. followed by a newline.",
	evType_e::Normal
);

EventDef EV_ScriptThread_RandomFloat
(
	"randomfloat",
	EV_DEFAULT,
	"f",
	"max",
	"randomfloat",
	evType_e::Return
);

EventDef EV_ScriptThread_RandomInt
(
	"randomint",
	EV_DEFAULT,
	"i",
	"max",
	"randomint",
	evType_e::Return
);

EventDef EV_ScriptThread_RegisterCommand
(
	"registercmd",
	EV_DEFAULT,
	"ss",
	"name label",
	"Registers a command.",
	evType_e::Normal
);
EventDef EV_ScriptThread_KillClass
(
	"killclass",
	EV_CHEAT,
	"sI",
	"class_name except",
	"Kills everything in the specified class except for the specified entity (optional)."
);
EventDef EV_ScriptThread_RemoveClass
(
	"removeclass",
	EV_CHEAT,
	"sI",
	"class_name except",
	"Removes everything in the specified class except for the specified entity (optional)."
);

EventDef EV_ScriptThread_Trigger
(
	"trigger",
	EV_DEFAULT,
	"s",
	"name",
	"Trigger the specified target or entity."
);

EventDef EV_ScriptThread_Spawn
(
	"spawn",
	EV_DEFAULT,
	"sSSSSSSSS",
	"entityname keyname1 value1 keyname2 value2 keyname3 value3 keyname4 value4",
	"Spawns an entity.",
	evType_e::Normal
);

EventDef EV_ScriptThread_SpawnReturn
(
	"spawn",
	EV_DEFAULT,
	"sSSSSSSSS",
	"entityname keyname1 value1 keyname2 value2 keyname3 value3 keyname4 value4",
	"Spawns an entity.",
	evType_e::Return
);

EventDef EV_ScriptThread_VectorAdd
(
	"vector_add",
	EV_DEFAULT,
	"vv",
	"vector1 vector2",
	"Returns vector1 + vector2.",
	evType_e::Return
);

EventDef EV_ScriptThread_VectorCloser
(
	"vector_closer",
	EV_DEFAULT,
	"vvv",
	"vec_a vec_b vec_c",
	"returns 1 if the first vector is closer than the second vector to the third vector.",
	evType_e::Return
);

EventDef EV_ScriptThread_VectorCross
(
	"vector_cross",
	EV_DEFAULT,
	"vv",
	"vector1 vector2",
	"Returns vector1 x vector2.",
	evType_e::Return
);

EventDef EV_ScriptThread_VectorDot
(
	"vector_dot",
	EV_DEFAULT,
	"vv",
	"vector1 vector2",
	"Returns vector1 * vector2.",
	evType_e::Return
);

EventDef EV_ScriptThread_VectorLength
(
	"vector_length",
	EV_DEFAULT,
	"v",
	"vector",
	"Returns the length of the specified vector.",
	evType_e::Return
);

EventDef EV_ScriptThread_VectorNormalize
(
	"vector_normalize",
	EV_DEFAULT,
	"v",
	"vector",
	"Returns the normalized vector of the specified vector.",
	evType_e::Return
);

EventDef EV_ScriptThread_VectorScale
(
	"vector_scale",
	EV_DEFAULT,
	"vf",
	"vector1 scale_factor",
	"Returns vector1 * scale_factor.",
	evType_e::Return
);

EventDef EV_ScriptThread_VectorSubtract
(
	"vector_subtract",
	EV_DEFAULT,
	"vv",
	"vector1 vector2",
	"Returns vector1 - vector2.",
	evType_e::Return
);

EventDef EV_ScriptThread_VectorToAngles
(
	"vector_toangles",
	EV_DEFAULT,
	"v",
	"vector1",
	"Returns vector1 converted to angles.",
	evType_e::Return
);

EventDef EV_ScriptThread_VectorWithin
(
	"vector_within",
	EV_DEFAULT,
	"vvf",
	"position1 position2 distance",
	"returns 1 if the two points are <= distance apart, or 0 if they are greater than distance apart.",
	evType_e::Return
);

EventDef EV_ScriptThread_Pause
(
	"pause",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Pauses the thread.",
	evType_e::Normal
);

EventDef EV_ScriptThread_Wait
(
	"wait",
	EV_DEFAULT,
	"f",
	"wait_time",
	"Wait for the specified amount of time.",
	evType_e::Normal
);

EventDef EV_ScriptThread_WaitFrame
(
	"waitframe",
	EV_DEFAULT,
	NULL,
	NULL,
	"Wait for one server frame.",
	evType_e::Normal
);

//
// world stuff
//
EventDef EV_Cache
(
	"cache",
	EV_CACHE,
	"s",
	"resourceName",
	"pre-cache the given resource."
);

EventDef EV_ScriptThread_IsArray
(
	"isarray",
	EV_DEFAULT,
	"s",
	"object",
	"Checks whether this variable is an array",
	evType_e::Return
);

EventDef EV_ScriptThread_IsDefined
(
	"isdefined",
	EV_DEFAULT,
	"s",
	"object",
	"Checks whether this entity/variable is defined",
	evType_e::Return
);

/*
EventDef EV_ScriptThread_FileOpen
(
	"fopen",
	EV_DEFAULT,
	"ss",
	"filename accesstype",
	"Opens file, returning it's handle",
	evType_e::Return
);

EventDef EV_ScriptThread_FileWrite
(
	"fwrite",
	EV_DEFAULT,
	"eii",
	"player index virtual",
	"Writes binary buffer to file",
	evType_e::Normal
);

EventDef EV_ScriptThread_FileRead
(
	"fread",
	EV_DEFAULT,
	"eii",
	"player index virtual",
	"Reads binary buffer from file",
	evType_e::Normal
);

EventDef EV_ScriptThread_FileClose
(
	"fclose",
	EV_DEFAULT,
	"i",
	"filehandle",
	"Closes file of given file handle",
	evType_e::Return
);

EventDef EV_ScriptThread_FileEof
(
	"feof",
	EV_DEFAULT,
	"i",
	"filehandle",
	"Checks for end of file",
	evType_e::Return
);

EventDef EV_ScriptThread_FileSeek
(
	"fseek",
	EV_DEFAULT,
	"iii",
	"filehandle offset startpos",
	"Sets file carret at given position",
	evType_e::Return
);

EventDef EV_ScriptThread_FileTell
(
	"ftell",
	EV_DEFAULT,
	"i",
	"filehandle",
	"Gets current file carret position",
	evType_e::Return
);

EventDef EV_ScriptThread_FileRewind
(
	"frewind",
	EV_DEFAULT,
	"i",
	"filehandle",
	"Rewinds file carret to files beginning",
	evType_e::Normal
);

EventDef EV_ScriptThread_FilePutc
(
	"fputc",
	EV_DEFAULT,
	"ii",
	"filehandle character",
	"Writes single character to file",
	evType_e::Return
);

EventDef EV_ScriptThread_FilePuts
(
	"fputs",
	EV_DEFAULT,
	"is",
	"filehandle text",
	"Writes string line to file",
	evType_e::Return
);

EventDef EV_ScriptThread_FileGetc
(
	"fgetc",
	EV_DEFAULT,
	"i",
	"filehandle",
	"Reads single character from file",
	evType_e::Return
);

EventDef EV_ScriptThread_FileGets
(
	"fgets",
	EV_DEFAULT,
	"ii",
	"filehandle maxbuffsize",
	"Reads string line from file",
	evType_e::Return
);

EventDef EV_ScriptThread_FileError
(
	"ferror",
	EV_DEFAULT,
	"i",
	"filehandle",
	"Checks for last file i/o error",
	evType_e::Return
);

EventDef EV_ScriptThread_FileFlush
(
	"fflush",
	EV_DEFAULT,
	"i",
	"filehandle",
	"Flushes given stream. Writes all unsaved data from stream buffer to stream",
	evType_e::Return
);
*/

EventDef EV_ScriptThread_FlagClear
(
	"flag_clear",
	EV_DEFAULT,
	"s",
	"name",
	"Clears and deletes a flag.",
	evType_e::Normal
);

EventDef EV_ScriptThread_FlagInit
(
	"flag_init",
	EV_DEFAULT,
	"s",
	"name",
	"Initializes a flag so it can be set. Flags MUST be initialized before they can be used for the first time.",
	evType_e::Normal
);

EventDef EV_ScriptThread_FlagSet
(
	"flag_set",
	EV_DEFAULT,
	"s",
	"name",
	"Sets a flag which alerts all flag_wait()'s and changes are reflected in flag() checks thereafter.",
	evType_e::Normal
);

EventDef EV_ScriptThread_FlagWait
(
	"flag_wait",
	EV_DEFAULT,
	"s",
	"name",
	"Pauses execution flow until a flag has been set.",
	evType_e::Normal
);

EventDef EV_ScriptThread_Lock
(
	"lock",
	EV_DEFAULT,
	"l",
	"lock",
	"Lock the thread."
);

EventDef EV_ScriptThread_UnLock
(
	"unlock",
	EV_DEFAULT,
	"l",
	"lock",
	"Unlock the thread."
);

EventDef EV_ScriptThread_GetArrayKeys
(
	"getarraykeys",
	EV_DEFAULT,
	"s",
	"array",
	"Retrieves a full list containing the name of arrays",
	evType_e::Return
);

EventDef EV_ScriptThread_GetArrayValues
(
	"getarrayvalues",
	EV_DEFAULT,
	"s",
	"array",
	"Retrieves the full list of an array that was set-up with name, such as local.array[ \"name\" ] and return their values",
	evType_e::Return
);

EventDef EV_ScriptThread_GetTime
(
	"gettime",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets current time",
	evType_e::Return
);

EventDef EV_ScriptThread_GetTimeZone
(
	"gettimezone",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets current time zone",
	evType_e::Return
);

EventDef EV_ScriptThread_PregMatch
(
	"preg_match",
	EV_DEFAULT,
	"ss",
	"pattern subject",
	"Searches subject for a match to the regular expression given in pattern.",
	evType_e::Return
);

EventDef EV_ScriptThread_GetDate
(
	"getdate",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets current date",
	evType_e::Return
);

EventDef EV_ScriptThread_CharToInt
(
	"chartoint",
	EV_DEFAULT,
	"s",
	"character",
	"Converts char to int",
	evType_e::Return
);


EventDef EV_ScriptThread_FileExists
(
	"fexists",
	EV_DEFAULT,
	"s",
	"filename",
	"Checks if file exists",
	evType_e::Return
);

EventDef EV_ScriptThread_FileReadAll
(
	"freadall",
	EV_DEFAULT,
	"i",
	"filehandle",
	"Reads whole file and returns it as string",
	evType_e::Return
);

EventDef EV_ScriptThread_FileSaveAll
(
	"fsaveall",
	EV_DEFAULT,
	"is",
	"filehandle text",
	"Saves whole text to file",
	evType_e::Return
);

EventDef EV_ScriptThread_FileRemove
(
	"fremove",
	EV_DEFAULT,
	"s",
	"filename",
	"Removes the file",
	evType_e::Return
);

EventDef EV_ScriptThread_FileRename
(
	"frename",
	EV_DEFAULT,
	"ss",
	"oldfilename newfilename",
	"Renames the file",
	evType_e::Return
);

EventDef EV_ScriptThread_FileCopy
(
	"fcopy",
	EV_DEFAULT,
	"ss",
	"filename copyfilename",
	"Copies the file",
	evType_e::Return
);

EventDef EV_ScriptThread_FileReadPak
(
	"freadpak",
	EV_DEFAULT,
	"s",
	"filename",
	"Reads file from pak file",
	evType_e::Return
);

EventDef EV_ScriptThread_FileList
(
	"flist",
	EV_DEFAULT,
	"ssi",
	"path extension wantSubs",
	"Returns the list of files with given extension",
	evType_e::Return
);

EventDef EV_ScriptThread_FileNewDirectory
(
	"fnewdir",
	EV_DEFAULT,
	"s",
	"path",
	"Creates a new directory",
	evType_e::Return
);

EventDef EV_ScriptThread_FileRemoveDirectory
(
	"fremovedir",
	EV_DEFAULT,
	"s",
	"path",
	"Removes a directory",
	evType_e::Return
);


EventDef EV_ScriptThread_MathCos
(
	"cos",
	EV_DEFAULT,
	"f",
	"x",
	"Compute cosine",
	evType_e::Return
);

EventDef EV_ScriptThread_MathSin
(
	"sin",
	EV_DEFAULT,
	"f",
	"x",
	"Compute sine",
	evType_e::Return
);

EventDef EV_ScriptThread_MathTan
(
	"tan",
	EV_DEFAULT,
	"f",
	"x",
	"Compute tangent",
	evType_e::Return
);

EventDef EV_ScriptThread_MathACos
(
	"acos",
	EV_DEFAULT,
	"f",
	"x",
	"Compute arc cosine",
	evType_e::Return
);

EventDef EV_ScriptThread_MathASin
(
	"asin",
	EV_DEFAULT,
	"f",
	"x",
	"Compute arc sine",
	evType_e::Return
);

EventDef EV_ScriptThread_MathATan
(
	"atan",
	EV_DEFAULT,
	"f",
	"x",
	"Compute arc tangent",
	evType_e::Return
);

EventDef EV_ScriptThread_MathATan2
(
	"atan2",
	EV_DEFAULT,
	"ff",
	"x y",
	"Compute arc tangent with two parameters",
	evType_e::Return
);

EventDef EV_ScriptThread_MathCosH
(
	"cosh",
	EV_DEFAULT,
	"f",
	"x",
	"Compute hyperbolic cosine",
	evType_e::Return
);

EventDef EV_ScriptThread_MathSinH
(
	"sinh",
	EV_DEFAULT,
	"f",
	"x",
	"Compute hyperbolic sine",
	evType_e::Return
);

EventDef EV_ScriptThread_MathTanH
(
	"tanh",
	EV_DEFAULT,
	"f",
	"x",
	"Compute hyperbolic tangent",
	evType_e::Return
);

EventDef EV_ScriptThread_MathExp
(
	"exp",
	EV_DEFAULT,
	"f",
	"x",
	"Compute exponential function",
	evType_e::Return
);

EventDef EV_ScriptThread_MathFrexp
(
	"frexp",
	EV_DEFAULT,
	"f",
	"x",
	"Get significand and exponent",
	evType_e::Return
);

EventDef EV_ScriptThread_MathLdexp
(
	"ldexp",
	EV_DEFAULT,
	"f",
	"x",
	"Generate number from significand and exponent",
	evType_e::Return
);

EventDef EV_ScriptThread_MathLog
(
	"log",
	EV_DEFAULT,
	"f",
	"x",
	"Compute natural logarithm",
	evType_e::Return
);

EventDef EV_ScriptThread_MathLog10
(
	"log10",
	EV_DEFAULT,
	"f",
	"x",
	"Compute common logarithm",
	evType_e::Return
);

EventDef EV_ScriptThread_MathModf
(
	"modf",
	EV_DEFAULT,
	"f",
	"x",
	"Break into fractional and integral parts",
	evType_e::Return
);

EventDef EV_ScriptThread_MathPow
(
	"pow",
	EV_DEFAULT,
	"ff",
	"x y",
	"Raise to power",
	evType_e::Return
);

EventDef EV_ScriptThread_MathSqrt
(
	"sqrt",
	EV_DEFAULT,
	"f",
	"x",
	"Compute square root",
	evType_e::Return
);

EventDef EV_ScriptThread_MathCeil
(
	"ceil",
	EV_DEFAULT,
	"f",
	"x",
	"Round up value",
	evType_e::Return
);

EventDef EV_ScriptThread_MathFloor
(
	"floor",
	EV_DEFAULT,
	"f",
	"x",
	"Round down value",
	evType_e::Return
);

EventDef EV_ScriptThread_MathFmod
(
	"fmod",
	EV_DEFAULT,
	"f",
	"x",
	"Compute remainder of division",
	evType_e::Return
);

EventDef EV_ScriptThread_strncpy
(
	"strncpy",
	EV_DEFAULT,
	"is",
	"bytes source",
	"Returns the copied string with the specified bytes",
	evType_e::Return
);

EventDef EV_ScriptThread_TypeOf
(
	"typeof",
	EV_DEFAULT,
	"i",
	"variable",
	"Returns the type of variable",
	evType_e::Return
);

EventDef EV_ScriptThread_Md5String
(
	"md5string",
	EV_DEFAULT,
	"s",
	"text",
	"generates MD5 hash of given text",
	evType_e::Return
);

EventDef EV_ScriptThread_SetTimer
(
	"settimer",
	EV_DEFAULT,
	"is",
	"interval script",
	"Sets timer that will execute script after given interval.",
	evType_e::Return
);

EventDef EV_ScriptThread_CancelWaiting
(
	"_cancelwaiting",
	EV_CODEONLY,
	NULL,
	NULL,
	"internal EventDef"
);
};

using namespace mfuse;

ScriptThread::ScriptThread()
{
	m_ScriptVM = nullptr;
	m_ThreadState = threadState_e::Running;
}

ScriptThread::ScriptThread(ScriptClass *scriptClass, const opval_t *pCodePos)
{
	m_ScriptVM = new ScriptVM(scriptClass, pCodePos, this);
	m_ThreadState = threadState_e::Running;
}

ScriptThread::~ScriptThread()
{
	ScriptVM* vm = m_ScriptVM;
	if (!vm)
	{
		//ScriptError("Attempting to delete a dead thread.");
		return;
	}

	m_ScriptVM = nullptr;
	if (m_ThreadState == threadState_e::Timing)
	{
		m_ThreadState = threadState_e::Running;
		ScriptContext::Get().GetDirector().RemoveTiming(this);
	}
	else if (m_ThreadState == threadState_e::Waiting)
	{
		m_ThreadState = threadState_e::Running;
		CancelWaitingAll();
	}

	vm->NotifyDelete();
}

void* ScriptThread::operator new(size_t)
{
	return ScriptContext::Get().GetAllocator().GetBlock<ScriptThread>().Alloc();
}

void ScriptThread::operator delete(void* ptr)
{
	ScriptContext::Get().GetAllocator().GetBlock<ScriptThread>().Free(ptr);
}

void ScriptThread::CreateReturnThread(Event& ev)
{
	m_ScriptVM->GetScriptClass()->CreateReturnThread(ev);
}

void ScriptThread::CreateThread(Event& ev)
{
	m_ScriptVM->GetScriptClass()->CreateThread(ev);
}

void ScriptThread::ExecuteReturnScript(Event& ev)
{
	m_ScriptVM->GetScriptClass()->ExecuteReturnScript(ev);
}

void ScriptThread::ExecuteScript(Event& ev)
{
	m_ScriptVM->GetScriptClass()->ExecuteScript(ev);
}

void ScriptThread::EventCreateListener(Event& ev)
{
	Listener* const l = new Listener();

	ScriptContext& context = ScriptContext::Get();
	// make sure to free the listener afterwards
	context.GetTrackedInstances().Add(l);

	ev.AddListener(l);
}

void ScriptThread::CharToInt(Event& ev)
{
	str c = ev.GetString(1);

	ev.AddInteger(c[0u]);
}

/*
void ScriptThread::FileOpen(Event& ev)
{
	str filename;
	str accesstype;
	FILE *f = NULL;
	rawchar_t buf[16] = { 0 };

	size_t numArgs = ev.NumArgs();

	if (numArgs != 2)
		ScriptError("Wrong arguments count for fopen!\n");

	filename = ev.GetString(1);

	accesstype = ev.GetString(2);

	f = fopen(filename.c_str(), accesstype);

	if (f == NULL)
	{
		ev.AddInteger(0);
		return;
	}
	else
	{
		ev.AddInteger((int)(int64_t)f);
		return;
	}


}

void ScriptThread::FileWrite(Event& ev)
{

}

void ScriptThread::FileRead(Event& ev)
{

}

void ScriptThread::FileClose(Event& ev)
{
	int id = 0;
	int ret = 0;
	FILE *f = NULL;
	char buf[16] = { 0 };

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
		ScriptError("Wrong arguments count for fclose!\n");

	id = ev.GetInteger(1);

	f = (FILE *)id;

	if (f == NULL) {
		ScriptError("File handle is NULL for fclose!\n");
	}

	ret = fclose(f);

	if (ret == 0)
	{
		ev.AddInteger(0);
		return;
	}
	else
	{
		ev.AddInteger(ret);
		return;
	}


}

void ScriptThread::FileEof(Event& ev)
{
	int id = 0;
	int ret = 0;
	FILE *f = NULL;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1) {
		ScriptError("Wrong arguments count for feof!\n");
	}

	id = ev.GetInteger(1);

	f = (FILE *)id;

	ret = feof(f);

	ev.AddInteger(ret);
}

void ScriptThread::FileSeek(Event& ev)
{
	int id = 0;
	int pos = 0;
	long int offset = 0;
	int ret = 0;
	FILE *f = NULL;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 3) {
		ScriptError("Wrong arguments count for fseek!\n");
	}

	id = ev.GetInteger(1);

	f = (FILE *)id;

	offset = ev.GetInteger(2);

	if (offset < 0) {
		ScriptError("Wrong file offset! Should be starting from 0. - fseek\n");
	}

	pos = ev.GetInteger(3);

	if (pos != 0 && pos != 1 && pos != 2) {
		ScriptError("Wrong file offset start! Should be between 0 - 2! - fseek\n");
	}

	ret = fseek(f, offset, pos);

	ev.AddInteger(ret);


}

void ScriptThread::FileTell(Event& ev)
{
	int id = 0;
	long int ret = 0;
	FILE *f = NULL;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1) {
		ScriptError("Wrong arguments count for ftell!\n");
	}

	id = ev.GetInteger(1);

	f = (FILE *)id;

	ret = ftell(f);

	ev.AddInteger(ret);
}

void ScriptThread::FileRewind(Event& ev)
{
	int id = 0;
	long int ret = 0;
	FILE *f = NULL;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1) {
		ScriptError("Wrong arguments count for frewind!\n");
	}

	id = ev.GetInteger(1);

	f = (FILE *)id;

	rewind(f);

}

void ScriptThread::FilePutc(Event& ev)
{
	int id = 0;
	int ret = 0;
	FILE *f = NULL;
	int c = 0;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 2) {
		ScriptError("Wrong arguments count for fputc!\n");
	}

	id = ev.GetInteger(1);

	f = (FILE *)id;

	c = ev.GetInteger(2);

	ret = fputc((char)c, f);

	ev.AddInteger(ret);
}

void ScriptThread::FilePuts(Event& ev)
{
	int id = 0;
	int ret = 0;
	FILE *f = NULL;
	str c;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 2) {
		ScriptError("Wrong arguments count for fputs!\n");
	}

	id = ev.GetInteger(1);

	f = (FILE *)id;

	c = ev.GetString(2);
	//gi.Printf("Putting line into a file\n");
	ret = fputs(c.c_str(), f);
	//gi.Printf("Ret val: %i\n", ret);
	ev.AddInteger(ret);
}

void ScriptThread::FileGetc(Event& ev)
{
	int id = 0;
	int ret = 0;
	FILE *f = NULL;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1) {
		ScriptError("Wrong arguments count for fgetc!\n");
	}

	id = ev.GetInteger(1);

	f = (FILE *)id;

	ret = fgetc(f);

	ev.AddInteger(ret);
}

void ScriptThread::FileGets(Event& ev)
{
	int id = 0;
	int maxCount = 0;
	FILE *f = NULL;
	char *c = NULL;
	char *buff = NULL;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 2) {
		ScriptError("Wrong arguments count for fgets!\n");
	}

	id = ev.GetInteger(1);

	f = (FILE *)id;

	maxCount = ev.GetInteger(2);

	if (maxCount <= 0) {
		ScriptError("Maximum buffer size should be higher than 0! - fgets\n");
	}

	buff = (char*)malloc(maxCount + 1);

	if (buff == NULL)
	{
		ScriptError("Failed to allocate memory during fputs scriptCommand text buffer initialization! Try setting maximum buffer length lower.\n");
		ev.AddInteger(-1);
	}

	memset(buff, 0, maxCount + 1);

	c = fgets(buff, maxCount, f);

	if (c == NULL)
		ev.AddString("");
	else
		ev.AddString(c);

	free(buff);
}

void ScriptThread::FileError(Event& ev)
{
	int id = 0;
	int ret = 0;
	FILE *f = NULL;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1) {
		ScriptError("Wrong arguments count for ferror!\n");
	}

	id = ev.GetInteger(1);

	f = (FILE *)id;

	ret = ferror(f);

	ev.AddInteger(ret);
}

void ScriptThread::FileFlush(Event& ev)
{
	int id = 0;
	int ret = 0;
	FILE *f = NULL;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		ScriptError("Wrong arguments count for fflush!\n");
	}

	id = ev.GetInteger(1);

	f = (FILE *)id;

	ret = fflush(f);

	ev.AddInteger(ret);

}

void ScriptThread::FileExists(Event& ev)
{
	int id = 0;
	FILE *f = 0;
	str filename;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		ScriptError("Wrong arguments count for fexists!\n");
	}

	filename = ev.GetString(1);

	if (filename == NULL)
	{
		ScriptError("Empty file name passed to fexists!\n");
	}

	f = fopen(filename, "r");
	if (f)
	{
		fclose(f);
		ev.AddInteger(1);
	}
	else
	{
		ev.AddInteger(0);
	}

}

void ScriptThread::FileReadAll(Event& ev)
{
	int id = 0;
	FILE *f = NULL;
	char *ret = NULL;
	long currentPos = 0;
	size_t size = 0;
	size_t sizeRead = 0;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		ScriptError("Wrong arguments count for freadall!\n");
	}

	id = ev.GetInteger(1);

	f = (FILE *)id;

	currentPos = ftell(f);
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, currentPos, SEEK_SET);

	ret = (char*)malloc(sizeof(char) * size + 1);

	if (ret == NULL)
	{
		ev.AddInteger(-1);
		ScriptError("Error while allocating memory buffer for file content - freadall!\n");
	}

	sizeRead = fread(ret, 1, size, f);
	ret[sizeRead] = '\0';

	ev.AddString(ret);

	free(ret);
}

void ScriptThread::FileSaveAll(Event& ev)
{
	int id = 0;
	FILE *f = NULL;
	size_t sizeWrite = 0;
	str text;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 2)
	{
		ScriptError("Wrong arguments count for fsaveall!\n");
	}

	id = ev.GetInteger(1);
	f = (FILE *)id;

	text = ev.GetString(2);

	if (text == NULL)
	{
		ev.AddInteger(-1);
		ScriptError("Text to be written is NULL - fsaveall!\n");
	}

	sizeWrite = fwrite(text, 1, strlen(text), f);

	ev.AddInteger((int)sizeWrite);
}

void ScriptThread::FileRemove(Event& ev)
{
	int id = 0;
	int ret = 0;
	str filename;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		ScriptError("Wrong arguments count for fremove!\n");
	}

	filename = ev.GetString(1);

	if (filename == NULL)
	{
		ScriptError("Empty file name passed to fremove!\n");
	}

	ret = remove(filename);

	ev.AddInteger(ret);

}

void ScriptThread::FileRename(Event& ev)
{
	int id = 0;
	int ret = 0;
	str oldfilename, newfilename;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 2)
	{
		ScriptError("Wrong arguments count for frename!\n");
	}

	oldfilename = ev.GetString(1);
	newfilename = ev.GetString(2);

	if (!oldfilename)
	{
		ScriptError("Empty old file name passed to frename!\n");
	}

	if (!newfilename)
	{
		ScriptError("Empty new file name passed to frename!\n");
	}

	ret = rename(oldfilename, newfilename);

	ev.AddInteger(ret);

}

void ScriptThread::FileCopy(Event& ev)
{
	size_t n = 0;
	unsigned int ret = 0;
	str filename, copyfilename;
	FILE *f = NULL, *fCopy = NULL;
	char buffer[4096];

	size_t numArgs = ev.NumArgs();

	if (numArgs != 2)
	{
		//glbs.Printf("Wrong arguments count for fcopy!\n");
		return;
	}

	filename = ev.GetString(1);
	copyfilename = ev.GetString(2);

	if (!filename)
	{
		//glbs.Printf("Empty file name passed to fcopy!\n");
		return;
	}

	if (copyfilename)
	{
		//glbs.Printf("Empty copy file name passed to fcopy!\n");
		return;
	}

	f = fopen(filename, "rb");

	if (f == NULL)
	{
		//glbs.Printf("Could not open \"%s\" for copying - fcopy!\n", filename.c_str());
		ev.AddInteger(-1);
		return;
	}

	fCopy = fopen(copyfilename, "wb");

	if (fCopy == NULL)
	{
		fclose(f);
		//glbs.Printf("Could not open \"%s\" for copying - fcopy!\n", copyfilename.c_str());
		ev.AddInteger(-2);
		return;
	}

	while ((n = fread(buffer, sizeof(char), sizeof(buffer), f)) > 0)
	{
		if (fwrite(buffer, sizeof(char), n, fCopy) != n)
		{
			fclose(f);
			fflush(fCopy);
			fclose(fCopy);
			//glbs.Printf("There was an error while copying files - fcopy!\n");
			ev.AddInteger(-3);
			return;
		}
	}

	fclose(f);
	fflush(fCopy);
	fclose(fCopy);

	ev.AddInteger(0);
}
#if 0
void ScriptThread::FileReadPak(Event& ev)
{
	str filename;
	const char *content = NULL;
	int ret = 0;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for freadpak!\n");
		return;
	}

	filename = ev.GetString(1);

	if (filename == NULL)
	{
		//glbs.Printf("Filename is NULL - freadpak!\n");
		return;
	}

	FilePtr file = GetFileManager()->OpenFile(filename);
	if (file)
	{
		file->ReadBuffer((void**)&content);
	}

	if (content == NULL)
	{
		//glbs.Printf("Error while reading pak file content - freadpak!\n");
		ev.AddInteger(-1);
		return;
	}

	ev.AddString(content);
}

void ScriptThread::FileList(Event& ev)
{
	const char *path = NULL;
	str extension;
	bool wantSubs = 0;
	size_t numFiles = 0;
	FileEntryList list;
	ScriptVariable *ref = new ScriptVariable;
	ScriptVariable *array = new ScriptVariable;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 3)
	{
		//glbs.Printf("Wrong arguments count for flist!\n");
		return;
	}

	path = ev.GetString(1);
	extension = ev.GetString(2);
	wantSubs = ev.GetBoolean(3);

	FileManager* FM = GetFileManager();

	list = FM->ListFilteredFiles(path, extension, wantSubs);
	numFiles = list.GetNumFiles();

	if (numFiles == 0)
	{
		return;
	}

	ref->setRefValue(array);

	for (size_t i = 0; i < numFiles; i++)
	{
		ScriptVariable *indexes = new ScriptVariable;
		ScriptVariable *values = new ScriptVariable;

		indexes->setIntValue((int)i);
		values->setStringValue(list.GetFileEntry(i)->GetRawName());

		ref->setArrayAt(*indexes, *values);
	}

	ev.AddValue(*array);

	return;

}
#endif
void ScriptThread::FileNewDirectory(Event& ev)
{
	str path;
	int ret = 0;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for fnewdir!\n");
		return;
	}

	path = ev.GetString(1);

	if (path == NULL)
	{
		//glbs.Printf("Path is NULL - fnewdir!\n");
		return;
	}

#ifdef WIN32
	//ret = _mkdir(path);
#else
	//ret = mkdir(path, 0777);
#endif

	ev.AddInteger(ret);
	return;
}

void ScriptThread::FileRemoveDirectory(Event& ev)
{
	str path;
	int ret = 0;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for fremovedir!\n");
		return;
	}

	path = ev.GetString(1);

	if (path == NULL)
	{
		//glbs.Printf("Path is NULL - fremovedir!\n");
		return;
	}

#ifdef _WIN32
	ret = _rmdir(path);
#else
	ret = rmdir(path);
#endif

	ev.AddInteger(ret);
	return;
}
*/

void ScriptThread::GetArrayKeys(Event& ev)
{
	const ScriptVariable& array = ev.GetValue(1);

	const size_t arraySize = array.arraysize();

	ScriptVariable constArray;
	// create a const array for holding all keys
	constArray.createConstArrayValue(arraySize);

	size_t i = 1;
	for (ScriptVariableIterator it(array); it; ++it, ++i)
	{
		// add all keys to the array
		constArray[i] = it.GetKey();
	}

	ev.AddValue(std::move(constArray));
}

void ScriptThread::GetArrayValues(Event& ev)
{
	const ScriptVariable& array = ev.GetValue(1);

	const size_t arraySize = array.arraysize();

	ScriptVariable constArray;
	// create a const array for holding all values
	constArray.createConstArrayValue(arraySize);

	size_t i = 1;
	for (ScriptVariableIterator it(array); it; ++it, ++i)
	{
		// add all keys to the array
		constArray[i] = it.GetValue();
	}

	ev.AddValue(std::move(constArray));
}

void ScriptThread::GetDate(Event& ev)
{
	char buff[1024];
	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buff, 64, "%d.%m.%Y %r", timeinfo);

	ev.AddString(buff);
}

void ScriptThread::GetTimeZone(Event& ev)
{
	int gmttime;
	int local;

	time_t rawtime;
	struct tm * timeinfo, *ptm;

	int timediff;
	int tmp;

	tmp = ev.GetInteger(1);

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	local = timeinfo->tm_hour;

	ptm = gmtime(&rawtime);

	gmttime = ptm->tm_hour;

	timediff = local - gmttime;

	ev.AddInteger(timediff);
}

void ScriptThread::PregMatch(Event&)
{
	/*
	slre_cap sl_cap[32];
	int i, j;
	size_t iMaxLength;
	size_t iLength;
	size_t iFoundLength = 0;
	str pattern, subject;
	ScriptVariable index, value, subindex, subvalue;
	ScriptVariable array, subarray;

	memset(sl_cap, 0, sizeof(sl_cap));

	pattern = ev.GetString(1);
	subject = ev.GetString(2);

	iMaxLength = strlen(subject);
	iLength = 0;
	i = 0;

	while (iLength < iMaxLength &&
		(iFoundLength = slre_match(pattern, subject.c_str() + iLength, iMaxLength - iLength, sl_cap, sizeof(sl_cap) / sizeof(sl_cap[0]), 0)) > 0)
	{
		subarray.Clear();

		for (j = 0; sl_cap[j].ptr != NULL; j++)
		{
			char *buffer;

			buffer = (char *)glbs.Malloc(sl_cap[j].len + 1);
			buffer[sl_cap[j].len] = 0;
			strncpy(buffer, sl_cap[j].ptr, sl_cap[j].len);

			subindex.setIntValue(j);
			subvalue.setStringValue(buffer);
			subarray.setArrayAtRef(subindex, subvalue);

			glbs.Free(buffer);

			iLength += sl_cap[j].ptr - subject.c_str();
		}

		index.setIntValue(i);
		array.setArrayAtRef(index, subarray);

		i++;
	}

	ev.AddValue(array);
	*/
}

void ScriptThread::EventIsArray(Event& ev)
{
	ScriptVariable * value = &ev.GetValue(1);

	if (value == NULL)
	{
		return ev.AddInteger(0);
	}

	ev.AddInteger(
		value->GetType() == variableType_e::Array
		|| value->GetType() == variableType_e::ConstArray
		|| value->GetType() == variableType_e::SafeContainer
	);
}

void ScriptThread::EventIsDefined(Event& ev)
{
	ev.AddInteger(!ev.IsNilAt(1));
}

void ScriptThread::FlagClear(Event& ev)
{
	const str name = ev.GetString(1);

	Flag* flag = ScriptContext::Get().GetDirector().flags.FindFlag(name.c_str());

	if (flag == NULL) {
		throw ScriptException("Invalid flag " + name);
	}

	delete flag;
}

void ScriptThread::FlagInit(Event& ev)
{
	const str name = ev.GetString(1);

	Flag* flag = ScriptContext::Get().GetDirector().flags.FindFlag(name.c_str());

	if (flag != NULL)
	{
		flag->Reset();
		return;
	}

	flag = new Flag;
	flag->bSignaled = false;
	flag->flagName = name;
}

void ScriptThread::FlagSet(Event& ev)
{
	const str name = ev.GetString(1);

	Flag* flag = ScriptContext::Get().GetDirector().flags.FindFlag(name.c_str());

	if (flag == NULL) {
		throw ScriptException("Invalid flag '" + name + "'");
	}

	flag->Set();
}

void ScriptThread::FlagWait(Event& ev)
{
	const str name = ev.GetString(1);

	Flag* flag = ScriptContext::Get().GetDirector().flags.FindFlag(name.c_str());

	if (flag == NULL) {
		throw ScriptException("Invalid flag '" + name + "'");
	}

	flag->Wait(this);
}

void ScriptThread::Lock(Event& ev)
{
	ScriptMutex *pMutex = (ScriptMutex *)ev.GetListener(1);

	if (!pMutex) {
		throw ScriptException("Invalid mutex.");
	}

	pMutex->Lock();
}

void ScriptThread::UnLock(Event& ev)
{
	ScriptMutex *pMutex = (ScriptMutex *)ev.GetListener(1);

	if (!pMutex) {
		throw ScriptException("Invalid mutex.");
	}

	pMutex->Unlock();
}

void ScriptThread::MathCos(Event& ev)
{
	double x = 0.0f, res = 0.0f;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for cos!\n");
		return;
	}

	x = (double)ev.GetFloat(1);
	res = cos(x);

	ev.AddFloat((float)res);
}

void ScriptThread::MathSin(Event& ev)
{
	double x = 0.0f, res = 0.0f;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for sin!\n");
		return;
	}

	x = ev.GetFloat(1);
	res = sin(x);

	ev.AddFloat((float)res);
}

void ScriptThread::MathTan(Event& ev)
{
	double x = 0.0f, res = 0.0f;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for tan!\n");
		return;
	}

	x = ev.GetFloat(1);
	res = tan(x);

	ev.AddFloat((float)res);
}

void ScriptThread::MathACos(Event& ev)
{
	double x = 0.0f, res = 0.0f;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for acos!\n");
		return;
	}

	x = ev.GetFloat(1);
	res = acos(x);

	ev.AddFloat((float)res);
}

void ScriptThread::MathASin(Event& ev)
{
	double x = 0.0f, res = 0.0f;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for asin!\n");
		return;
	}

	x = ev.GetFloat(1);
	res = asin(x);

	ev.AddFloat((float)res);
}

void ScriptThread::MathATan(Event& ev)
{
	double x = 0.0f, res = 0.0f;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for atan!\n");
		return;
	}

	x = ev.GetFloat(1);
	res = atan(x);

	ev.AddFloat((float)res);
}

void ScriptThread::MathATan2(Event& ev)
{
	double x = 0.0f, y = 0.0f, res = 0.0f;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 2)
	{
		//glbs.Printf("Wrong arguments count for atan2!\n");
		return;
	}

	y = ev.GetFloat(1);
	x = ev.GetFloat(2);

	res = atan2(y, x);

	ev.AddFloat((float)res);
}

void ScriptThread::MathCosH(Event& ev)
{
	double x = 0.0f, res = 0.0f;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for cosh!\n");
		return;
	}

	x = ev.GetFloat(1);
	res = cosh(x);

	ev.AddFloat((float)res);
}

void ScriptThread::MathSinH(Event& ev)
{
	double x = 0.0f, res = 0.0f;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for sinh!\n");
		return;
	}

	x = ev.GetFloat(1);
	res = sinh(x);

	ev.AddFloat((float)res);
}

void ScriptThread::MathTanH(Event& ev)
{
	double x = 0.0f, res = 0.0f;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for tanh!\n");
		return;
	}

	x = ev.GetFloat(1);
	res = tanh(x);

	ev.AddFloat((float)res);
}

void ScriptThread::MathExp(Event& ev)
{
	double x = 0.0f, res = 0.0f;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for exp!\n");
		return;
	}

	x = ev.GetFloat(1);
	res = exp(x);

	ev.AddFloat((float)res);
}

void ScriptThread::MathFrexp(Event& ev)
{
	double x = 0.0f, res = 0.0f;
	int exp = 0;
	ScriptVariable *ref = new ScriptVariable;
	ScriptVariable *array = new ScriptVariable;
	ScriptVariable *SignificandIndex = new ScriptVariable;
	ScriptVariable *ExponentIndex = new ScriptVariable;
	ScriptVariable *SignificandVal = new ScriptVariable;
	ScriptVariable *ExponentVal = new ScriptVariable;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for frexp!\n");
		return;
	}

	x = ev.GetFloat(1);
	res = frexp(x, &exp);

	ref->setRefValue(array);

	SignificandIndex->setStringValue("significand");
	ExponentIndex->setStringValue("exponent");

	SignificandVal->setFloatValue((float)res);
	ExponentVal->setIntValue(exp);

	ref->setArrayAt(*SignificandIndex, *SignificandVal);
	ref->setArrayAt(*ExponentIndex, *ExponentVal);

	ev.AddValue(*array);
}

void ScriptThread::MathLdexp(Event& ev)
{
	double x = 0.0f, res = 0.0f;
	int exp = 0;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 2)
	{
		//glbs.Printf("Wrong arguments count for ldexp!\n");
		return;
	}

	x = ev.GetFloat(1);
	exp = ev.GetInteger(2);

	res = ldexp(x, exp);

	ev.AddFloat((float)res);
}

void ScriptThread::MathLog(Event& ev)
{
	double x = 0.0f, res = 0.0f;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for log!\n");
		return;
	}

	x = ev.GetFloat(1);
	res = log(x);

	ev.AddFloat((float)res);
}

void ScriptThread::MathLog10(Event& ev)
{
	double x = 0.0f, res = 0.0f;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for log10!\n");
		return;
	}

	x = ev.GetFloat(1);
	res = log10(x);

	ev.AddFloat((float)res);
}

void ScriptThread::MathModf(Event& ev)
{
	double x = 0.0f, res = 0.0f;
	double intpart = 0;
	ScriptVariable *array = new ScriptVariable;
	ScriptVariable *ref = new ScriptVariable;
	ScriptVariable *IntpartIndex = new ScriptVariable;
	ScriptVariable *FractionalIndex = new ScriptVariable;
	ScriptVariable *FractionalVal = new ScriptVariable;
	ScriptVariable *IntpartVal = new ScriptVariable;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for modf!\n");
		return;
	}

	x = ev.GetFloat(1);
	res = modf(x, &intpart);

	ref->setRefValue(array);

	IntpartIndex->setStringValue("intpart");
	FractionalIndex->setStringValue("fractional");
	FractionalVal->setFloatValue((float)res);
	IntpartVal->setFloatValue((float)intpart);

	ref->setArrayAt(*IntpartIndex, *IntpartVal);
	ref->setArrayAt(*FractionalIndex, *FractionalVal);

	ev.AddValue(*array);
}

void ScriptThread::MathPow(Event& ev)
{
	double base = 0.0f, res = 0.0f;
	int exponent = 0;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 2)
	{
		//glbs.Printf("Wrong arguments count for pow!\n");
		return;
	}

	base = ev.GetFloat(1);
	exponent = ev.GetInteger(2);
	res = pow(base, exponent);

	ev.AddFloat((float)res);
}

void ScriptThread::MathSqrt(Event& ev)
{
	double x = 0.0f, res = 0.0f;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for sqrt!\n");
		return;
	}

	x = ev.GetFloat(1);
	res = sqrt(x);

	ev.AddFloat((float)res);
}

void ScriptThread::MathCeil(Event& ev)
{
	double x = 0.0f, res = 0.0f;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for ceil!\n");
		return;
	}

	x = ev.GetFloat(1);
	res = ceil(x);

	ev.AddFloat((float)res);
}

void ScriptThread::MathFloor(Event& ev)
{
	double x = 0.0f, res = 0.0f;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for floor!\n");
		return;
	}

	x = ev.GetFloat(1);
	res = floor(x);

	ev.AddFloat((float)res);
}

void ScriptThread::MathFmod(Event& ev)
{
	double numerator = 0.0f, denominator = 0.0f, res = 0.0f;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 2)
	{
		//glbs.Printf("Wrong arguments count for fmod!\n");
		return;
	}

	numerator = ev.GetFloat(1);
	denominator = ev.GetFloat(2);
	res = fmod(numerator, denominator);

	ev.AddFloat((float)res);
}

/*
int checkMD5(const char *filepath, char *md5Hash)
{
	md5_state_t state;
	md5_byte_t digest[16];
	int di;

	FILE *f = NULL;
	char *buff = NULL;
	size_t filesize = 0;
	size_t bytesread = 0;


	f = fopen(filepath, "rb");

	if (f == NULL)
		return -1;

	fseek(f, 0, SEEK_END);
	filesize = ftell(f);
	rewind(f);

	////glbs.Printf("Size: %i\n", filesize);

	buff = (char *)glbs.Malloc(filesize + 1);

	if (buff == NULL)
	{
		fclose(f);
		Com_Printf("error0\n");
		return -2;
	}

	buff[filesize] = '\0';

	bytesread = fread(buff, 1, filesize, f);

	if (bytesread < filesize)
	{
		glbs.Free(buff);
		fclose(f);
		Com_Printf("error1: %i\n", bytesread);
		return -3;
	}

	fclose(f);

	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)buff, filesize);
	md5_finish(&state, digest);

	for (di = 0; di < 16; ++di)
		sprintf(md5Hash + di * 2, "%02x", digest[di]);


	glbs.Free(buff);

	return 0;
}

int checkMD5String(const char *string, char *md5Hash)
{
	md5_state_t state;
	md5_byte_t digest[16];
	int di;

	char *buff = NULL;
	size_t stringlen = 0;

	stringlen = strlen(string);

	buff = (char *)glbs.Malloc(stringlen + 1);

	if (buff == NULL)
	{
		return -1;
	}

	buff[stringlen] = '\0';
	memcpy(buff, string, stringlen);

	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)buff, stringlen);
	md5_finish(&state, digest);

	for (di = 0; di < 16; ++di)
		sprintf(md5Hash + di * 2, "%02x", digest[di]);


	glbs.Free(buff);

	return 0;
}
*/

void ScriptThread::Md5File(Event&)
{
	/*
	char hash[64];
	str filename = NULL;
	int ret = 0;

	if (ev.NumArgs() != 1)
	{
		ScriptError("Wrong arguments count for md5file!\n");
		return;
	}

	filename = ev.GetString(1);

	ret = checkMD5(filename, hash);
	if (ret != 0)
	{
		ev.AddInteger(-1);
		ScriptError("Error while generating MD5 checksum for file - md5file!\n");
		return;
	}

	ev.AddString(hash);
	*/
}

void ScriptThread::Md5String(Event&)
{
	/*
	char hash[64];
	str text = NULL;
	int ret = 0;

	if (ev.NumArgs() != 1)
	{
		ScriptError("Wrong arguments count for md5string!\n");
		return;
	}

	text = ev.GetString(1);

	ret = checkMD5String(text, hash);
	if (ret != 0)
	{
		ev.AddInteger(-1);
		ScriptError("Error while generating MD5 checksum for strin!\n");
		return;
	}

	ev.AddString(hash);
	*/
}

void ScriptThread::TypeOfVariable(Event& ev)
{
	char *type = NULL;
	ScriptVariable * variable;

	size_t numArgs = ev.NumArgs();

	if (numArgs != 1)
	{
		//glbs.Printf("Wrong arguments count for typeof!\n");
		return;
	}

	variable = (ScriptVariable*)&ev.GetValue(1);
	type = (char*)variable->GetTypeName();

	ev.AddString(type);
}

void ScriptThread::CancelWaiting(Event&)
{
	CancelWaitingAll();
}

void ScriptThread::Archive(Archiver&)
{
}

void ScriptThread::ArchiveInternal(Archiver& arc)
{
	Listener::Archive(arc);

	arc.ArchiveObjectPosition(this);
	m_ScriptVM->Archive(arc);
	arc.ArchiveEnum(m_ThreadState);
}

void ScriptThread::Abs(Event& ev)
{
	ev.AddFloat(fabsf(ev.GetFloat(1)));
}

void ScriptThread::SetTimer(Event& ev)
{
	int interval = -1;
	Event *event;

	if (ev.NumArgs() != 2) {
		throw ScriptException("Wrong arguments count for settimer!");
	}

	interval = ev.GetInteger(1);

	if (interval <= 0)
	{
		ev.AddInteger(1);
		return;
	}

	event = new Event(EV_Listener_ExecuteScript);
	event->AddValue(ev.GetValue(2));

	PostEvent(event, interval);
}

void ScriptThread::Angles_ToForward(Event& ev)
{
	Vector fwd;

	ev.GetVector(1).AngleVectorsLeft(&fwd);
	ev.AddVector(fwd);
}

void ScriptThread::Angles_ToLeft(Event& ev)
{
	Vector left;

	ev.GetVector(1).AngleVectorsLeft(NULL, &left);
	ev.AddVector(left);
}

void ScriptThread::Angles_ToUp(Event& ev)
{
	Vector up;

	ev.GetVector(1).AngleVectorsLeft(NULL, NULL, &up);
	ev.AddVector(up);
}

void ScriptThread::Assert(Event& ev)
{
	assert(ev.GetBoolean(1));
}

void ScriptThread::Cache(Event&)
{
}

void ScriptThread::CastBoolean(Event& ev)
{
	ev.AddInteger(ev.GetBoolean(1));
}

void ScriptThread::CastFloat(Event& ev)
{
	ev.AddFloat(ev.GetFloat(1));
}

void ScriptThread::CastInt(Event& ev)
{
	ev.AddInteger(ev.GetInteger(1));
}

void ScriptThread::CastString(Event& ev)
{
	ev.AddString(ev.GetString(1));
}

void ScriptThread::EventDelayThrow(Event& ev)
{
	if (!m_ScriptVM->m_PrevCodePos) {
		return;
	}

	if (m_ScriptVM->EventThrow(ev))
	{
		if (m_ScriptVM->State() == vmState_e::Idling)
		{
			Wait(0);
		}
		else
		{
			Stop();

			m_ScriptVM->Resume();
		}
	}
	else
	{
		// we make sure this won't get deleted
		SafePtr< ScriptThread > This = this;

		Stop();

		if (!BroadcastEvent(const_str(0), ev))
		{
			m_ScriptVM->GetScriptClass()->EventDelayThrow(ev);
		}

		if (This)
		{
			delete this;
		}
	}
}

void ScriptThread::EventEnd(Event& ev)
{
	if (ev.NumArgs() > 0)
	{
		m_ScriptVM->EndRef(ev.GetValue(1));
	}
	else
	{
		m_ScriptVM->End();
	}
}

void ScriptThread::EventTimeout(Event&)
{
	//Director.maxTime = ev.GetFloat(1) * 1000.0f + 0.5f;
}

void ScriptThread::EventError(Event& ev)
{
	if (ev.NumArgs() <= 1)
	{
		throw ScriptException(ev.GetString(1));
	} else {
		throw ScriptAbortException(ev.GetString(1));
	}
}

void ScriptThread::EventGoto(Event& ev)
{
	m_ScriptVM->EventGoto(ev);

	if (m_ScriptVM->State() == vmState_e::Idling)
	{
		ScriptExecuteInternal();
	}
	else
	{
		Stop();
		m_ScriptVM->Resume();
	}
}

void ScriptThread::EventRegisterCommand(Event&)
{
}

void ScriptThread::EventThrow(Event& ev)
{
	if (!m_ScriptVM->m_PrevCodePos) {
		return;
	}

	if (m_ScriptVM->EventThrow(ev))
	{
		if (m_ScriptVM->State() == vmState_e::Idling)
		{
			ScriptExecuteInternal();
		}
		else
		{
			Stop();

			m_ScriptVM->Resume();
		}
	}
	else
	{
		// we make sure this won't get deleted
		SafePtr< ScriptThread > This = this;

		Stop();

		if (!BroadcastEvent(ConstStrings::Empty, ev))
		{
			m_ScriptVM->GetScriptClass()->EventThrow(ev);
		}

		if (This)
		{
			delete this;
		}
	}
}

void ScriptThread::EventPause(Event& ev)
{
	Pause();
}

void ScriptThread::EventWait(Event& ev)
{
	Wait(uint64_t(ev.GetFloat(1) * 1000.f));
}

void ScriptThread::EventWaitFrame(Event&)
{
	// FIXME: Proper wait?
	Wait(ScriptContext::Get().GetTimeManager().GetTime());
}

void ScriptThread::GetSelf(Event& ev)
{
	ev.AddListener(m_ScriptVM->GetScriptClass()->GetSelf());
}

void ScriptThread::Println(Event& ev)
{
	std::ostream* out = ScriptContext::Get().GetOutputInfo().GetOutput(outputLevel_e::Output);
	if (out)
	{
		const size_t numArgs = ev.NumArgs();
		for (uintptr_t i = 1; i <= numArgs; ++i)
		{
			*out << ev.GetString(i).c_str();
			if (i != numArgs) *out << ' ';
		}

		*out << std::endl;
	}
}

void ScriptThread::Print(Event& ev)
{
	std::ostream* out = ScriptContext::Get().GetOutputInfo().GetOutput(outputLevel_e::Output);
	if (out)
	{
		const size_t numArgs = ev.NumArgs();
		for (uintptr_t i = 1; i <= numArgs; ++i)
		{
			*out << ev.GetString(i).c_str();
			if (i != numArgs) *out << ' ';
		}
	}
}

void ScriptThread::MPrintln(Event& ev)
{
	SimpleEntity *m_Self = (SimpleEntity *)m_ScriptVM->GetScriptClass()->GetSelf();

	if (!m_Self || !m_Self->inheritsFrom(&SimpleEntity::staticclass()))
	{
		return;
	}

	MPrint(ev);
	// FIXME
	//m_Self->MPrintf("\n");
}

void ScriptThread::MPrint(Event&)
{
	SimpleEntity *m_Self = (SimpleEntity *)m_ScriptVM->GetScriptClass()->GetSelf();

	if (!m_Self || !m_Self->inheritsFrom(&SimpleEntity::staticclass()))
	{
		return;
	}
// FIXME
/*
	for (size_t i = 1; i <= ev.NumArgs(); i++)
	{
		m_Self->MPrintf(ev.GetString(i));
	}
*/
}

void ScriptThread::RandomFloat(Event& ev)
{
	ev.AddFloat(Random(ev.GetFloat(1)));
}

void ScriptThread::RandomInt(Event& ev)
{
	ev.AddInteger((int)Random((float)ev.GetInteger(1)));
}

void ScriptThread::Spawn(Event& ev)
{
	SpawnInternal(ev);
}

Listener *ScriptThread::SpawnInternal(Event& ev)
{
	if (ev.NumArgs() <= 0)
	{
		throw ScriptException("Usage: spawn classname [keyname] [value]...");
	}

	const str className = ev.GetString(1);
	SpawnArgs args;
	args.setArg("classname", className);

	const size_t numArgs = ev.NumArgs();
	for (uintptr_t i = 2; i < numArgs; i++)
	{
		args.setArg(ev.GetString(i), ev.GetString(i + 1));
	}

	const rawchar_t* spawntarget = args.getArg("spawntarget");

	ScriptContext& context = ScriptContext::Get();
	if (spawntarget)
	{
		const const_str targetString = context.GetDirector().GetDictionary().Get(spawntarget);
		if (targetString)
		{
			const TargetList& tl = ScriptContext::Get().GetTargetList();
			const Listener* existingListener = tl.GetTarget(targetString);
			if (!existingListener)
			{
				throw TargetListErrors::NoTargetException(targetString);
			}

			// FIXME: refactor, make it more flexible
			const SimpleEntity* ent = dynamic_cast<const SimpleEntity*>(existingListener);
			if (ent)
			{
				const Vector& org = ent->getOrigin();
				const Vector& ang = ent->getAngles();
				args.setArg("origin", str(org[0]) + " " + str(org[1]) + " " + str(org[2]));
				args.setArg("angle", str(ang[1]));
			}
		}
	}

	Listener* const l = args.Spawn();

	context.GetTrackedInstances().Add(l);

	return l;
}

void ScriptThread::SpawnReturn(Event& ev)
{
	Listener *listener = SpawnInternal(ev);

	ev.AddListener(listener);
}

void ScriptThread::EventVectorAdd(Event& ev)
{
	ev.AddVector(ev.GetVector(1) + ev.GetVector(2));
}

void ScriptThread::EventVectorCloser(Event&)
{
}

void ScriptThread::EventVectorCross(Event& ev)
{
	ev.AddVector(Vector::Cross(ev.GetVector(1), ev.GetVector(2)));
}

void ScriptThread::EventVectorDot(Event& ev)
{
	Vector vector1 = ev.GetVector(1), vector2 = ev.GetVector(2);

	ev.AddFloat(Vector::Dot(vector1, vector2));
}

void ScriptThread::EventVectorLength(Event& ev)
{
	ev.AddFloat(ev.GetVector(1).length());
}

void ScriptThread::EventVectorNormalize(Event& ev)
{
	Vector vector = ev.GetVector(1);

	vector.normalize();

	ev.AddVector(vector);
}

void ScriptThread::EventVectorScale(Event& ev)
{
	Vector vector = ev.GetVector(1);

	vector *= ev.GetFloat(2);

	ev.AddVector(vector);
}

void ScriptThread::EventVectorSubtract(Event& ev)
{
	ev.AddVector(ev.GetVector(1) - ev.GetVector(2));
}

void ScriptThread::EventVectorToAngles(Event& ev)
{
	ev.AddVector(ev.GetVector(1).toAngles());
}

void ScriptThread::EventVectorWithin(Event& ev)
{
	Vector delta;
	float dist = ev.GetFloat(3);

	delta = ev.GetVector(1) - ev.GetVector(2);

	// check squared distance
	ev.AddInteger(((delta * delta) < (dist * dist)));
}

void ScriptThread::GetTime(Event& ev)
{
	int timearray[3], gmttime;
	char buff[1024];

	time_t rawtime;
	struct tm * timeinfo, *ptm;

	int timediff;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	timearray[0] = timeinfo->tm_hour;
	timearray[1] = timeinfo->tm_min;
	timearray[2] = timeinfo->tm_sec;

	ptm = gmtime(&rawtime);

	gmttime = ptm->tm_hour;

	timediff = timearray[0] - gmttime;

	sprintf(buff, "%02i:%02i:%02i", (int)timearray[0], (int)timearray[1], (int)timearray[2]);

	ev.AddString(buff);
}

void ScriptThread::Execute()
{
	assert(m_ScriptVM);

	try
	{
		ScriptExecuteInternal();
	}
	catch (ScriptAbortExceptionBase&)
	{
		throw;
	}
	catch (ScriptException&)
	{
	}
}

void ScriptThread::Execute(Event& ev)
{
	assert(m_ScriptVM);

	try
	{
		ScriptVariable returnValue;

		// reserve at least 2 for script VM and the callee
		returnValue.newPointer(2);

		ScriptExecute(ev.GetListView(), returnValue);

		if (!returnValue.IsNone())
		{
			// add the returned value
			ev.AddValue(std::move(returnValue));
		}
	}
	catch (ScriptAbortExceptionBase&)
	{
		throw;
	}
	catch (ScriptException&)
	{
	}
}

void ScriptThread::DelayExecute()
{
	ScriptContext::Get().GetDirector().AddTiming(this, 0);
}

void ScriptThread::DelayExecute(Event& ev)
{
	assert(m_ScriptVM);

	ScriptVariable returnValue;

	m_ScriptVM->SetFastData(ev.GetListView());

	returnValue.newPointer(2);
	m_ScriptVM->m_ReturnValue = returnValue;
	ev.AddValue(returnValue);

	ScriptContext::Get().GetDirector().AddTiming(this, 0);
}

ScriptClass *ScriptThread::GetScriptClass() const
{
	return m_ScriptVM->GetScriptClass();
}

mfuse::ScriptVM* ScriptThread::GetScriptVM() const
{
	return m_ScriptVM;
}

threadState_e ScriptThread::GetThreadState(void)
{
	return m_ThreadState;
}

void ScriptThread::SetThreadState(threadState_e newThreadState)
{
	m_ThreadState = newThreadState;
}

void ScriptThread::StartTiming(uinttime_t time)
{
	Stop();

	m_ThreadState = threadState_e::Timing;

	ScriptContext::Get().GetDirector().AddTiming(this, time);
}

void ScriptThread::StartTiming()
{
	Stop();

	m_ThreadState = threadState_e::Timing;

	ScriptContext& context = ScriptContext::Get();
	context.GetDirector().AddTiming(this, 0);
}

void ScriptThread::StartWaiting()
{
	m_ThreadState = threadState_e::Waiting;
}

void ScriptThread::ScriptExecute(const VarListView& data, ScriptVariable& returnValue)
{
	m_ScriptVM->m_ReturnValue = returnValue;

	ScriptExecuteInternal(data);
}

void ScriptThread::ScriptExecuteInternal(const VarListView& data)
{
	ScriptMaster& Director = ScriptContext::Get().GetDirector();
	const SafePtr<ScriptThread> currentThread = Director.CurrentThread();
	const SafePtr<ScriptThread> previousThread = this;

	// assign us as the current thread
	Director.m_PreviousThread = currentThread;
	Director.m_CurrentThread = this;

	Stop();
	m_ScriptVM->Execute(data);

	// restore the previous values
	Director.m_CurrentThread = currentThread;
	Director.m_PreviousThread = previousThread;

	Director.ExecuteRunning();
}

void ScriptThread::StoppedNotify(void)
{
	// This is invalid and we mustn't get here
	if (m_ScriptVM) {
		delete this;
	}
}

void ScriptThread::StartedWaitFor(void)
{
	Stop();
	StartWaiting();
	m_ScriptVM->Suspend();
}

void ScriptThread::StoppedWaitFor(const_str name, bool bDeleting)
{
	if (!m_ScriptVM)
	{
		return;
	}

	// The thread is deleted if the listener is deleting
	if (bDeleting)
	{
		delete this;
		return;
	}

	CancelEventsOfType(EV_ScriptThread_CancelWaiting);

	if (m_ThreadState == threadState_e::Waiting)
	{
		if (name)
		{
			if (m_ScriptVM->state == vmState_e::Idling)
			{
				Execute();
			}
			else
			{
				m_ScriptVM->Resume();
			}
		}
		else
		{
			StartTiming();
		}
	}
}

void ScriptThread::Pause()
{
	Stop();
	m_ScriptVM->Suspend();
}

void ScriptThread::Stop(void)
{
	if (m_ThreadState == threadState_e::Timing)
	{
		m_ThreadState = threadState_e::Running;
		ScriptContext::Get().GetDirector().RemoveTiming(this);
	}
	else if (m_ThreadState == threadState_e::Waiting)
	{
		m_ThreadState = threadState_e::Running;
		CancelWaitingAll();
	}
}

void ScriptThread::Wait(uinttime_t time)
{
	StartTiming(time);
	m_ScriptVM->Suspend();
}

void ScriptThread::Resume()
{
	SetThreadState(threadState_e::Running);
	m_ScriptVM->Execute();
}

MFUS_CLASS_DECLARATION(Listener, ScriptThread, NULL)
{
	{ &EV_Listener_CreateReturnThread,			&ScriptThread::CreateReturnThread },
	{ &EV_Listener_CreateThread,				&ScriptThread::CreateThread },
	{ &EV_Listener_ExecuteReturnScript,			&ScriptThread::ExecuteReturnScript },
	{ &EV_Listener_ExecuteScript,				&ScriptThread::ExecuteScript },
	{ &EV_ScriptThread_Abs,						&ScriptThread::Abs },
	{ &EV_ScriptThread_AnglesToForward,			&ScriptThread::Angles_ToForward },
	{ &EV_ScriptThread_AnglesToLeft,			&ScriptThread::Angles_ToLeft },
	{ &EV_ScriptThread_AnglesToUp,				&ScriptThread::Angles_ToUp },
	{ &EV_ScriptThread_Assert,					&ScriptThread::Assert },
	{ &EV_Cache,								&ScriptThread::Cache },
	{ &EV_ScriptThread_CastBoolean,				&ScriptThread::CastBoolean },
	{ &EV_ScriptThread_CastFloat,				&ScriptThread::CastFloat },
	{ &EV_ScriptThread_CastInt,					&ScriptThread::CastInt },
	{ &EV_ScriptThread_CastString,				&ScriptThread::CastString },
	{ &EV_ScriptThread_CreateListener,			&ScriptThread::EventCreateListener },
	{ &EV_DelayThrow,							&ScriptThread::EventDelayThrow },
	{ &EV_ScriptThread_End,						&ScriptThread::EventEnd },
	{ &EV_ScriptThread_Timeout,					&ScriptThread::EventTimeout },
	{ &EV_ScriptThread_Error,					&ScriptThread::EventError },
	{ &EV_ScriptThread_GetSelf,					&ScriptThread::GetSelf },
	{ &EV_ScriptThread_Goto,					&ScriptThread::EventGoto },
	{ &EV_ScriptThread_Println,					&ScriptThread::Println },
	{ &EV_ScriptThread_Print,					&ScriptThread::Print },
	{ &EV_ScriptThread_MPrintln,				&ScriptThread::MPrintln },
	{ &EV_ScriptThread_MPrint,					&ScriptThread::MPrint },
	{ &EV_ScriptThread_RandomFloat,				&ScriptThread::RandomFloat },
	{ &EV_ScriptThread_RandomInt,				&ScriptThread::RandomInt },
	{ &EV_ScriptThread_RegisterCommand,			&ScriptThread::EventRegisterCommand },
	{ &EV_ScriptThread_Spawn,					&ScriptThread::Spawn },
	{ &EV_ScriptThread_SpawnReturn,				&ScriptThread::SpawnReturn },
	{ &EV_Throw,								&ScriptThread::EventThrow },
	{ &EV_ScriptThread_VectorAdd,				&ScriptThread::EventVectorAdd },
	{ &EV_ScriptThread_VectorCloser,			&ScriptThread::EventVectorCloser },
	{ &EV_ScriptThread_VectorCross,				&ScriptThread::EventVectorCross },
	{ &EV_ScriptThread_VectorDot,				&ScriptThread::EventVectorDot },
	{ &EV_ScriptThread_VectorLength,			&ScriptThread::EventVectorLength },
	{ &EV_ScriptThread_VectorNormalize,			&ScriptThread::EventVectorNormalize },
	{ &EV_ScriptThread_VectorScale,				&ScriptThread::EventVectorScale },
	{ &EV_ScriptThread_VectorSubtract,			&ScriptThread::EventVectorSubtract },
	{ &EV_ScriptThread_VectorToAngles,			&ScriptThread::EventVectorToAngles },
	{ &EV_ScriptThread_VectorWithin,			&ScriptThread::EventVectorWithin },
	{ &EV_ScriptThread_Pause,					&ScriptThread::EventPause },
	{ &EV_ScriptThread_Wait,					&ScriptThread::EventWait },
	{ &EV_ScriptThread_WaitFrame,				&ScriptThread::EventWaitFrame },
	{ &EV_ScriptThread_IsArray,					&ScriptThread::EventIsArray },
	{ &EV_ScriptThread_IsDefined,				&ScriptThread::EventIsDefined },
	{ &EV_ScriptThread_MathACos,				&ScriptThread::MathACos },
	{ &EV_ScriptThread_MathASin,				&ScriptThread::MathASin },
	{ &EV_ScriptThread_MathATan,				&ScriptThread::MathATan },
	{ &EV_ScriptThread_MathATan2,				&ScriptThread::MathATan2 },
	{ &EV_ScriptThread_MathCeil,				&ScriptThread::MathCeil },
	{ &EV_ScriptThread_MathCos,					&ScriptThread::MathCos },
	{ &EV_ScriptThread_MathCosH,				&ScriptThread::MathCosH },
	{ &EV_ScriptThread_MathExp,					&ScriptThread::MathExp },
	{ &EV_ScriptThread_MathFloor,				&ScriptThread::MathFloor },
	{ &EV_ScriptThread_MathFmod,				&ScriptThread::MathFmod },
	{ &EV_ScriptThread_MathFrexp,				&ScriptThread::MathFrexp },
	{ &EV_ScriptThread_MathLdexp,				&ScriptThread::MathLdexp },
	{ &EV_ScriptThread_MathLog,					&ScriptThread::MathLog },
	{ &EV_ScriptThread_MathLog10,				&ScriptThread::MathLog10 },
	{ &EV_ScriptThread_MathModf,				&ScriptThread::MathModf },
	{ &EV_ScriptThread_MathPow,					&ScriptThread::MathPow },
	{ &EV_ScriptThread_MathSin,					&ScriptThread::MathSin },
	{ &EV_ScriptThread_MathSinH,				&ScriptThread::MathSinH },
	{ &EV_ScriptThread_MathSqrt,				&ScriptThread::MathSqrt },
	{ &EV_ScriptThread_MathTan,					&ScriptThread::MathTan },
	{ &EV_ScriptThread_MathTanH,				&ScriptThread::MathTanH },
	{ &EV_ScriptThread_Md5String,				&ScriptThread::Md5String },
	{ &EV_ScriptThread_TypeOf,					&ScriptThread::TypeOfVariable },
	{ &EV_ScriptThread_CancelWaiting,			&ScriptThread::CancelWaiting },
	{ &EV_ScriptThread_GetTime,					&ScriptThread::GetTime },
	{ &EV_ScriptThread_GetTimeZone,				&ScriptThread::GetTimeZone },
	{ &EV_ScriptThread_PregMatch,				&ScriptThread::PregMatch },
	{ &EV_ScriptThread_FlagClear,				&ScriptThread::FlagClear },
	{ &EV_ScriptThread_FlagInit,				&ScriptThread::FlagInit },
	{ &EV_ScriptThread_FlagSet,					&ScriptThread::FlagSet },
	{ &EV_ScriptThread_FlagWait,				&ScriptThread::FlagWait },
	{ &EV_ScriptThread_Lock,					&ScriptThread::Lock },
	{ &EV_ScriptThread_UnLock,					&ScriptThread::UnLock },

	{ &EV_ScriptThread_CharToInt,				&ScriptThread::CharToInt },
	/*
	{ &EV_ScriptThread_FileClose,				&ScriptThread::FileClose },
	{ &EV_ScriptThread_FileCopy,				&ScriptThread::FileCopy },
	{ &EV_ScriptThread_FileEof,					&ScriptThread::FileEof },
	{ &EV_ScriptThread_FileError,				&ScriptThread::FileError },
	{ &EV_ScriptThread_FileExists,				&ScriptThread::FileExists },
	{ &EV_ScriptThread_FileFlush,				&ScriptThread::FileFlush },
	{ &EV_ScriptThread_FileGetc,				&ScriptThread::FileGetc },
	{ &EV_ScriptThread_FileGets,				&ScriptThread::FileGets },
	//{ &EV_ScriptThread_FileList,				&ScriptThread::FileList },
	{ &EV_ScriptThread_FileNewDirectory,		&ScriptThread::FileNewDirectory },
	{ &EV_ScriptThread_FileOpen,				&ScriptThread::FileOpen },
	{ &EV_ScriptThread_FilePutc,				&ScriptThread::FilePutc },
	{ &EV_ScriptThread_FilePuts,				&ScriptThread::FilePuts },
	{ &EV_ScriptThread_FileRead,				&ScriptThread::FileRead },
	{ &EV_ScriptThread_FileReadAll,				&ScriptThread::FileReadAll },
	{ &EV_ScriptThread_FileRemove,				&ScriptThread::FileRemove },
	{ &EV_ScriptThread_FileRemoveDirectory,		&ScriptThread::FileRemoveDirectory },
	{ &EV_ScriptThread_FileRename,				&ScriptThread::FileRename },
	{ &EV_ScriptThread_FileRewind,				&ScriptThread::FileRewind },
	{ &EV_ScriptThread_FileSaveAll,				&ScriptThread::FileSaveAll },
	{ &EV_ScriptThread_FileSeek,				&ScriptThread::FileSeek },
	{ &EV_ScriptThread_FileTell,				&ScriptThread::FileTell },
	{ &EV_ScriptThread_FileWrite,				&ScriptThread::FileWrite },
	*/
	{ &EV_ScriptThread_GetArrayKeys,			&ScriptThread::GetArrayKeys },
	{ &EV_ScriptThread_GetArrayValues,			&ScriptThread::GetArrayValues },
	{ &EV_ScriptThread_GetDate,					&ScriptThread::GetDate },
	{ &EV_ScriptThread_SetTimer,				&ScriptThread::SetTimer },
	{ NULL, NULL }
};

MFUS_CLASS_DECLARATION(Listener, ScriptMutex, NULL)
{
	{ NULL, NULL }
};

ScriptMutex::ScriptMutex()
{
	m_iLockCount = 0;
	LL::Reset<mutex_thread_list_t*, &mutex_thread_list_t::next, &mutex_thread_list_t::prev>(&m_list);
}

ScriptMutex::~ScriptMutex()
{
	mutex_thread_list_t *list, *next;

	list = m_list.next;
	while (!LL::IsEmpty<mutex_thread_list_t*, &mutex_thread_list_t::next, &mutex_thread_list_t::prev>(&m_list))
	{
		next = list->next;
		LL::Remove<mutex_thread_list_t*, &mutex_thread_list_t::next, &mutex_thread_list_t::prev>(list);
		delete list;
		list = next;
	}
}

void ScriptMutex::setOwner(ScriptThread *pThread)
{
	m_pLockThread = pThread;
}

void ScriptMutex::Lock(mutex_thread_list_t *pList)
{
	ScriptThread *pThread = pList->m_pThread;

	if (!m_pLockThread)
	{
		// Acquire ownership
		setOwner(pThread);
	}
	else
	{
		// Wait for the owner to unlock
		Register(const_str(0), pThread);
	}

	m_iLockCount++;
}

void ScriptMutex::Lock(void)
{
	mutex_thread_list_t *list;

	ScriptMaster& Director = ScriptContext::Get().GetDirector();

	list = new mutex_thread_list_t;
	list->m_pThread = Director.CurrentThread();
	LL::Add<mutex_thread_list_t*, &mutex_thread_list_t::next, &mutex_thread_list_t::prev>(&m_list, list);

	Lock(list);
}

void ScriptMutex::Unlock(void)
{
	mutex_thread_list_t *list, *next;

	ScriptMaster& Director = ScriptContext::Get().GetDirector();

	m_iLockCount--;

	list = m_list.next;
	while (list != &m_list)
	{
		if (list->m_pThread == Director.CurrentThread())
		{
			next = list->next;
			LL::Remove<mutex_thread_list_t*, &mutex_thread_list_t::next, &mutex_thread_list_t::prev>(list);
			delete list;
		}
		else
		{
			next = list->next;
		}

		list = next;
	}

	if (Director.CurrentThread() == m_pLockThread)
	{
		m_pLockThread = NULL;

		if (!LL::IsEmpty<mutex_thread_list_t*, &mutex_thread_list_t::next, &mutex_thread_list_t::prev>(&m_list))
		{
			list = m_list.next;

			setOwner(list->m_pThread);
			Unregister(const_str(0), list->m_pThread);
		}
	}
}

void ScriptMutex::StoppedNotify(void)
{
	ScriptMaster& Director = ScriptContext::Get().GetDirector();

	if (Director.CurrentThread() == m_pLockThread)
	{
		// Safely unlock in case the thread is exiting
		Unlock();
	}
}

Flag *FlagList::FindFlag(const rawchar_t* name)
{
	for (size_t i = 0; i < m_Flags.NumObjects(); i++)
	{
		Flag * index = m_Flags[i];

		// found the flag
		if (!str::cmp(index->flagName.c_str(), name)) {
			return index;
		}
	}

	return NULL;
}

void FlagList::AddFlag(Flag *flag)
{
	m_Flags.AddObject(flag);
}

void FlagList::RemoveFlag(Flag *flag)
{
	m_Flags.RemoveObject(flag);
}

Flag::Flag()
{
	ScriptContext::Get().GetDirector().flags.AddFlag(this);
}

Flag::~Flag()
{
	ScriptContext::Get().GetDirector().flags.RemoveFlag(this);

	m_WaitList.FreeObjectList();
}

void Flag::Reset()
{
	bSignaled = false;
}

void Flag::Set()
{
	// Don't signal again
	if (bSignaled)
	{
		return;
	}

	bSignaled = true;

	for (size_t i = 0; i < m_WaitList.NumObjects(); i++)
	{
		ScriptVM *Thread = m_WaitList[i];

		if (Thread->State() != vmState_e::Destroyed && Thread->GetScriptThread() != NULL) {
			Thread->GetScriptThread()->StoppedWaitFor(ConstStrings::Empty, false);
		}
	}

	// Clear the list
	m_WaitList.FreeObjectList();
}

void Flag::Wait(ScriptThread *Thread)
{
	// Don't wait if it's signaled
	if (bSignaled)
	{
		return;
	}

	Thread->StartedWaitFor();

	m_WaitList.AddObject(Thread->GetScriptVM());
}
