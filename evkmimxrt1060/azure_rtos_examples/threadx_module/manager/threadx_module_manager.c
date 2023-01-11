#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "tx_api.h"
#include "txm_module.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_STACK_SIZE      1024

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Define the ThreadX object control blocks...  */
TX_THREAD               module_manager;
TXM_MODULE_INSTANCE     module1;
TXM_MODULE_INSTANCE     module2;

/* Define the object pool area.  */
static ULONG manager_stack[DEMO_STACK_SIZE / sizeof(ULONG)];
static UCHAR            mgr_memory[16384];
static UCHAR            object_memory[16384];
/* Define the count of memory faults.  */
static ULONG            memory_faults;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* Define thread prototypes.  */
void module_manager_entry(ULONG thread_input);
void thread_entry(ULONG thread_input);

/*******************************************************************************
 * Code
 ******************************************************************************/
/* Define fault handler.  */
VOID module_fault_handler(TX_THREAD *thread, TXM_MODULE_INSTANCE *module)
{
    PRINTF("[Error] <Module \"%s\": Thread \"%s\">: Memory Fault\r\n",
           module->txm_module_instance_name, thread->tx_thread_name);
    PRINTF("thread addr = 0x%08X\r\n", thread);

    /* Just increment the fault counter.   */
    memory_faults++;
}

/* Define main entry point.  */
int main()
{
    /* Init board hardware. */
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("ThreadX Module Manager example ...\r\n");

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();

    return 0;
}

/* Define what the initial system looks like.  */
void tx_application_define(void *first_unused_memory)
{
    TX_PARAMETER_NOT_USED(first_unused_memory);

    tx_thread_create(&module_manager, "Module Manager Thread", module_manager_entry, 0,  
                     manager_stack, DEMO_STACK_SIZE, 1, 1, TX_NO_TIME_SLICE, TX_AUTO_START);
}

/* Define the test threads.  */
void module_manager_entry(ULONG thread_input)
{
    /* Initialize the module manager.   */
    PRINTF("Mgr: Initializing memory from mgr_memory for modules\r\n");
    txm_module_manager_initialize(mgr_memory, sizeof(mgr_memory));

    PRINTF("Mgr: Creating object pool for modules\r\n");
    txm_module_manager_object_pool_create(object_memory, sizeof(object_memory));

    /* Register a fault handler.  */
    txm_module_manager_memory_fault_notify(module_fault_handler);

    /* Load the module that is already there, in this example it is placed there by the multiple image download.  */
    PRINTF("Mgr: Loading \"module\" from 0x60080000\r\n");
    txm_module_manager_in_place_load(&module1, "module", (VOID *) 0x60080000);

    /* Start the module.  */
    PRINTF("Mgr: Starting \"module\"\r\n");
    txm_module_manager_start(&module1);

    /* Sleep for a while....  */
    PRINTF("Mgr: Let modules run for 5 seconds\r\n");
    tx_thread_sleep(500);

    /* Stop the module.  */
    PRINTF("Mgr: Stopping \"module\"\r\n");
    txm_module_manager_stop(&module1);

    /* Unload the module.  */
    PRINTF("Mgr: Unloading \"module\"\r\n");
    txm_module_manager_unload(&module1);

    /* Now just spin...  */
    while(1)
    {
        tx_thread_sleep(100);
    }
}
