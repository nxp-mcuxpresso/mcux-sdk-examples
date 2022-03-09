/*
* Copyright 2020 NXP
*
* NXP Confidential. This software is owned or controlled by NXP and may only
* be used strictly in accordance with the applicable license terms found in
* file LICENSE.txt
*/


/** @file
 *  Header file defining the standard PL_types for use in the application layer
 *  interface of VIT module
 */

#ifndef PL_MEMORY_REGION_H
#define PL_MEMORY_REGION_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



/****************************************************************************************/
/*                                                                                      */
/*  definitions                                                                         */
/*                                                                                      */
/****************************************************************************************/

/* Memory table*/
#define PL_MEMREGION_PERSISTENT_SLOW_DATA      0   ///< Offset to the instance memory region
#define PL_MEMREGION_PERSISTENT_FAST_DATA      1   ///< Offset to the persistent data memory region
#define PL_MEMREGION_PERSISTENT_COEF           2   ///< Offset to the persistent coefficient memory region
#define PL_MEMREGION_TEMPORARY                 3   ///< Offset to temporary memory region
#define PL_NR_MEMORY_REGIONS                   4   ///< Number of memory regions
#define PL_MEMREGION_ROM                       5   ///< memory ROM. As it is an independant memory it is not in part of PL_NR_MEMORY_REGIONS

/****************************************************************************************/
/*                                                                                      */
/*  Basic types                                                                         */
/*                                                                                      */
/****************************************************************************************/


/****************************************************************************************/
/*                                                                                      */
/*  Standard Enumerated types                                                           */
/*                                                                                      */
/****************************************************************************************/

/**
The @ref PL_MemoryTypes_en enumerated type identifies the memory region types so that they can be correctly placed in memory
by the calling application.
The module initially has no permanent memory storage and makes no use of persistent memory allocation internally.
The calling application must allocate memory for the module to use.

Four memory regions are required:
@li @ref PL_MEMREGION_PERSISTENT_SLOW_DATA : this type of memory is used to store all the control data that needs to be saved between two consecutive calls to the process function.
@li @ref PL_MEMREGION_PERSISTENT_FAST_DATA : this type of memory is used to store data such as filter history
@li @ref PL_MEMREGION_PERSISTENT_COEF : this type of memory is used to store filter coefficients.
@li @ref PL_MEMREGION_TEMPORARY (scratch): this type of memory is used to store temporary data. This memory can be reused by the application in between calls to the process function.

This collection of memory regions forms the module instance.

Typically the memory is allocated by the application dynamically; however, it can be allocated statically if required.
The sizes of the memory regions can be found by running the GetMemoryTable functions on a simulator and noting
the returned values. Alternatively contact NXP who can provide the figures.
It is possible that these memory sizes will change between release versions of the library and hence the dynamic memory allocation method is preferred where possible.
On some target platforms the placement of memory regions is critical for achieving optimal performance of the module.
*/
typedef enum
{
    PL_PERSISTENT_SLOW_DATA    = PL_MEMREGION_PERSISTENT_SLOW_DATA,       ///< Persistent slow memory region
    PL_PERSISTENT_FAST_DATA    = PL_MEMREGION_PERSISTENT_FAST_DATA,       ///< Persistent fast memory region
    PL_PERSISTENT_FAST_COEF    = PL_MEMREGION_PERSISTENT_COEF,            ///< Persisten memory for coefficient storage
    PL_TEMPORARY_FAST          = PL_MEMREGION_TEMPORARY,                  ///< Temporary memory region
    PL_MEMORYTYPE_DUMMY        = (PL_TEMPORARY_FAST + 1),
} PL_MemoryTypes_en;


/**
The @ref PL_MemoryRegion_st type defines a memory region by specifying its size in bytes, its region type and its base pointer.
@see PL_MemoryTypes_en
*/
typedef struct
{
    PL_UINT32                  Size;                   ///< The size of the memory region in bytes
    PL_MemoryTypes_en          Type;                   ///< Type of memory region
    void                       *pBaseAddress;          ///< Pointer to the memory region base address
} PL_MemoryRegion_st;

/**
The PL_MemoryTable_st type defines the memory requirements of the module as an array of region definitions.
The number of required memory regions is given by the constant @ref PL_NR_MEMORY_REGIONS
@see PL_MemoryRegion_st
*/
typedef struct
{
    PL_MemoryRegion_st         Region[PL_NR_MEMORY_REGIONS];  ///< One definition of all memory regions
} PL_MemoryTable_st;

/**
The PL_ContextTable_st type defines a memory region by specifying its size in bytes and its base pointer.
@see PL_ContextTable_st
*/
typedef struct
{
    PL_UINT32 ContextTableLength;
    PL_INT8   *pContext;
} PL_ContextTable_st;


/****************************************************************************************/
/*                                                                                      */
/*  Standard Function Prototypes                                                        */
/*                                                                                      */
/****************************************************************************************/


/****************************************************************************************/
/*                                                                                      */
/*  End of file                                                                         */
/*                                                                                      */
/****************************************************************************************/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* PL_MEMORY_REGION_H */
