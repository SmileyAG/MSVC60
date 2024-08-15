/***
* align.c - Aligned allocation, reallocation or freeing of memory in the heap
*
*       Copyright (c) 1989-1999, Microsoft Corporation. All rights reserved.
*
* Purpose:
*       Defines the _aligned_malloc(),
*                   _aligned_realloc(),
*                   _aligned_offset_malloc(),
*                   _aligned_offset_realloc() and
*                   _aligned_free() functions.
*
* Revision History:
*       11-05-99  GB    Module created.
*       01-20-99  GB    Fixed realloc to carry over the current memory chunk.
*
*******************************************************************************/

#ifdef CRTDLL
#undef CRTDLL
#define _DLL
#endif  /* CRTDLL */


#include <stddef.h>
#include <stdlib.h>
#include <crtdbg.h>
#include <errno.h>
#include <malloc.h>
#include <memory.h>

#define IS_2_POW_N(X)   (((X)&(X-1)) == 0)


/***
* void *_aligned_malloc(size_t size, size_t alignment)
*       - Get a block of aligned memory from the heap.
*
* Purpose:
*       Allocate of block of aligned memory aligned on the alignment of at least
*       size bytes from the heap and return a pointer to it.
*
* Entry:
*       size_t size - size of block requested
*       size_t alignment - alignment of memory
*
* Exit:
*       Sucess: Pointer to memory block
*       Faliure: Null
*******************************************************************************/

void * __cdecl _aligned_malloc(
    size_t size,
    size_t alignment
    )
{
    size_t ptr, r_ptr;
    size_t *reptr;

    if (!IS_2_POW_N(alignment))
    {
        _ASSERTE(("alignment must be a power of 2", 0));
        errno = EINVAL;
        return NULL;
    }

    alignment = (alignment > sizeof(void *) ? alignment : sizeof(void *));

    if ((ptr = (size_t)malloc(size + alignment + sizeof(void *))) == (size_t)NULL)
        return NULL;

    r_ptr = (ptr + alignment + sizeof(void *)) & ~(alignment -1);
    reptr = (size_t *)(r_ptr - sizeof(void *));
    *reptr = ptr;

    return (void *)r_ptr;
}

/***
* void *_aligned_offset_malloc(size_t size, size_t alignment, int offset)
*       - Get a block of memory from the heap.
*
* Purpose:
*       Allocate a block of memory which is shifted by offset from alignment of
*       at least size bytes from the heap and return a pointer to it.
*
* Entry:
*       size_t size - size of block of memory
*       size_t alignment - alignment of memory
*       size_t offset - offset of memory from the alignment
*
* Exit:
*       Sucess: Pointer to memory block
*       Faliure: Null
*******************************************************************************/


void * __cdecl _aligned_offset_malloc(
    size_t size,
    size_t alignment,
    size_t offset
    )
{
    size_t ptr, r_ptr, t_ptr;
    size_t *reptr;

    if (!IS_2_POW_N(alignment))
    {
        _ASSERTE(("alignment must be a power of 2", 0));
        errno = EINVAL;
        return NULL;
    }

    alignment = (alignment > sizeof(void *) ? alignment : sizeof(void *));

    /* round up the neartest paragraph */
    t_ptr = (offset + sizeof(void *) -1)&~(sizeof(void *) -1);

    if ( (ptr = (size_t) malloc(t_ptr + size + alignment + sizeof(void *))) == (size_t)NULL)
        return NULL;

    r_ptr = (ptr + alignment + sizeof(void *) + t_ptr) & ~(alignment -1);
    reptr = (size_t *)(r_ptr - t_ptr - sizeof(void *));
    r_ptr -= offset;
    *reptr = ptr;

    return (void *)r_ptr;
}

/***
*
* void *_aligned_malloc(size_t size, size_t alignment)
*       - Reallocate a block of aligned memory from the heap.
*
* Purpose:
*       Reallocates of block of aligned memory aligned on the alignment of at
*       least size bytes from the heap and return a pointer to it. Size can be
*       either greater or less than the original size of the block.
*       The reallocation may result in moving the block as well as changing the
*       size.
*
* Entry:
*       void *memblock - pointer to block in the heap previously allocated by
*               call to _aligned_malloc(), _aligned_offset_malloc(),
*               _aligned_realloc() or _aligned_offset_realloc().
*       size_t size - size of block requested
*       size_t alignment - alignment of memory
*
* Exit:
*       Sucess: Pointer to re-allocated memory block
*       Faliure: Null
*
*******************************************************************************/

