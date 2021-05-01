/*
 *  @file usermem_tlsf.c
 *  @author Marek Niepiekło (marek.niepieklo@mudita.com)
 *  @date 1 maj 2021
 *  @brief TLSF wrapper for dynamic memory allocator for user space
 *  @copyright Copyright (C) 2021 mudita.com
 *  @details
 */

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include "FreeRTOS.h"
#include "task.h"
#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include "usermem.h"
#include "tlsf/tlsf.h"
#include "macros.h"

#include <stddef.h>
#include <string.h>

/* Allocate the memory for the heap. */
CACHEABLE_SECTION_SDRAM_ALIGN(static uint8_t userUcHeap[ USERMEM_TOTAL_HEAP_SIZE ], usermemBYTE_ALIGNMENT) ;

static tlsf_t userHeapPool;

/* Keeps track of the number of free bytes remaining, but says nothing about
fragmentation. */
static size_t xFreeBytesRemaining = 0U;
static size_t xMinimumEverFreeBytesRemaining = 0xFFFFFFFF;

/*
 * Called automatically to setup the required heap structures the first time
 * pvPortMalloc() is called.
 */
static void prvHeapInit( void );

static UBaseType_t savedInterruptStatus;

static void EnterCriticalSection(void)
{
    if (isIRQ()) {
        savedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
    }
    else {
        vPortEnterCritical();
    }
}

static void ExitCriticalSection(void)
{
    if (isIRQ()) {
        portCLEAR_INTERRUPT_MASK_FROM_ISR(savedInterruptStatus);
    }
    else {
        vPortExitCritical();
    }
}

void usermem_check_pool(void)
{
    (void)tlsf_check_pool((char*)userUcHeap + tlsf_size());
}

void *usermalloc(size_t xWantedSize)
{
	void *pvReturn = NULL;

    EnterCriticalSection();
    {
        /* If this is the first call to malloc then the heap will require
        initialisation to setup the list of free blocks. */
        if(userHeapPool == NULL)
        {
            prvHeapInit();
        }

        pvReturn = tlsf_malloc(userHeapPool, xWantedSize);
        xFreeBytesRemaining -= xWantedSize;

        if (xFreeBytesRemaining < xMinimumEverFreeBytesRemaining) {
            xMinimumEverFreeBytesRemaining = xFreeBytesRemaining;
        }
    }
    ExitCriticalSection();
    #if( configUSE_MALLOC_FAILED_HOOK == 1 )
    {
        if( pvReturn == NULL )
        {
            extern void vApplicationMallocFailedHook( void );
            vApplicationMallocFailedHook();
        }
    }
    #endif

    configASSERT( ( ( ( size_t ) pvReturn ) & ( size_t ) usermemBYTE_ALIGNMENT_MASK ) == 0 );

    return pvReturn;
}

void userfree(void *pv)
{
    EnterCriticalSection();
    // (void)tlsf_check(userHeapPool);
    xFreeBytesRemaining += tlsf_block_size(pv);
    tlsf_free(userHeapPool, pv);
    ExitCriticalSection();
}

size_t usermemGetFreeHeapSize(void)
{
    return xFreeBytesRemaining;
}

size_t usermemGetMinimumEverFreeHeapSize(void)
{
    return xMinimumEverFreeBytesRemaining;
}

void *userrealloc(void *pv, size_t xWantedSize)
{
    void *new = NULL;
    EnterCriticalSection();
    // (void)tlsf_check(userHeapPool);
    xFreeBytesRemaining += tlsf_block_size(pv);
    new = tlsf_realloc(userHeapPool, pv, xWantedSize);
    xFreeBytesRemaining -= xWantedSize;
    ExitCriticalSection();

    return new;
}

static void prvHeapInit( void )
{
    userHeapPool = tlsf_create_with_pool(userUcHeap, USERMEM_TOTAL_HEAP_SIZE);

    if (userHeapPool == NULL) {
        abort();
    }

    xMinimumEverFreeBytesRemaining = 0xFFFFFFFF;
    xFreeBytesRemaining = USERMEM_TOTAL_HEAP_SIZE;
}
