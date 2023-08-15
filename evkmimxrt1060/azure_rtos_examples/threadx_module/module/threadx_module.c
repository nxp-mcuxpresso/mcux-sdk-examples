/* This is a small demo of the high-performance ThreadX kernel running as a module. It includes 
   examples of eight threads of different priorities, using a message queue, semaphore, mutex, 
   event flags group, byte pool, and block pool.  */

/* Specify that this is a module!  */
#define TXM_MODULE

/* Include the ThreadX module definitions.  */
#include "txm_module.h"
#include "rom_api.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_NAME      "Module Test"
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define PRINTF        g_romapiTree->Printf

#define DEMO_STACK_SIZE         1024
#define DEMO_BYTE_POOL_SIZE     6000
#define DEMO_BLOCK_POOL_SIZE    100
#define DEMO_QUEUE_SIZE         100

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Define the pool space in the bss section of the module. ULONG is used to 
   get the word alignment.  */
ULONG demo_module_pool_space[DEMO_BYTE_POOL_SIZE / 4];

/* Define the ThreadX object control blocks...  */
TX_THREAD            *thread_0;
TX_THREAD            *thread_1;
TX_QUEUE             *queue_0_rx;
TX_QUEUE             *queue_0_tx;
TX_QUEUE             *queue_1_rx;
TX_QUEUE             *queue_1_tx;
TX_BYTE_POOL         *byte_pool;

/* Define the counters used in the demo application...  */
ULONG thread_0_messages_sent;
ULONG thread_1_messages_sent;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* Define thread prototypes.  */
void thread_0_entry(ULONG thread_input);
void thread_1_entry(ULONG thread_input);

/*******************************************************************************
 * Code
 ******************************************************************************/
/* Define the module start function.  */
void demo_module_start(ULONG id)
{
    CHAR *pointer = TX_NULL;

    PRINTF("Start the module app(%s).\r\n", APP_NAME);

    txm_module_object_allocate((void*)&thread_0, sizeof(TX_THREAD));
    txm_module_object_allocate((void*)&thread_1, sizeof(TX_THREAD));
    txm_module_object_allocate((void*)&queue_0_tx, sizeof(TX_QUEUE));
    txm_module_object_allocate((void*)&queue_1_tx, sizeof(TX_QUEUE));
    txm_module_object_allocate((void*)&byte_pool, sizeof(TX_BYTE_POOL));

    tx_byte_pool_create(byte_pool, "module byte pool", (UCHAR*)demo_module_pool_space, DEMO_BYTE_POOL_SIZE);

    tx_byte_allocate(byte_pool, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);
    tx_thread_create(thread_0, "module thread 0", thread_0_entry, 0,  
                     pointer, DEMO_STACK_SIZE, 1, 1, TX_NO_TIME_SLICE, TX_AUTO_START);
    tx_byte_allocate(byte_pool, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);
    tx_thread_create(thread_1, "module thread 1", thread_1_entry, 0,
                     pointer, DEMO_STACK_SIZE, 1, 1, TX_NO_TIME_SLICE, TX_AUTO_START);

    /* Allocate the message queue.  */
    tx_byte_allocate(byte_pool, (VOID **) &pointer, DEMO_QUEUE_SIZE * sizeof(ULONG), TX_NO_WAIT);
    /* Create the message queue to send to thread 1.  */
    tx_queue_create(queue_0_tx, "queue_0_to_1", TX_1_ULONG, pointer, DEMO_QUEUE_SIZE * sizeof(ULONG));
    tx_byte_allocate(byte_pool, (VOID **) &pointer, DEMO_QUEUE_SIZE * sizeof(ULONG), TX_NO_WAIT);
    tx_queue_create(queue_1_tx, "queue_1_to_0", TX_1_ULONG, pointer, DEMO_QUEUE_SIZE * sizeof(ULONG));
}

void thread_0_entry(ULONG thread_input)
{
    ULONG   received_message;
    UINT    status;

    PRINTF("Start Module:Thread 0 ...\r\n");
    do {
        /* Get the message queue created by thread 1.  */
        status = txm_module_object_pointer_get(TXM_QUEUE_OBJECT, "queue_1_to_0", (void*) &queue_0_rx);
    } while(status != TX_SUCCESS);

    /* This thread sends messages to a queue that thread 1 reads.
       This thread also receives messages from thread 1.  */
    while (1) {
        /* Send message to queue.  */
        status = tx_queue_send(queue_0_tx, &thread_0_messages_sent, TX_WAIT_FOREVER);

        /* Check completion status.  */
        if (status != TX_SUCCESS)
            break;
        PRINTF("Start Module:Thread 0 sends message ==> %d\r\n", thread_0_messages_sent);

        /* Increment the message sent.  */
        thread_0_messages_sent++;
        status = tx_queue_receive(queue_0_rx, &received_message, TX_WAIT_FOREVER);

        /* Check completion status.  */
        if (status != TX_SUCCESS)
            break;
        PRINTF("Start Module:Thread 0 receives message ==> %d\r\n", received_message);
        tx_thread_sleep(100);
    }
}

void thread_1_entry(ULONG thread_input)
{
    ULONG   received_message;
    UINT    status;

    PRINTF("Start Module:Thread 1 ...\r\n");
    do {
        /* Get the message queue created by thread 0.  */
        status = txm_module_object_pointer_get(TXM_QUEUE_OBJECT, "queue_0_to_1", (void*) &queue_1_rx);
    } while(status != TX_SUCCESS);

    /* This thread sends messages to a queue that thread 0 reads.
       This thread also receives messages from thread 0.  */
    while (1) {
        /* Send message to queue.  */
        status = tx_queue_send(queue_1_tx, &thread_1_messages_sent, TX_WAIT_FOREVER);

        /* Check completion status.  */
        if (status != TX_SUCCESS)
            break;
        PRINTF("Start Module:Thread 1 sends message ==> %d\r\n", thread_1_messages_sent);

        /* Increment the message sent.  */
        thread_1_messages_sent++;
        status = tx_queue_receive(queue_1_rx, &received_message, TX_WAIT_FOREVER);

        /* Check completion status.  */
        if (status != TX_SUCCESS)
            break;
        PRINTF("Start Module:Thread 1 receives message ==> %d\r\n", received_message);
        tx_thread_sleep(100);
    }
}

