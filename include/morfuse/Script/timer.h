#pragma once

#include "../Script/Class.h"
#include "../Common/Time.h"
#include "../Container/Container.h"

namespace mfuse
{
    class Class;

    namespace con
    {
    class timer : public Class
    {
    public:
        class Element
        {
        public:
            uinttime_t time;
            Class* obj;
        };

    private:
        uinttime_t m_time;
        Container<timer::Element> m_Elements;
        bool m_bDirty;

    public:
        timer();

        void AddElement(Class *e, uinttime_t time);
        void RemoveElement(Class* e);

        Class* GetNextElement(uint64_t& foundTime);

        void SetDirty();
        bool IsDirty() const;
        bool HasAnyElement() const;
        void SetTime(uinttime_t time);

        static void ArchiveElement(Archiver& arc, Element& e);
        virtual void Archive(Archiver& arc);
    };
    }
}
