#pragma once

#include "Listener.h"
#include "../Common/Vector.h"
#include "../Common/str.h"
#include "Components/TargetComponent.h"

namespace mfuse
{
    extern EventDef EV_SetAngles;
    extern EventDef EV_SetAngle;
    extern EventDef EV_SetOrigin;

    typedef unsigned int entflags_t;

    class Game;
    class Level;
    class World;

    class SimpleEntity;
    typedef SafePtr< SimpleEntity > SimpleEntityPtr;

    class mfuse_PUBLIC SimpleEntity : public Listener
    {
        MFUS_CLASS_PROTOTYPE_EXPORTS(SimpleEntity);

    public:
        mfuse_EXPORTS SimpleEntity();
        mfuse_EXPORTS ~SimpleEntity();

        mfuse_EXPORTS void Archive(Archiver& arc) override;

        mfuse_EXPORTS void setOrigin(Vector origin);
        mfuse_EXPORTS void setOriginEvent(Vector origin);
        mfuse_EXPORTS void setAngles(Vector angles);

        mfuse_EXPORTS const Vector& getAngles() const;
        mfuse_EXPORTS const Vector& getOrigin() const;
        mfuse_EXPORTS TargetComponent& GetTargetComponent();

    private:
        mfuse_LOCAL void EventGetAngle(Event& ev);
        mfuse_LOCAL void EventGetAngles(Event& ev);
        mfuse_LOCAL void EventGetOrigin(Event& ev);
        mfuse_LOCAL void EventGetTargetname(Event& ev);
        mfuse_LOCAL void EventGetTarget(Event& ev);

        mfuse_LOCAL void EventSetAngle(Event& ev);
        mfuse_LOCAL void EventSetAngles(Event& ev);
        mfuse_LOCAL void EventSetOrigin(Event& ev);
        mfuse_LOCAL void EventSetTargetname(Event& ev);
        mfuse_LOCAL void EventSetTarget(Event& ev);

        mfuse_LOCAL void GetCentroid(Event& ev);

        mfuse_LOCAL void GetForwardVector(Event& ev);
        mfuse_LOCAL void GetLeftVector(Event& ev);
        mfuse_LOCAL void GetRightVector(Event& ev);
        mfuse_LOCAL void GetUpVector(Event& ev);

    private:
        void SimpleArchive(Archiver& arc);

    private:
        // Base coord variable
        Vector origin;
        Vector angles;

        // Used by scripts
        TargetComponent targetComp;
        str target;

        // Centered origin based on mins/maxs
        Vector centroid;
    };
};
