#include <morfuse/Script/ClassEventPrinter.h>

#include <fstream>

using namespace mfuse;

int main(int argc, const char* argv[])
{
	ClassEventPrinter printer;
	std::ofstream ostream("allclasses.html", std::ios_base::out | std::ios_base::binary);

	HTMLFormater htmlFormater;
	printer.DumpAllClasses(ostream, htmlFormater, "App Classes");
	return 0;
}
