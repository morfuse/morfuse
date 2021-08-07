#include <morfuse/Script/ClassEventPrinter.h>
#include <morfuse/Script/Class.h>
#include <morfuse/Script/Event.h>
#include <morfuse/Common/str.h>
#include <algorithm>
#include <iomanip>

using namespace mfuse;

struct EventCompare
{
	bool operator()(const EventDef* d1, const EventDef* d2)
	{
		return xstr::icmp(d1->GetAttributes().GetString(), d2->GetAttributes().GetString()) < 0;
	}
};

struct ClassCompare
{
	bool operator()(const ClassDef* c1, const ClassDef* c2)
	{
		return xstr::icmp(c1->GetClassName(), c2->GetClassName()) < 0;
	}
};

void ClassEventPrinter::DumpAllClasses(std::ostream& class_stream, AbstractFormater& formater, const rawchar_t* title)
{
	// construct the HTML header for the document
	formater.PrintDocumentHeader(class_stream, title);

	/*
	CLASS_Print(class_file, "<HTML>\n");
	CLASS_Print(class_file, "<HEAD>\n");
	CLASS_Print(class_file, "<Title>%s Classes</Title>\n", class_title.c_str());
	CLASS_Print(class_file, "</HEAD>\n");
	CLASS_Print(class_file, "<BODY>\n");
	CLASS_Print(class_file, "<H1>\n");
	CLASS_Print(class_file, "<center>%s Classes</center>\n", class_title.c_str());
	CLASS_Print(class_file, "</H1>\n");
	*/

	// FIXME: Print commonly used classnames
	/*
	//
	// print out some commonly used classnames
	//
	CLASS_Print(class_file, "<h2>");
	CLASS_Print(class_file, "<a href=\"#Actor\">Actor</a>, ");
	CLASS_Print(class_file, "<a href=\"#Animate\">Animate</a>, ");
	CLASS_Print(class_file, "<a href=\"#Entity\">Entity</a>, ");
	CLASS_Print(class_file, "<a href=\"#ScriptSlave\">ScriptSlave</a>, ");
	CLASS_Print(class_file, "<a href=\"#ScriptThread\">ScriptThread</a>, ");
	CLASS_Print(class_file, "<a href=\"#Sentient\">Sentient</a>, ");
	CLASS_Print(class_file, "<a href=\"#StateMap\">StateMap</a>, ");
	CLASS_Print(class_file, "<a href=\"#Trigger\">Trigger</a>, ");
	CLASS_Print(class_file, "<a href=\"#World\">World</a>");
	CLASS_Print(class_file, "</h2>");
	*/

	con::Container<EventDef*> sortedEvents;
	con::Container<ClassDef*> sortedClasses;

	SortEventList(sortedEvents);
	SortClassList(sortedClasses);

	const size_t num = sortedClasses.NumObjects();

	// go through and process each class from smallest to greatest
	for (uintptr_t i = 1; i <= num; i++)
	{
		ClassDef* c = sortedClasses.ObjectAt(i);
		DumpClass(c, class_stream, formater, sortedEvents);
	}

	//formater.BeginHeading(class_stream);
	//class_stream << sortedClasses.NumObjects() << " " << title << ".";
	//formater.NewLine(class_stream);
	//class_stream << sortedEvents.NumObjects() << " " << title << ".";
	// The formater can add additional information at the end of the document
	formater.PrintDocumentFooter(class_stream);
}

void ClassEventPrinter::SortClassList(con::Container<ClassDef*>& sortedList)
{
	const size_t num = ClassDef::GetNumClasses();
	sortedList.Resize(num);

	for (auto c = ClassDef::GetList(); c; c = c.Next())
	{
		sortedList.AddObject(c);
	}

	std::sort(sortedList.Data(), sortedList.Data() + num, ClassCompare());
}

void ClassEventPrinter::SortEventList(con::Container<EventDef*>& sortedList)
{
	const size_t num = EventDef::GetDefCount();
	sortedList.Resize(num);

	for (auto def = EventDef::GetList(); def; def = def.Next())
	{
		sortedList.AddObject(def);
	}

	std::sort(sortedList.Data(), sortedList.Data() + num, EventCompare());
}