void * __cdecl _aligned_realloc(
    void *memblock,
    size_t size,
    size_t alignment
    )
{
    size_t ptr, r_ptr, s_ptr;
    size_t *reptr;
    size_t mov_sz;

    if (memblock == NULL)
    {
        errno = EINVAL;
        return NULL;
    }

    s_ptr = (size_t)memblock;

    /* ptr points to the pointer to starting of the memory block */
    s_ptr = (s_ptr & ~(sizeof(void *) -1)) - sizeof(void *);

    /* ptr is the pointer to the start of memory block*/
    s_ptr = *((size_t *)s_ptr);

    if (!IS_2_POW_N(alignment))
    {
        _ASSERTE(("alignment must be a power of 2", 0));
        errno = EINVAL;
        return NULL;
    }

    alignment = (alignment > sizeof(void *) ? alignment : sizeof(void *));

    /* Calculate the size that is needed to move */
    mov_sz = _msize((void *)s_ptr) - ((size_t)memblock - s_ptr);

    if ((ptr = (size_t)malloc(size + alignment + sizeof(void *))) == (size_t)NULL)
        return NULL;

    r_ptr = (ptr + alignment + sizeof(void *)) & ~(alignment -1);
    reptr = (size_t *)(r_ptr - sizeof(void *));
    *reptr = ptr;

    /* copy the content from older memory location to newer one */
    memcpy((void *)r_ptr, memblock, mov_sz > size ? size: mov_sz);
    free((void *)s_ptr);

    return (void *)r_ptr;
}

/***
*
* void *_aligned_offset_malloc(size_t size, size_t alignment, int offset)
*       - Reallocate a block of memory from the heap.
*
* Purpose:
*       Reallocates a block of memory which is shifted by offset from
*       alignment of at least size bytes from the heap and return a pointer
*       to it. Size can be either greater or less than the original size of the
*       block.
*
* Entry:
*       void *memblock - pointer to block in the heap previously allocated by
*               call to _aligned_malloc(), _aligned_offset_malloc(),
*               _aligned_realloc() or _aligned_offset_realloc().
*       size_t size - size of block of memory
*       size_t alignment - alignment of memory
*       size_t offset - offset of memory from the alignment
*
* Exit:
*       Sucess: Pointer to the re-allocated memory block
*       Faliure: Null
*
*******************************************************************************/

void * __cdecl _aligned_offset_realloc(
    void *memblock,
    size_t size,
    size_t alignment,
    size_t offset
    )
{
    size_t ptr, r_ptr, t_ptr, s_ptr;
    size_t *reptr;
    size_t mov_sz;

    if (memblock == NULL)
    {
        errno = EINVAL;
        return NULL;
    }

    s_ptr = (size_t)memblock;

    /* ptr points to the pointer to starting of the memory block */
    s_ptr = (s_ptr & ~(sizeof(void *) -1)) - sizeof(void *);

    /* ptr is the pointer to the start of memory block*/
    s_ptr = *((size_t *)s_ptr);

    if (!IS_2_POW_N(alignment))
    {
        _ASSERTE(("alignment must be a power of 2", 0));
        errno = EINVAL;
        return NULL;
    }

    /* Calculate the size that is needed to move */
    mov_sz = _msize((void *)s_ptr) - ((size_t)memblock - s_ptr);

    alignment = (alignment > sizeof(void *) ? alignment : sizeof(void *));
    /* round up the offest to the neartest paragraph */
    t_ptr = (offset + sizeof(void *) -1)&~(sizeof(void *) -1);

    if ((ptr = (size_t) malloc(t_ptr + size + alignment + sizeof(void *))) == (size_t)NULL)
        return NULL;

    r_ptr = (ptr + alignment + sizeof(void *) + t_ptr) & ~(alignment -1);
    reptr = (size_t *)(r_ptr - t_ptr - sizeof(void *));
    r_ptr -= offset;
    *reptr = ptr;

    /* copy the content from older memory location to newer one */
    memcpy((void *)r_ptr, memblock, mov_sz > size ? size: mov_sz);
    free((void *)s_ptr);

    return (void *)r_ptr;
}


/***
*
* void *_aligned_free(void *memblock)
*       - Free the memory which was allocated using _aligned_malloc or
*       _aligned_offset_memory
*
* Purpose:
*       Frees the algned memory block which was allocated using _aligned_malloc
*       or _aligned_memory.
*
* Entry:
*       void * memblock - pointer to the block of memory
*
*******************************************************************************/


void  __cdecl _aligned_free(void *memblock)
{
    size_t ptr;

    if (memblock == NULL)
        return;

    ptr = (size_t)memblock;

    /* ptr points to the pointer to starting of the memory block */
    ptr = (ptr & ~(sizeof(void *) -1)) - sizeof(void *);

    /* ptr is the pointer to the start of memory block*/
    ptr = *((size_t *)ptr);
    free((void *)ptr);
}
