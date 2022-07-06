// Can provide alternatives to malloc and free
// by defining the following macros:
#ifndef PISTON_MEM_MALLOC
#include <stdlib.h>
#define PISTON_MEM_MALLOC(x) malloc(x)
#define PISTON_MEM_FREE(x) free(x)
#endif

// Must supply alternatives for these macros if not using the default.
#ifndef PISTON_MEM_NOSTDIO
#include <stdio.h>
#define PISTON_MEM_PRINTF_RETURN(R, fmt, ...) { printf(fmt, ...); return R; }
#else
#define PISTON_MEM_PRINTF_RETURN(R, fmt, ...) { return R; }
#endif

// Must supply altenatives to these types if
// you define PISTON_MEM_NOSTDINT.
#ifndef PISTON_MEM_NOSTDINT
#include <stdint.h>
#define PISTON_MEM_U8 uint8_t
#define PISTON_MEM_U16 uint16_t
#define PISTON_MEM_U32 uint32_t
#define PISTON_MEM_U64 uint64_t
#define PISTON_MEM_USIZE size_t
#endif

#ifndef __PISTON_INCLUDE_PISTON_MEM_H__
#define __PISTON_INCLUDE_PISTON_MEM_H__

#ifdef PISTON_MEM_STATIC
#define PISTON_MEM_DEF static
#else
#define PISTON_MEM_DEF extern
#endif

// PISTON_MEM_ALLOCATOR_FLAG_BUMP:
// Allocate memory by moving a pointer, can only free from the end.
// PISTON_MEM_ALLOCATOR_FLAG_REUSE:
// Try to reuse freed memory.
// PISTON_MEM_ALLOCATOR_FLAG_EXPAND:
// Expand heap when out of space.
enum piston_mem_allocator_flags {
	PISTON_MEM_ALLOCATOR_FLAG_BUMP = 1 << 0,
	PISTON_MEM_ALLOCATOR_FLAG_REUSE = 1 << 1,
	PISTON_MEM_ALLOCATOR_FLAG_EXPAND = 1 << 2,
};

unsigned piston_mem_allocator_create(enum piston_mem_allocator_flags flags);
void piston_mem_set_active_allocator(unsigned id);

void *piston_mem_alloc(size_t size);
void *piston_mem_realloc(void *ptr, size_t size);
void piston_mem_free(void *ptr);

#endif

#ifdef PISTON_MEM_IMPLEMENTATION

struct piston_mem_allocator_block_64 {
	u64 block_map;
	void *data;
};

struct piston_mem_allocator_block_32 {
	u32 block_map;
	void *data;
};

struct piston_mem_allocator_block_16 {
	u16 block_map;
	void *data;
};

struct piston_mem_allocator_block_8 {
	u8 block_map;
	void *data;
};

static struct piston_mem_allocator *piston_mem_allocator_array = NULL;
static unsigned piston_mem_allocator_count = 0;

PISTON_MEM_DEF unsigned piston_mem_allocator_create(enum piston_mem_allocator_flags flags)
{
	if ((flags & PISTON_MEM_ALLOCATOR_FLAG_BUMP) && (flags & PISTON_MEM_ALLOCATOR_FLAG_REUSE)) {
		PISTON_MEM_PRINTF_RETURN((unsigned)-1, "PISTON_MEM_ALLOCATOR_FLAG_BUMP and PISTON_MEM_ALLOCATOR_FLAG_REUSE are mutually exclusive\n");
	}
}

#endif