void ClassEventPrinter::DumpClass(ClassDef* classDef, std::ostream& class_stream, AbstractFormater& formater, const con::Container<EventDef*>& sortedEvents)
{
	formater.BeginClassHeader(class_stream, classDef->GetClassName(), classDef->GetClassID());

	// print parent
	for (const ClassDef* c = classDef->GetSuper(); c; c = c->GetSuper())
	{
		formater.PrintClassLineage(class_stream, c->GetClassName());
	}

	formater.EndClassHeader(class_stream);

	formater.BeginClassBody(class_stream);

	const ResponseDefClass* response = classDef->GetResponseList();
	if (response && response->event)
	{
		const size_t sortedNum = sortedEvents.NumObjects();
		for(uintptr_t i = 1; i <= sortedNum; ++i)
		{
			const EventDef* const sortedDef = sortedEvents.ObjectAt(i);

			for (const ResponseDefClass* r = classDef->GetResponseList(); r->event; ++r)
			{
				if (r->response && r->event == sortedDef)
				{
					PrintEventDocumentation(*r->event, class_stream, formater);
					break;
				}
			}
		}
	}

	formater.EndClassBody(class_stream);
}

void ClassEventPrinter::PrintDocumentation(EventDef& def, std::ostream& event_stream, AbstractFormater& formater)
{
	const rawchar_t* name = def.GetAttributes().GetString();

	formater.BeginEventDoc(event_stream, name);

	const con::Container<EventArgDef> definition = SetupDocumentation(def);

	if (definition.NumObjects())
	{
		formater.BeginEventDefinition(event_stream);

		for (size_t i = 1; i <= definition.NumObjects(); i++)
		{
			const EventArgDef& argDef = definition.ObjectAt(i);
			const bool isOptional = argDef.isOptional();

			formater.BeginEventArgument(event_stream, isOptional);

			formater.PrintEventArgumentType(event_stream, argDef.GetTypeName());
			formater.PrintEventArgumentName(event_stream, argDef.getName());

			PrintEventArgumentRange(event_stream, argDef, formater);

			formater.EndEventArgument(event_stream, isOptional, i >= definition.NumObjects());
		}

		formater.EndEventDefinition(event_stream);
	}

	formater.EndEventDoc(event_stream);

	const rawchar_t* documentation = def.GetDocumentation();
	if (documentation)
	{
		formater.PrintEventDocumentation(event_stream, documentation);
	}
}

void ClassEventPrinter::PrintEventArgumentRange(std::ostream& event_file, const EventArgDef& argDef, AbstractFormater& formater)
{
	size_t numRanges;
	bool integer, single;
	argDef.GetRange(numRanges, integer, single);

	for (uintptr_t i = 0; i < numRanges; i++)
	{
		if (single)
		{
			if (!argDef.GetMinRangeDefault(i))
			{
				formater.PrintEventArgumentRangeSingle(event_file, argDef.GetMinRange(i), integer);
			}
		}
		else
		{
			// both non-default
			if (!argDef.GetMinRangeDefault(i) && !argDef.GetMaxRangeDefault(i))
			{
				formater.PrintEventArgumentRange(event_file, argDef.GetMinRange(i), argDef.GetMaxRange(i), integer);
			}
			// max default
			else if (!argDef.GetMinRangeDefault(i) && argDef.GetMaxRangeDefault(i))
			{
				formater.PrintEventArgumentRange(event_file, argDef.GetMinRange(i), -1, integer);
			}
			// min default
			else if (argDef.GetMinRangeDefault(i) && !argDef.GetMaxRangeDefault(i))
			{
				formater.PrintEventArgumentRange(event_file, -1, argDef.GetMaxRange(i), integer);
			}
		}
	}
}

