#pragma once

#include "../Container/Container.h"
#include "../Container/Container_archive.h"

namespace mfuse
{
    class Archiver;

    namespace con
    {
    template<class Type>
    class ContainerClass : public Container<Type>, public Class {
    public:
        using Container<Type>::Container;

        virtual void Archive(Archiver& arc);
        void Archive(Archiver& arc, void(*ArchiveFunc)(Archiver& arc, Type& obj));
    };

    template<class Type>
    void ContainerClass<Type>::Archive(Archiver& arc, ContainerArchiveFunc<Type> ArchiveFunc)
    {
        con::Archive(arc, *this, ArchiveFunc);
    }
    }
};
