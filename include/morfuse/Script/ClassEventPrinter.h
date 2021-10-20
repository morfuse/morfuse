#pragma once

#include "../Global.h"
#include "../Common/rawchar.h"
#include "../Common/str.h"
#include "../Container/Container.h"

#include <iostream>
#include <functional>

namespace mfuse
{
	class EventDef;
	class EventArgDef;
	class ClassDef;
	class AbstractFormater;

	class ClassEventPrinter
	{
	public:
		mfuse_EXPORTS void DumpAllClasses(std::ostream& class_stream, AbstractFormater& formater, const rawchar_t* title);
		void DumpClass(ClassDef* classDef, std::ostream& class_stream, AbstractFormater& formater, const con::Container<EventDef*>& sortedEvents);
		void ClassEvents(const ClassDef* classDef, std::ostream& class_stream, AbstractFormater& formater, const con::Container<EventDef*>& sortedEvents);
		void PrintDocumentation(EventDef& def, std::ostream& event_stream, AbstractFormater& formater);
		void PrintEventDocumentation(EventDef& def, std::ostream& event_stream, AbstractFormater& formater);
		void PrintEventArgumentRange(std::ostream& event_file, const EventArgDef& argDef, AbstractFormater& formater);

		con::Container<EventArgDef> SetupDocumentation(EventDef& def);

	private:
		void SortEventList(con::Container<EventDef*>& eventList);
		void SortClassList(con::Container<ClassDef*>& classList);
	};

	enum vartype
	{
		IS_STRING,
		IS_VECTOR,
		IS_BOOLEAN,
		IS_INTEGER,
		IS_FLOAT,
		IS_ENTITY,
		IS_LISTENER
	};

	class EventArgDef
	{
	private:
		vartype type;
		xstr name;
		float minRange[3];
		bool minRangeDefault[3];
		float maxRange[3];
		bool maxRangeDefault[3];
		bool optional;
	public:

		EventArgDef();

		void Setup(const rawchar_t* eventName, const rawchar_t* argName, const rawchar_t* argType, const rawchar_t* argRange);
		int getType() const;
		const rawchar_t* GetTypeName() const;
		const rawchar_t* getName() const;
		bool isOptional() const;
		void GetRange(size_t& numRanges, bool& isInteger, bool& isSingle) const;
		float GetMinRange(uintptr_t index) const;
		bool GetMinRangeDefault(uintptr_t index) const;
		float GetMaxRange(uintptr_t index) const;
		bool GetMaxRangeDefault(uintptr_t index) const;
	};

	using formatPrintFunc = std::function<void(std::ostream&)>;

	class mfuse_EXPORTS AbstractFormater
	{
	public:
		virtual ~AbstractFormater();

		virtual void PrintDocumentHeader(std::ostream& stream, const rawchar_t* title) = 0;
		virtual void BeginClassHeader(std::ostream& stream, const rawchar_t* className, const rawchar_t* classID) = 0;
		virtual void EndClassHeader(std::ostream& stream) = 0;
		virtual void PrintClassLineage(std::ostream& stream, const rawchar_t* className) = 0;
		virtual void BeginClassBody(std::ostream& stream) = 0;
		virtual void EndClassBody(std::ostream& stream) = 0;
		virtual void BeginEventDoc(std::ostream& stream, const rawchar_t* eventName) = 0;
		virtual void EndEventDoc(std::ostream& stream) = 0;
		virtual void BeginEventDefinition(std::ostream& stream) = 0;
		virtual void EndEventDefinition(std::ostream& stream) = 0;
		virtual void BeginEventArgument(std::ostream& stream, bool isOptional) = 0;
		virtual void EndEventArgument(std::ostream& stream, bool isOptional, bool isLast) = 0;
		virtual void PrintEventArgumentType(std::ostream& stream, const rawchar_t* argumentType) = 0;
		virtual void PrintEventArgumentName(std::ostream& stream, const rawchar_t* argumentName) = 0;
		virtual void PrintEventArgumentRangeSingle(std::ostream& stream, float minRange, bool isInteger) = 0;
		virtual void PrintEventArgumentRange(std::ostream& stream, float minRange, float maxRange, bool isInteger) = 0;
		virtual void PrintEventDocumentation(std::ostream& stream, const rawchar_t* documentation) = 0;
		virtual void PrintDocumentFooter(std::ostream& stream) = 0;
	};

	class mfuse_EXPORTS HTMLFormater : public AbstractFormater
	{
	public:
		void PrintDocumentHeader(std::ostream& stream, const rawchar_t* title) override;
		void BeginClassHeader(std::ostream& stream, const rawchar_t* className, const rawchar_t* classID) override;
		void EndClassHeader(std::ostream& stream) override;
		void BeginClassBody(std::ostream& stream) override;
		void EndClassBody(std::ostream& stream) override;
		void BeginEventDoc(std::ostream& stream, const rawchar_t* eventName) override;
		void EndEventDoc(std::ostream& stream) override;
		void BeginEventDefinition(std::ostream& stream) override;
		void EndEventDefinition(std::ostream& stream) override;
		void BeginEventArgument(std::ostream& stream, bool isOptional) override;
		void EndEventArgument(std::ostream& stream, bool isOptional, bool isLast) override;
		void PrintEventArgumentType(std::ostream& stream, const rawchar_t* argumentType) override;
		void PrintEventArgumentName(std::ostream& stream, const rawchar_t* argumentName) override;
		void PrintEventArgumentRangeSingle(std::ostream& stream, float minRange, bool isInteger) override;
		void PrintEventArgumentRange(std::ostream& stream, float minRange, float maxRange, bool isInteger) override;
		void PrintEventDocumentation(std::ostream& stream, const rawchar_t* documentation) override;
		void PrintClassLineage(std::ostream& stream, const rawchar_t* className) override;
		void PrintDocumentFooter(std::ostream& stream) override;

	};

	class RawTextFormater : public AbstractFormater
	{};

	class JsonFormater : public AbstractFormater
	{};
}