void ClassEventPrinter::PrintEventDocumentation(EventDef& def, std::ostream& event_stream, AbstractFormater& formater)
{
	if (def.GetFlags() & EV_CODEONLY)
	{
		// don't parse non-script events
		return;
	}

	// purposely suppressed
	if (def.GetAttributes().GetString()[0] == '_')
	{
		return;
	}

	PrintDocumentation(def, event_stream, formater);
}

con::Container<EventArgDef> ClassEventPrinter::SetupDocumentation(EventDef& def)
{
	const rawchar_t* name = def.GetAttributes().GetString();
	const rawchar_t* formatspec = def.GetFormatSpec();
	const rawchar_t* argument_names = def.GetArgumentNames();

	con::Container<EventArgDef> definition;

	// setup documentation
	if (formatspec)
	{
		if (argument_names)
		{
			con::Container<xstr> argNames;

			const rawchar_t* specPtr = formatspec;
			//
			// store off all the names
			//
			if (argument_names)
			{
				// get the number of args
				const rawchar_t* argStart = argument_names;
				const rawchar_t* p = argStart;
				while (*p)
				{
					if (*p == ' ')
					{
						argNames.AddObject(xstr(argStart, p - argStart));
						while (*p == ' ') ++p;
						argStart = p;
					}
					else {
						p++;
					}
				}

				// add the last argument
				argNames.AddObject(xstr(argStart, p - argStart));
			}

			size_t index = 0;
			size_t specLength = xstr::len(formatspec);

			//
			// create the definition container
			//
			definition.Resize(argNames.NumObjects());

			// go throught he formatspec
			while (specLength)
			{
				// clear the rangeSpec
				xstr rangeSpec = "";
				// get the argSpec
				xstr argSpec = "";
				argSpec += *specPtr;
				specPtr++;
				specLength--;
				// see if there is a range specified
				while (*specPtr == '[')
				{
					// add in all the characters until NULL or ']'
					while (specLength && (*specPtr != ']'))
					{
						rangeSpec += *specPtr;
						specPtr++;
						specLength--;
					}
					if (specLength && (*specPtr == ']'))
					{
						rangeSpec += *specPtr;
						specPtr++;
						specLength--;
					}
				}
				if (index < argNames.NumObjects())
				{
					xstr argName = argNames.ObjectAt(index + 1);
					EventArgDef* argDef = new (definition) EventArgDef;
					argDef->Setup(name, argName, argSpec, rangeSpec);
				}
				else
				{
					assert(0);
					//Error("More format specifiers than argument names for event %s\n", name);
				}
				index++;
			}
			if (index < argNames.NumObjects())
			{
				assert(0);
				//Error("More argument names than format specifiers for event %s\n", name);
			}
		}
	}

	return definition;
}

EventArgDef::EventArgDef()
{
	type = vartype::IS_INTEGER;
	//name        = "undefined";
	optional = false;
};

