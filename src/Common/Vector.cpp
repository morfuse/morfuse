#include <morfuse/Container/set_generic_hash.h>
#include <morfuse/Common/Vector.h>

using namespace mfuse;

template<>
intptr_t Hash<Vector>::operator()(const Vector& key) const
{
    return (int)((key[0] + key[1] + key[2]) / 3);
}
