// Basic Hash functions

#include <morfuse/Container/set.h>
#include <morfuse/Container/set_generic_hash.h>
#include <morfuse/Common/str.h>
#include <morfuse/Common/Vector.h>
#include <stddef.h>

using namespace mfuse;

size_t con::set_primes[24] =
{
    7,
    17,
    37,
    79,
    163,
    331,
    673,
    1361,
    2729,
    5471,
    10949,
    21911,
    43853,
    87719,
    175447,
    701819,
    1403641,
    2807303,
    5614657,
    11229331,
    22458671,
    44917381,
    89834777
};