void EventArgDef::Setup(const rawchar_t* eventName, const rawchar_t* argName, const rawchar_t* argType, const rawchar_t* argRange)
{
	rawchar_t        scratch[256];
	const rawchar_t* ptr;
	rawchar_t* tokptr;
	const rawchar_t* endptr;
	int         index;

	// set name
	name = argName;

	// set optionality
	if (isupper(argType[0]))
	{
		optional = true;
	}
	else
	{
		optional = false;
	}

	// grab the ranges
	index = 0;
	memset(minRangeDefault, true, sizeof(minRangeDefault));
	memset(minRange, 0, sizeof(minRange));
	memset(maxRangeDefault, true, sizeof(maxRangeDefault));
	memset(maxRange, 0, sizeof(maxRange));

	if (argRange && argRange[0])
	{
		ptr = argRange;
		while (1)
		{
			// find opening '['
			tokptr = (rawchar_t*)strchr(ptr, '[');
			if (!tokptr)
			{
				break;
			}
			// find closing ']'
			endptr = strchr(tokptr, ']');
			if (!endptr)
			{
				assert(0);
				printf("Argument defintion %s, no matching ']' found for range spec in event %s.\n", name.c_str(), eventName);
				break;
			}
			// point to the next range
			ptr = endptr;
			// skip the '['
			tokptr++;
			// copy off the range spec
			// skip the ']'
			strncpy(scratch, tokptr, endptr - tokptr);
			// terminate the range
			scratch[endptr - tokptr] = 0;
			// see if there is one or two parameters here
			tokptr = strchr(scratch, ',');
			if (!tokptr)
			{
				// just one parameter
				minRange[index >> 1] = (float)atof(scratch);
				minRangeDefault[index >> 1] = false;
				index++;
				// skip the second parameter
				index++;
			}
			else if (tokptr == scratch)
			{
				// just second parameter
				// skip the first paremeter
				index++;
				tokptr++;
				maxRange[index >> 1] = (float)atof(scratch);
				maxRangeDefault[index >> 1] = false;
				index++;
			}
			else
			{
				bool second;
				// one or two parameters
				// see if there is anything behind the ','
				if (strlen(scratch) > size_t(tokptr - scratch + 1))
					second = true;
				else
					second = false;
				// zero out the ','
				*tokptr = 0;
				minRange[index >> 1] = (float)atof(scratch);
				minRangeDefault[index >> 1] = false;
				index++;
				// skip over the nul character
				tokptr++;
				if (second)
				{
					maxRange[index >> 1] = (float)atof(tokptr);
					maxRangeDefault[index >> 1] = false;
				}
				index++;
			}
		}
	}

	// figure out the type of variable it is
	switch (tolower(argType[0]))
	{
	case 'e':
		type = IS_ENTITY;
		break;
	case 'v':
		type = IS_VECTOR;
		break;
	case 'i':
		type = IS_INTEGER;
		break;
	case 'f':
		type = IS_FLOAT;
		break;
	case 's':
		type = IS_STRING;
		break;
	case 'b':
		type = IS_BOOLEAN;
		break;
	case 'l':
		type = IS_LISTENER;
		break;
	}
}

void EventArgDef::GetRange(size_t& numRanges, bool& isInteger, bool& isSingle) const
{
	switch (type)
	{
	case IS_VECTOR:
		isInteger = false;
		isSingle = false;
		numRanges = 3;
		break;
	case IS_FLOAT:
		isInteger = false;
		isSingle = false;
		numRanges = 1;
		break;
	case IS_STRING:
		isInteger = true;
		isSingle = true;
		numRanges = 1;
		break;
	default:
		isSingle = false;
		isInteger = true;
		numRanges = 1;
		break;
	}
}

float EventArgDef::GetMinRange(uintptr_t index) const
{
	if (index < 3)
		return minRange[index];
	return 0.0;
}

bool EventArgDef::GetMinRangeDefault(uintptr_t index) const
{
	if (index < 3)
		return minRangeDefault[index];
	return false;
}

float EventArgDef::GetMaxRange(uintptr_t index) const
{
	if (index < 3)
		return maxRange[index];
	return 0.0;
}

bool EventArgDef::GetMaxRangeDefault(uintptr_t index) const
{
	if (index < 3)
		return maxRangeDefault[index];
	return false;
}

int EventArgDef::getType() const
{
	return type;
}

const rawchar_t* EventArgDef::GetTypeName() const
{
	switch (type)
	{
	case IS_ENTITY:
		return "Entity";
	case IS_VECTOR:
		return "Vector";
	case IS_INTEGER:
		return "Integer";
	case IS_FLOAT:
		return "Float";
	case IS_STRING:
		return "String";
	case IS_BOOLEAN:
		return "Boolean";
	case IS_LISTENER:
		return "Listener";
	default:
		return "???";
	}
}

const rawchar_t* EventArgDef::getName() const
{
	return name.c_str();
}

bool EventArgDef::isOptional() const
{
	return optional;
}

AbstractFormater::~AbstractFormater()
{

}

void HTMLFormater::PrintDocumentHeader(std::ostream& stream, const rawchar_t* title)
{
	stream << "<html><head><title>" << title << "</title></head><body><h1><center>" << title << "</center></h1>";
}

