
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fx_api.h"
#ifdef FX_ENABLE_FAULT_TOLERANT
#include "fx_fault_tolerant.h"
#endif /* FX_ENABLE_FAULT_TOLERANT */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_STACK_SIZE    2048

#define DEMO_SECTOR_SIZE   512
#define DEMO_SECTOR_COUNT  64
#define DEMO_RAM_DISK_SIZE (DEMO_SECTOR_SIZE * DEMO_SECTOR_COUNT)

/* Buffer for FileX FX_MEDIA sector cache. This must be large enough for at least one
   sector, which are typically 512 bytes in size.  */

/*******************************************************************************
 * Variables
 ******************************************************************************/

unsigned char media_memory[512];

#ifdef FX_ENABLE_FAULT_TOLERANT
UCHAR fault_tolerant_memory[FX_FAULT_TOLERANT_MAXIMUM_LOG_FILE_SIZE];
#endif /* FX_ENABLE_FAULT_TOLERANT */

CHAR demo_stack[DEMO_STACK_SIZE];

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* Define RAM device driver entry.  */

VOID _fx_ram_driver(FX_MEDIA *media_ptr);

/* Define thread prototypes.  */

void thread_0_entry(ULONG thread_input);

/* Define FileX global data structures.  */

FX_MEDIA ram_disk;
FX_FILE my_file;
CHAR ram_disk_memory[DEMO_RAM_DISK_SIZE];

/* Define ThreadX global data structures.  */

TX_THREAD thread_0;
ULONG thread_0_counter;

/*******************************************************************************
 * Code
 ******************************************************************************/
int main(void)
{
    /* Initialize the board.  */
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("FILEX example.....\r\n");

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();

    return 0;
}

/* Define what the initial system looks like.  */

void tx_application_define(void *first_unused_memory)
{
    TX_THREAD_NOT_USED(first_unused_memory);

    /* Put system definition stuff in here, e.g. thread creates and other assorted
       create information.  */

    /* Create the main thread.  */
    tx_thread_create(&thread_0, "thread 0", thread_0_entry, 0, (VOID *)demo_stack, DEMO_STACK_SIZE, 1, 1,
                     TX_NO_TIME_SLICE, TX_AUTO_START);

    /* Initialize FileX.  */
    fx_system_initialize();
}

void thread_0_entry(ULONG thread_input)
{
    UINT status;
    ULONG actual;
    CHAR local_buffer[30];

    /* Format the RAM disk - the memory for the RAM disk was setup in
       tx_application_define above.  */
#ifdef FX_ENABLE_EXFAT
    fx_media_exFAT_format(&ram_disk,
                          _fx_ram_driver,       // Driver entry
                          (VOID *)ram_disk_memory,      // RAM disk memory pointer
                          media_memory,         // Media buffer pointer
                          sizeof(media_memory), // Media buffer size
                          "MY_RAM_DISK",        // Volume Name
                          1,                    // Number of FATs
                          0,                    // Hidden sectors
                          DEMO_SECTOR_COUNT,    // Total sectors
                          DEMO_SECTOR_SIZE,     // Sector size
                          8,                    // exFAT Sectors per cluster
                          12345,                // Volume ID
                          1);                   // Boundary unit
#else
    fx_media_format(&ram_disk,
                    _fx_ram_driver,       // Driver entry
                    (VOID *)ram_disk_memory,    // RAM disk memory pointer
                    media_memory,         // Media buffer pointer
                    sizeof(media_memory), // Media buffer size
                    "MY_RAM_DISK",        // Volume Name
                    1,                    // Number of FATs
                    32,                   // Directory Entries
                    0,                    // Hidden sectors
                    DEMO_SECTOR_COUNT,    // Total sectors
                    DEMO_SECTOR_SIZE,     // Sector size
                    8,                    // Sectors per cluster
                    1,                    // Heads
                    1);                   // Sectors per track
#endif /* FX_ENABLE_EXFAT */

    /* Loop to repeat the demo over and over!  */
    do
    {
        PRINTF("Open RAM Disk.....\r\n");
        /* Open the RAM disk.  */
        status =
            fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, media_memory, sizeof(media_memory));

        /* Check the media open status.  */
        if (status != FX_SUCCESS)
        {
            /* Error, break the loop!  */
            break;
        }

#ifdef FX_ENABLE_FAULT_TOLERANT
        status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_memory, sizeof(fault_tolerant_memory));

        if (status != FX_SUCCESS)
        {
            /* Fault tolerant enable error, break the loop.  */
            break;
        }
#endif /* FX_ENABLE_FAULT_TOLERANT */

        PRINTF("Creat TEST.TXT.....\r\n");
        /* Create a file called TEST.TXT in the root directory.  */
        status = fx_file_create(&ram_disk, "TEST.TXT");

        /* Check the create status.  */
        if (status != FX_SUCCESS)
        {
            /* Check for an already created status. This is expected on the
               second pass of this loop!  */
            if (status != FX_ALREADY_CREATED)
            {
                /* Create error, break the loop.  */
                break;
            }
        }

        /* Open the test file.  */
        status = fx_file_open(&ram_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);

        /* Check the file open status.  */
        if (status != FX_SUCCESS)
        {
            /* Error opening file, break the loop.  */
            break;
        }

        /* Seek to the beginning of the test file.  */
        status = fx_file_seek(&my_file, 0);

        /* Check the file seek status.  */
        if (status != FX_SUCCESS)
        {
            /* Error performing file seek, break the loop.  */
            break;
        }

        PRINTF("Write TEST.TXT.....\r\n");
        /* Write a string to the test file.  */
        status = fx_file_write(&my_file, " ABCDEFGHIJKLMNOPQRSTUVWXYZ\n", 28);

        /* Check the file write status.  */
        if (status != FX_SUCCESS)
        {
            /* Error writing to a file, break the loop.  */
            break;
        }

        /* Seek to the beginning of the test file.  */
        status = fx_file_seek(&my_file, 0);

        /* Check the file seek status.  */
        if (status != FX_SUCCESS)
        {
            /* Error performing file seek, break the loop.  */
            break;
        }

        /* Read the first 28 bytes of the test file.  */
        status = fx_file_read(&my_file, local_buffer, 28, &actual);

        /* Check the file read status.  */
        if ((status != FX_SUCCESS) || (actual != 28))
        {
            /* Error reading file, break the loop.  */
            break;
        }

        PRINTF("Close TEST.TXT.....\r\n");
        /* Close the test file.  */
        status = fx_file_close(&my_file);

        /* Check the file close status.  */
        if (status != FX_SUCCESS)
        {
            /* Error closing the file, break the loop.  */
            break;
        }

        PRINTF("Close RAM Disk.....\r\n");
        /* Close the media.  */
        status = fx_media_close(&ram_disk);

        /* Check the media close status.  */
        if (status != FX_SUCCESS)
        {
            /* Error closing the media, break the loop.  */
            break;
        }

        /* Increment the thread counter, which represents the number
           of successful passes through this loop.  */
        thread_0_counter++;

        /* sleep for a while */
        tx_thread_sleep(4 * TX_TIMER_TICKS_PER_SECOND);

    } while (1);

    /* If we get here the FileX test failed!  */
    return;
}
