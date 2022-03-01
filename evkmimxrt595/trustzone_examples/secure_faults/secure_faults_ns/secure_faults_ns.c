/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "veneer_table.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_SEC_ADDRESS 0x30000000

/* 0x20400000 - 0x20401FFF is configured Non-Secure SRAM in SAU.
The SRAM alias 0x20400000 - 0x20401FFF is configured secure in AHB_SECURE_CTRL. */
#define DEMO_NONSEC_ADDRESS 0x20400000
typedef void (*funcptr_t)(char const *s);
#define PRINTF_NSE DbgConsole_Printf_NSE

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Global variables
 ******************************************************************************/
uint32_t testCaseNumber;

/*******************************************************************************
 * Code
 ******************************************************************************/

void SystemInit(void)
{
}
/*!
 * @brief Main function
 */
int main(void)
{
    int result;
    uint32_t *test_ptr;
    volatile uint32_t test_value = 0;
    funcptr_t func_ptr;


    /* used only to suppress GCC warning unused variable */
    (void)test_value;

    testCaseNumber = GetTestCaseNumber_NSE();

    PRINTF_NSE("Welcome in normal world!\r\n");

    /**************************************************************************
     * TEST EXAMPLE 3 - Invalid data access from normal world
     * In this example the pointer is set to address 0x30000000.
     * This address has secure attribute (see SAU settings)
     * If data is read from this address, the secure fault is generated
     * In normal world, the application doesn't have access to secure memory
     **************************************************************************/
    if (testCaseNumber == FAULT_INV_NS_DATA_ACCESS)
    {
        test_ptr   = (uint32_t *)(DEMO_SEC_ADDRESS);
        test_value = *test_ptr;
    }
    /* END OF TEST EXAMPLE 3 */

    /**************************************************************************
     * TEST EXAMPLE 4 - Invalid input parameters in entry function
     * In this example the input parameter is set to address 0x30000000.
     * This address has secure attribute (see SAU settings)
     * This secure violation is not detected by secure fault, since the input parameter
     * is used by secure function in secure mode. So this function has access to whole memory
     * However every entry function should check source of all input data in order
     * to avoid potention data leak from secure memory.
     * The correctness of input data cannot be checked automatically.
     * This has to be check by software using TT instruction
     **************************************************************************/
    if (testCaseNumber == FAULT_INV_INPUT_PARAMS)
    {
        PRINTF_NSE((char *)(DEMO_SEC_ADDRESS));
    }
    /* END OF TEST EXAMPLE 4 */

    /**************************************************************************
     * TEST EXAMPLE 2 - Invalid entry from normal to secure world
     * In this example function pointer is intentionally increased by 4.
     * By this the SG instruction is skipped, when function is called.
     * This causes ilegal entry point into secure world and secure fault is generated
     * The correct entry point into secure world is ensured by using
     * __cmse_nonsecure_entry keyword attribute for every entry function
     * Then the linker creates veneer table for all entry functions with SG instruction
     **************************************************************************/
    if (testCaseNumber == FAULT_INV_S_ENTRY)
    {
        func_ptr = (funcptr_t)((uint32_t)&PRINTF_NSE + 4);
        func_ptr("Invalid Test Case\r\n");
    }
    /* END OF TEST EXAMPLE 2 */

    /**************************************************************************
     * TEST EXAMPLE 5 - Invalid data access from normal world
     * In this example the pointer is set to address defined by DEMO_NONSEC_ADDRESS.
     * This address has non-secure attribute in SAU but it has
     * secure attribute in AHB secure controller.
     * If data is read from this address, the data bus error is generated
     * Compare to test example 3, this error is catched by AHB secure controller,
     * not by SAU, because in SAU this address is non-secure so the access from
     * normal world is correct from SAU perspective.
     **************************************************************************/
    if (testCaseNumber == FAULT_INV_NS_DATA2_ACCESS)
    {
        test_ptr   = (uint32_t *)(DEMO_NONSEC_ADDRESS);
        test_value = *test_ptr;
    }
    PRINTF_NSE("This is a text printed from normal world!\r\n");

    result = StringCompare_NSE(&strcmp, "Test1\r\n", "Test2\r\n");
    if (result == 0)
    {
        PRINTF_NSE("Both strings are equal!\r\n");
    }
    else
    {
        PRINTF_NSE("Both strings are not equal!\r\n");
    }
    while (1)
    {
    }
}