void HTMLFormater::PrintDocumentFooter(std::ostream& stream)
{
	stream << "</body></html>";
}

void HTMLFormater::BeginClassHeader(std::ostream& stream, const rawchar_t* className, const rawchar_t* classID)
{
	if(classID) {
		stream << "<h2> <a name=\"" << className << "\">" << className << " (<i>" << classID << "</i>)</a>";
	}
	else {
		stream << "<h2> <a name=\"" << className << "\">" << className << "</a>";
	}
}

void HTMLFormater::EndClassHeader(std::ostream& stream)
{
	stream << "</h2>";
}

void HTMLFormater::BeginClassBody(std::ostream& stream)
{
	stream << "<BLOCKQUOTE>";
}

void HTMLFormater::EndClassBody(std::ostream& stream)
{
	stream << "</BLOCKQUOTE>";
}


void HTMLFormater::BeginEventDoc(std::ostream& stream, const rawchar_t* eventName)
{
	stream << "<p><tt><b>" << eventName << "</b>";
}

void HTMLFormater::EndEventDoc(std::ostream& stream)
{
	stream << "</tt><br>";
}

void HTMLFormater::BeginEventDefinition(std::ostream& stream)
{
	stream << "( <i>";
}

void HTMLFormater::EndEventDefinition(std::ostream& stream)
{
	stream << "</i> )";
}

void HTMLFormater::BeginEventArgument(std::ostream& stream, bool isOptional)
{
	if (isOptional) stream << "[ ";
}

void HTMLFormater::EndEventArgument(std::ostream& stream, bool isOptional, bool isLast)
{
	if (isOptional) stream << " ]";
	if (!isLast) stream << ", ";
}

void HTMLFormater::PrintEventArgumentType(std::ostream& stream, const rawchar_t* argumentType)
{
	stream << argumentType << " ";
}

void HTMLFormater::PrintEventArgumentName(std::ostream& stream, const rawchar_t* argumentName)
{
	stream << argumentName;
}

void HTMLFormater::PrintEventArgumentRangeSingle(std::ostream& stream, float minRange, bool isInteger)
{
	if (isInteger) {
		stream << "<" << (uint32_t)minRange << ">";
	}
	else {
		stream << "<" << std::setprecision(2) << minRange << ">" << std::fixed;
	}
}

void HTMLFormater::PrintEventArgumentRange(std::ostream& stream, float minRange, float maxRange, bool isInteger)
{
	if (isInteger)
	{
		if (minRange != -1 && maxRange != -1) {
			stream << "<" << (uint32_t)minRange << "..." << (uint32_t)maxRange << ">";
		}
		else if (minRange != -1 && maxRange == -1) {
			stream << "<" << (uint32_t)minRange << "...max_integer>";
		}
		else if (minRange == -1 && maxRange != -1) {
			stream << "<min_integer..." << (uint32_t)maxRange << ">";
		}
	}
	else
	{
		if (minRange != -1 && maxRange != -1) {
			stream << std::setprecision(2) << "<" << minRange << "..." << maxRange << ">";
		}
		else if (minRange != -1 && maxRange == -1) {
			stream << std::setprecision(2) << "<" << minRange << "...max_integer>";
		}
		else if (minRange == -1 && maxRange != -1) {
			stream << std::setprecision(2) << "<min_integer..." << maxRange << ">";
		}
	}
}

void HTMLFormater::PrintEventDocumentation(std::ostream& stream, const rawchar_t* documentation)
{
	stream << "<ul>";

	const rawchar_t* docStart = documentation;
	const rawchar_t* p = docStart;

	for(const rawchar_t* p = docStart; *p; ++p)
	{
		if (*p != '\n')
		{
			stream << *p;
		} else {
			stream << "<br>";
		}
	}

	stream << "</ul>";
}

void HTMLFormater::PrintClassLineage(std::ostream& stream, const rawchar_t* className)
{
	stream << " -> <a href=\"#" << className << "\">" << className << "</a>";
}
