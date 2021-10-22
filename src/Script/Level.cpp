#include <morfuse/Script/Level.h>

using namespace mfuse;

Level::Level()
{
}

MFUS_CLASS_DECLARATION(Listener, Level, NULL)
{
	{ NULL, NULL }
};

const mfuse::str& Level::GetCurrentScript() const
{
	return currentScript;
}

void Level::SetCurrentScript(const rawchar_t* name)
{
	currentScript = name;
}
