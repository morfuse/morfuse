#include <morfuse/Script/timer.h>
#include <morfuse/Script/Archiver.h>
#include <morfuse/Script/Archiver.h>

using namespace mfuse;

con::timer::timer()
{
	m_time = 0;
	m_bDirty = false;
}

void con::timer::AddElement(Class*e, uinttime_t time)
{
	Element element;

	element.obj = e;
	element.time = time;

	m_Elements.AddObject(element);

	if (time <= m_time)
	{
		SetDirty();
	}
}

void con::timer::RemoveElement(Class*e)
{
	for (intptr_t i = m_Elements.NumObjects(); i > 0; i--)
	{
		Element *index = &m_Elements.ObjectAt(i);

		if (index->obj == e)
		{
			m_Elements.RemoveObjectAt(i);
			return;
		}
	}
}

Class* con::timer::GetNextElement(uint64_t& foundtime)
{
	uint64_t best_time;
	intptr_t foundIndex;
	Class* result;

	foundIndex = 0;
	best_time = m_time;

	for (intptr_t i = m_Elements.NumObjects(); i > 0; i--)
	{
		if (m_Elements.ObjectAt(i).time <= best_time)
		{
			best_time = m_Elements.ObjectAt(i).time;
			foundIndex = i;
		}
	}

	if (foundIndex)
	{
		result = m_Elements.ObjectAt(foundIndex).obj;
		m_Elements.RemoveObjectAt(foundIndex);
		foundtime = best_time;
	}
	else
	{
		result = nullptr;
		m_bDirty = false;
	}

	return result;
}

void con::timer::SetDirty()
{
	m_bDirty = true;
}

bool con::timer::IsDirty() const
{
	return m_bDirty;
}

bool con::timer::HasAnyElement() const
{
	return m_Elements.NumObjects() > 0;
}

void con::timer::SetTime(uint64_t time)
{
	m_time = time;
}

void con::timer::ArchiveElement(Archiver& arc, Element *e)
{
	arc.ArchiveObjectPointer((const void*&)e->obj);
	arc.ArchiveUInt64(e->time);
}

void con::timer::Archive(Archiver&)
{
	/*
	arc.ArchiveBool(m_bDirty);
	arc.ArchiveUInt64(m_time);

	m_Elements.Archive(arc, con::timer::ArchiveElement);
	*/
}
