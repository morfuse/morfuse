#include <morfuse/Common/ConstStr.h>
#include <morfuse/Container/set_generic_hash.h>

using namespace mfuse;

template<>
intptr_t Hash<const_str>::operator()(const const_str& key) const
{
	return (intptr_t)key;
}
