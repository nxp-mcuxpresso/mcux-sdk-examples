
#include "fsl_common.h"
#include "fsl_rng.h"

#define RNG_EXAMPLE_RANDOM_BYTES       (4)

uint32_t get_seed(void)
{
    uint32_t data;

    RNG_Init(RNG);

    RNG_GetRandomData(RNG, &data, RNG_EXAMPLE_RANDOM_BYTES);

    return data;
}
