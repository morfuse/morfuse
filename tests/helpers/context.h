#include <morfuse/Script/Context.h>
#include <morfuse/Common/OutputInfo.h>

#include <iostream>

void setupContext(mfuse::ScriptContext& context)
{
	// Uncomment to debug the parser
	//mfuse::GlobalOutput::Get().SetOutputStream(mfuse::outputLevel_e::Verbose, &std::cout);
	mfuse::GlobalOutput::Get().SetOutputStream(mfuse::outputLevel_e::Debug, &std::cout);
	mfuse::GlobalOutput::Get().SetOutputStream(mfuse::outputLevel_e::Warn, &std::cout);
	mfuse::GlobalOutput::Get().SetOutputStream(mfuse::outputLevel_e::Error, &std::cout);
	mfuse::GlobalOutput::Get().SetOutputStream(mfuse::outputLevel_e::Output, &std::cout);

	// Initialize the event system
	mfuse::EventSystem::Get();

	context.EventContext::Set(&context);
	//context.GetOutputInfo().SetOutputStream(mfuse::outputLevel_e::Verbose, &std::cout);
	context.GetOutputInfo().SetOutputStream(mfuse::outputLevel_e::Debug, &std::cout);
	context.GetOutputInfo().SetOutputStream(mfuse::outputLevel_e::Warn, &std::cout);
	context.GetOutputInfo().SetOutputStream(mfuse::outputLevel_e::Error, &std::cout);
	context.GetOutputInfo().SetOutputStream(mfuse::outputLevel_e::Output, &std::cout);

	mfuse::ScriptSettings& settings = context.GetSettings();
	settings.SetDeveloperEnabled(true);
}
