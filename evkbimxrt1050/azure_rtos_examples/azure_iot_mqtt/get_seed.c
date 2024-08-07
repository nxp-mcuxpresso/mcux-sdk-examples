#include "fsl_common.h"
#include "fsl_debug_console.h"

#if defined(FSL_FEATURE_SOC_TRNG_COUNT) && (FSL_FEATURE_SOC_TRNG_COUNT > 0)
#include "fsl_trng.h"

static void delay(void)
{
    volatile uint32_t i = 0;
    for (i = 0; i < 1000000; ++i)
    {
        __asm("NOP"); /* delay */
    }
}

uint32_t get_seed(void)
{
    TRNG_Type *base_ptr = (TRNG_Type *[])TRNG_BASE_PTRS[0];
    uint32_t random_val = 0;
    trng_config_t trngConfig;

    TRNG_GetDefaultConfig(&trngConfig);
    trngConfig.sampleMode = kTRNG_SampleModeVonNeumann;
    TRNG_Init(base_ptr, &trngConfig);

    /* delay */
    delay();

    TRNG_GetRandomData(base_ptr, (void *)&random_val, sizeof(random_val));

    return random_val;
}

#elif defined(FSL_FEATURE_SOC_CAAM_COUNT) && (FSL_FEATURE_SOC_CAAM_COUNT > 0)
#include "fsl_caam.h"

AT_NONCACHEABLE_SECTION(static caam_job_ring_interface_t s_jrif0);

__WEAK uint32_t get_seed(void)
{
    uint32_t random_val = 0;
    caam_config_t caam_config;
    caam_handle_t caam_handle = {.jobRing = kCAAM_JobRing0};

    CAAM_GetDefaultConfig(&caam_config);

    caam_config.jobRingInterface[0] = &s_jrif0;

    /* Init CAAM driver, including CAAM's internal RNG */
    if (CAAM_Init(CAAM, &caam_config) != kStatus_Success)
    {
        PRINTF("Failed to init CAAM&RNG!\r\n\r\n");
        return 0;
    }

    CAAM_RNG_GetRandomData(CAAM, &caam_handle, kCAAM_RngStateHandle0,
                (uint8_t *)&random_val, 4, kCAAM_RngDataAny, NULL);

    return random_val;
}

#else

#include "tx_api.h"

__WEAK uint32_t get_seed(void)
{
    return (uint32_t)tx_time_get();
}

#endif
