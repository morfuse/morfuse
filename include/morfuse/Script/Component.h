#pragma once

#include "../Global.h"
#include "Class.h"

namespace mfuse
{
    class Listener;
    class Archiver;

    /** Base component. */
    class Component : public Class
    {
    public:
        mfuse_EXPORTS Component(Listener& parentRef);
        virtual ~Component();

        virtual void Archive(Archiver& arc);

        mfuse_EXPORTS Listener& Parent() const;

    private:
        Listener& parent;
    };
}