/*
 *  @file usermem.h
 *  @author Mateusz Piesta (mateusz.piesta@mudita.com)
 *  @date 30 lip 2018
 *  @brief DYnamic memory allocator for user space
 *  @copyright Copyright (C) 2018 mudita.com
 *  @details
 */

#ifndef USERMEM_H_
#define USERMEM_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Sets heap size in SDRAM
 */
#ifndef PROJECT_CONFIG_USER_DYNMEM_SIZE
#define USERMEM_TOTAL_HEAP_SIZE	0
#else
#define USERMEM_TOTAL_HEAP_SIZE	PROJECT_CONFIG_USER_DYNMEM_SIZE
#endif

/* Assumes 8bit bytes! */
#define heapBITS_PER_BYTE		( ( size_t ) 8 )
/**
 * Document specified below is must to read. It explains cached memory regions alignment requirements
 * and so on.
 * https://www.avrfreaks.net/sites/default/files/forum_attachments/AN-15679_SAMS70_E70_Cache_Coherence.pdf
 */

#define usermemBYTE_ALIGNMENT	   (4)
#define usermemBYTE_ALIGNMENT_MASK (usermemBYTE_ALIGNMENT - 1)

void *usermalloc(size_t xWantedSize);

void userfree(void *pv);

size_t usermemGetFreeHeapSize(void);

size_t usermemGetMinimumEverFreeHeapSize(void);

void *userrealloc(void *pv, size_t xWantedSize);

void usermem_check_pool(void);

#ifdef __cplusplus
}
#endif


#endif /* USERMEM_H_ */
