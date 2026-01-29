#include "../../Global.h"
#include "../../Common/str.h"
#include "../../Common/SafePtr.h"
#include "../Component.h"
#include "../StringResolvable.h"

namespace mfuse
{
    class StringResolvable;

    /**
     * Component for assigning targetname and pointing to target.
     */
    class TargetComponent : public Component
    {
    public:
        mfuse_EXPORTS TargetComponent(Listener& parentRef);
        mfuse_EXPORTS ~TargetComponent();

        mfuse_EXPORTS void SetTargetName(const StringResolvable& targetname);
        mfuse_EXPORTS const StringResolvable& GetTargetName();

        mfuse_EXPORTS void SetTarget(const StringResolvable& newTarget);
        mfuse_EXPORTS const StringResolvable& GetTarget();

        mfuse_EXPORTS Listener* Next() const;

        void Archive(Archiver& arc) override;

    private:
        StringResolvable target;
        StringResolvable targetName;
    };
}