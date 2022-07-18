/*
 * Copyright 2018, 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_casper.h"

#include "test_ecmul256.h"
#include "test_ecdoublemul256.h"

#include "test_ecmul384.h"
#include "test_ecdoublemul384.h"

#include "test_ecmul521.h"
#include "test_ecdoublemul521.h"

#include <string.h>

#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define TEST_ASSERT(a)       \
    if (!(a))                \
    {                        \
        PRINTF("error\r\n"); \
        do                   \
        {                    \
        } while (1);         \
    }

/*******************************************************************************
 * Variables
 ******************************************************************************/
static const unsigned pubkey0[2048 / 32] = {
    0x232577d1, 0xa1c2d50d, 0x99706557, 0x46c818ab, 0xaa53916b, 0x63f5f64b, 0x93cd68f4, 0x1ed00fb7,
    0xea292749, 0x39de13c1, 0x065b5911, 0x884b13c9, 0xe6f6c061, 0xef47223e, 0x92e9e488, 0x3931a79d,
    0x14861755, 0xde93cc46, 0x1e74bf9a, 0xb4a3d058, 0x2b63c4b1, 0xd37d1bf7, 0x3fd70745, 0x5095a782,
    0xec4bcd7b, 0xbef831fb, 0x7f682470, 0x56a9a012, 0x6136775a, 0x2777c47d, 0x89b3f94a, 0x62fa6f9f,
    0xc97a18ab, 0x55d68409, 0x39007ccb, 0xe3514d48, 0xe817cc0e, 0xafd713ba, 0x14a82e21, 0xe5ff1433,
    0x385a8131, 0x31f2ece8, 0x8d395a2a, 0x85622d91, 0x67634847, 0xb219d21d, 0x1ef8efaa, 0xfaa05682,
    0x109b9a8a, 0x41042b7e, 0x0ebe7f64, 0xdae23bff, 0x5cfd544b, 0x74b9cbf2, 0x9563cafb, 0x462b3911,
    0x16e9cdf4, 0x68ed6d7f, 0xc6e45a20, 0x65838412, 0xa261fc8b, 0xbdd913f2, 0xc1782e4d, 0xbad47a84,
};
static const unsigned plaintext0[2048 / 32] = {
    0x4b4fb4bc, 0x9da3c722, 0x1fa87ba2, 0xf312d3f1, 0xb3823c63, 0x0917140a, 0xa07c69d7, 0xc2c92b88,
    0x7e732102, 0xd3420e56, 0x7c089aa2, 0x518dc5f5, 0xdeb09cc4, 0x9d429cf9, 0x2deca5b4, 0x430bb1a7,
    0xe8fd387c, 0x083ec701, 0x518db8a2, 0x407db831, 0x5b2bf516, 0x570eb451, 0xc4f202a9, 0x77f504da,
    0x5b73edac, 0x61e5667e, 0xf131bd94, 0xf2d3ce56, 0x09c828d6, 0x57ce7f8f, 0xcfba290b, 0xf53c3d7f,
    0x16bd7ae8, 0x6e8ad8fd, 0x7995a8ba, 0x5d2102ef, 0x982a4658, 0x2d362945, 0x2428b8d3, 0xb6c2f765,
    0x608adb30, 0xfe6be10e, 0xdfcd8056, 0x37bbc360, 0x5d00f1a4, 0xbde4493f, 0x9fb4eab5, 0x80a14649,
    0x7a56082c, 0x1caf81b2, 0x21bb7186, 0x53576457, 0xf58300d2, 0xfbfb82b0, 0xf303a568, 0xeb7f0d4d,
    0x2c4b0b4e, 0xaf50cac4, 0x6f9d7808, 0x3f120e32, 0x9fa1cd64, 0x2a94b3ab, 0x95a4908d, 0x70992c9d,
};
static const unsigned signature0[2048 / 32] = {
    0x892b6f74, 0x85c033f9, 0x33ab8b20, 0x74007e6a, 0xf1687e00, 0x96960052, 0x875dbc47, 0xe2d51612,
    0x804bf80e, 0x0bcc5205, 0x5b630d07, 0x741553a4, 0xe77737da, 0xffdd47b3, 0xcad941db, 0xda40f72e,
    0x2a42eb3b, 0xdfd88bb2, 0xadd387fe, 0xaf641538, 0x72ce8a31, 0x965b713d, 0x35e78b46, 0xfee41c44,
    0x7ef74f17, 0x496ddbc7, 0xd9f09955, 0x26eda243, 0x210b25e9, 0x6e032a66, 0x430800e1, 0xabbe7f89,
    0xd339cc87, 0x81cac45c, 0x06e9e6ec, 0x32d5be61, 0x0632d363, 0x5404adb0, 0xa2dad9fa, 0x62dbc7a9,
    0xc299bd0e, 0xcc9ff240, 0x71d5c214, 0x3131e9b3, 0x6a8a974a, 0xc49551d8, 0xe457c1da, 0x45d99126,
    0x451c6b46, 0x51b753f7, 0x1ec0663e, 0x69d2429f, 0x404788b8, 0x8a28dd70, 0xc86f9d1b, 0xcb6bad9a,
    0xd84d8836, 0x37cff304, 0xf4c3ae42, 0x3dce66d0, 0x3fbf9896, 0x028c75dc, 0xa05f626b, 0x7e65b998,
};
static const unsigned pub_e = 0x10001;

