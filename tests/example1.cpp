#include <morfuse/Script/Context.h>
#include <morfuse/Common/membuf.h>

#include <iostream>

const char sampleScript[] =
"main:\n"
"local.string = \"Hello, world!\"\n"
"println local.string\n"
"end";

int main()
{
	// This example only cover the execution of script

	mfuse::ScriptContext context;
	// Set the stream to use for 'Output'.
	// The script above will call println which will use the 'Output' level
	context.GetOutputInfo().SetOutputStream(mfuse::outputLevel_e::Output, &std::cout);

	mfuse::ScriptMaster& director = context.GetDirector();

	// Instantiate a memory buffer stream (implementation of std::istream)
	// It will be used to read and parse the script from a memory buffer.
	mfuse::imemstream membuf(sampleScript, sizeof(sampleScript));
	
	// This will parse and compile the script into a program buffer (opcodes) that the interpreter will execute
	// The parser will read the script using the buffer above
	const mfuse::ProgramScript* const script = director.GetProgramScript("example", membuf);

	// This will create a thread that will execute the ProgramScript instance above
	// labels define the position at which threads will start
	// If no label is specified (second parameter), the thread starts at position 0 by default
	//
	// If the thread has finished executing, then this method will return null, otherwise, in the case of a wait, it will return an object of the thread
	director.ExecuteThread(script);

	// Output:
	// Hello, world!
	//

	return 0;
}
