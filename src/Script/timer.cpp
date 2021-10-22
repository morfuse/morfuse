#include <morfuse/Script/timer.h>
#include <morfuse/Script/Archiver.h>
#include <morfuse/Container/Container_archive.h>

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
	uintptr_t foundIndex = 0;
	const Element* foundElement = nullptr;
	uinttime_t best_time = m_time;

	for (intptr_t i = m_Elements.NumObjects(); i > 0; i--)
	{
		const Element& e = m_Elements.ObjectAt(i);
		if (e.time <= best_time)
		{
			best_time = e.time;
			foundIndex = i;
			foundElement = &e;
		}
	}

	if (foundIndex)
	{
		m_Elements.RemoveObjectAt(foundIndex);
		foundtime = best_time;
		return foundElement->obj;
	}
	else
	{
		m_bDirty = false;
		return nullptr;
	}
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

void con::timer::SetTime(uinttime_t time)
{
	m_time = time;
	m_bDirty = true;
}

void con::timer::ArchiveElement(Archiver& arc, Element& e)
{
	arc.ArchiveObjectPointer(e.obj);
	arc.ArchiveUInt64(e.time);
}

void con::timer::Archive(Archiver& arc)
{
	arc.ArchiveBoolean(m_bDirty);
	arc.ArchiveUInt64(m_time);

	con::Archive(arc, m_Elements, con::timer::ArchiveElement);
}