static uint8_t plaintext[2048 / 8];

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/* Example curve selector  */
#define CASPER_ECC_P256 1
#define CASPER_ECC_P384 1
#define CASPER_ECC_P521 1

/*!
 * @brief Main function.
 */
int main(void)
{
    /* Init hardware */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    /*Make sure casper ram buffer has power up*/
    POWER_DisablePD(kPDRUNCFG_PPD_CASPER_SRAM);
    POWER_ApplyPD();

    /* Initialize CASPER */
    CASPER_Init(CASPER);

    /* ModExp test */
    CASPER_ModExp(CASPER, (void *)signature0, (void *)pubkey0, sizeof(plaintext0) / sizeof(uint32_t), pub_e, plaintext);
    TEST_ASSERT(memcmp(plaintext0, plaintext, sizeof(plaintext)) == 0);
    PRINTF("ModExp Test pass.\r\n");

    /* ECC tests */

#if CASPER_ECC_P256

#define ECC_P256_WORDS 256U / 32U /* 256U/32U = 8 */

    CASPER_ecc_init(kCASPER_ECC_P256);

    PRINTF("Casper ECC Demo P256\r\n\r\n");

    /* Begin code to test elliptic curve scalar multiplication. */
    {
        int i;
        int m1, m2;

        int errors = 0;

        for (i = 0; i < 8; i++)
        {
            PRINTF("Round: %d\r\n", i);

            uint32_t X1[ECC_P256_WORDS], Y1[ECC_P256_WORDS];
            uint32_t *X3 = &test_ecmulans256[i][0];
            uint32_t *Y3 = &test_ecmulans256[i][ECC_P256_WORDS];

            CASPER_ECC_SECP256R1_Mul(CASPER, X1, Y1, &test_ecmulans256[0][0], &test_ecmulans256[0][ECC_P256_WORDS],
                                     test_ecmulscalar256[i]);
            CASPER_ECC_equal(&m1, X1, X3);
            CASPER_ECC_equal(&m2, Y1, Y3);
            if (m1 != 0 || m2 != 0)
            {
                errors++;
            }
        }
        if (errors != 0)
        {
            PRINTF("Not all EC scalar multiplication tests were successful.\r\n\r\n");
            PRINTF("%d / 8 tests failed.\n", errors);
        }
        else
        {
            PRINTF("All EC scalar multiplication tests were successful.\r\n\r\n");
        }
    }
    /* End code to test elliptic curve scalar multiplication. */

    /* Begin code to test elliptic curve double scalar multiplication. */
    {
        int i;
        int m1, m2;

        int errors = 0;
        uint32_t c3[ECC_P256_WORDS], c4[ECC_P256_WORDS];
        for (i = 0; i < 8; i++)
        {
            PRINTF("Round: %d\r\n", i);
            uint32_t *c1 = &test_ecddoublemul_result256[i][0];
            uint32_t *c2 = &test_ecddoublemul_result256[i][ECC_P256_WORDS];
            CASPER_ECC_SECP256R1_MulAdd(
                CASPER, c3, c4, &test_ecddoublemul_base256[0][0], &test_ecddoublemul_base256[0][ECC_P256_WORDS],
                &test_ecddoublemul_scalars256[i][0], &test_ecddoublemul_base256[1][0],
                &test_ecddoublemul_base256[1][ECC_P256_WORDS], &test_ecddoublemul_scalars256[i][ECC_P256_WORDS]);

            CASPER_ECC_equal(&m1, c1, c3);
            CASPER_ECC_equal(&m2, c2, c4);
            if (m1 != 0 || m2 != 0)
            {
                errors++;
            }
        }
        if (errors != 0)
        {
            PRINTF("Not all EC double scalar multiplication tests were successful.\r\n\r\n");
            PRINTF("%d / 8 tests failed.\n", errors);
        }
        else
        {
            PRINTF("All EC double scalar multiplication tests were successful.\r\n\r\n");
        }
    }
    /* End code to test elliptic curve double scalar multiplication. */

#endif /* CASPER_ECC_P256 */

#if CASPER_ECC_P384

#define ECC_P384_WORDS 384 / 32U /* 384U/32U = 12 */

    PRINTF("Casper ECC Demo P384\r\n\r\n");

    CASPER_ecc_init(kCASPER_ECC_P384);

    /* Begin code to test elliptic curve scalar multiplication. */
    {
        int i;
        int m1, m2;

        int errors = 0;

        for (i = 0; i < 8; i++)
        {
            PRINTF("Round: %d\r\n", i);

            uint32_t X1[ECC_P384_WORDS], Y1[ECC_P384_WORDS];
            uint32_t *X3 = &test_ecmulans384[i][0];
            uint32_t *Y3 = &test_ecmulans384[i][ECC_P384_WORDS];

            CASPER_ECC_SECP384R1_Mul(CASPER, X1, Y1, &test_ecmulans384[0][0], &test_ecmulans384[0][ECC_P384_WORDS],
                                     test_ecmulscalar384[i]);
            CASPER_ECC_equal(&m1, X1, X3);
            CASPER_ECC_equal(&m2, Y1, Y3);
            if (m1 != 0 || m2 != 0)
            {
                errors++;
            }
        }
        if (errors != 0)
        {
            PRINTF("Not all EC scalar multiplication tests were successful.\r\n");
            PRINTF("%d / 8 tests failed.\r\n", errors);
        }
        else
        {
            PRINTF("All EC scalar multiplication tests were successful.\r\n");
        }
    }
    /* End code to test elliptic curve scalar multiplication. */

    /* Begin code to test elliptic curve double scalar multiplication. */
    {
        int i;
        int m1, m2;

        int errors = 0;
        uint32_t c3[ECC_P384_WORDS], c4[ECC_P384_WORDS];
        for (i = 0; i < 8; i++)
        {
            PRINTF("Round: %d\r\n", i);
            uint32_t *c1 = &test_ecddoublemul_result384[i][0];
            uint32_t *c2 = &test_ecddoublemul_result384[i][ECC_P384_WORDS];
            CASPER_ECC_SECP384R1_MulAdd(
                CASPER, c3, c4, &test_ecddoublemul_base384[0][0], &test_ecddoublemul_base384[0][ECC_P384_WORDS],
                &test_ecddoublemul_scalars384[i][0], &test_ecddoublemul_base384[1][0],
                &test_ecddoublemul_base384[1][ECC_P384_WORDS], &test_ecddoublemul_scalars384[i][ECC_P384_WORDS]);
            CASPER_ECC_equal(&m1, c3, c1);
            CASPER_ECC_equal(&m2, c4, c2);
            if (m1 != 0 || m2 != 0)
            {
                errors++;
            }
        }
        if (errors != 0)
        {
            PRINTF("Not all EC double scalar multiplication tests were successful.\r\n\r\n");
            PRINTF("%d / 8 tests failed.\r\n", errors);
        }
        else
        {
            PRINTF("All EC double scalar multiplication tests were successful.\r\n\r\n");
        }
    }
    /* End code to test elliptic curve scalar multiplication. */

#endif /* CASPER_ECC_P384 */

#if CASPER_ECC_P521

#define ECC_P521_WORDS 576 / 32U /* 576U/32U = 18 */

    PRINTF("Casper ECC Demo P521\r\n\r\n");

    CASPER_ecc_init(kCASPER_ECC_P521);

    /* Begin code to test elliptic curve scalar multiplication. */

    {
        int i;
        int m1, m2;

        int errors = 0;

        for (i = 0; i < 8; i++)
        {
            PRINTF("Round: %d\r\n", i);

            uint32_t X1[ECC_P521_WORDS], Y1[ECC_P521_WORDS];
            uint32_t *X3 = &test_ecmulans521[i][0];
            uint32_t *Y3 = &test_ecmulans521[i][ECC_P521_WORDS];

            CASPER_ECC_SECP521R1_Mul(CASPER, X1, Y1, &test_ecmulans521[0][0], &test_ecmulans521[0][ECC_P521_WORDS],
                                     test_ecmulscalar521[i]);
            CASPER_ECC_equal(&m1, X1, X3);
            CASPER_ECC_equal(&m2, Y1, Y3);
            if (m1 != 0 || m2 != 0)
            {
                errors++;
            }
        }
        if (errors != 0)
        {
            PRINTF("Not all EC scalar multiplication tests were successful.\r\n");
            PRINTF("%d / 8 tests failed.\n", errors);
        }
        else
        {
            PRINTF("All EC scalar multiplication tests were successful.\r\n");
        }
    }

    /* End code to test elliptic curve scalar multiplication. */

    /* Begin code to test elliptic curve double scalar multiplication. */
    {
        int i;
        int m1, m2;

        int errors = 0;
        uint32_t c3[ECC_P521_WORDS], c4[ECC_P521_WORDS];
        for (i = 0; i < 8; i++)
        {
            PRINTF("Round: %d\r\n", i);
            uint32_t *c1 = &test_ecddoublemul_result521[i][0];
            uint32_t *c2 = &test_ecddoublemul_result521[i][ECC_P521_WORDS];
            CASPER_ECC_SECP521R1_MulAdd(
                CASPER, c3, c4, &test_ecddoublemul_base521[0][0], &test_ecddoublemul_base521[0][ECC_P521_WORDS],
                &test_ecddoublemul_scalars521[i][0], &test_ecddoublemul_base521[1][0],
                &test_ecddoublemul_base521[1][ECC_P521_WORDS], &test_ecddoublemul_scalars521[i][ECC_P521_WORDS]);

            CASPER_ECC_equal(&m1, c1, c3);
            CASPER_ECC_equal(&m2, c2, c4);
            if (m1 != 0 || m2 != 0)
            {
                errors++;
            }
        }
        if (errors != 0)
        {
            PRINTF("Not all EC double scalar multiplication tests were successful.\r\n");
            PRINTF("%d / 8 tests failed.\n", errors);
        }
        else
        {
            PRINTF("All EC double scalar multiplication tests were successful.\r\n");
        }
    }
    /* End code to test elliptic curve double scalar multiplication. */
#endif /* CASPER_ECC_P521 */

    /* Deinitialize CASPER */
    CASPER_Deinit(CASPER);

    while (1)
    {
    }
}
