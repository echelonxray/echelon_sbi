#include "memalloc.h"
#include "./../inc/types.h"

struct alloc_section {
	size_t size;
	void* next;
};

void* mem_block_start = 0;
void* mem_block_end = 0;

void* kmalloc(size_t size) {
	if (size % sizeof(uintRL_t)) {
		size += sizeof(uintRL_t) - (size % sizeof(uintRL_t));
	}
	// If allocation size is 0 or if the final size will overflow size_t, return 0
	if (size == 0 || (size + sizeof(struct alloc_section)) < size) {
		return 0;
	}
	size += sizeof(struct alloc_section);
	struct alloc_section* section = mem_block_start;
	while ((uintRL_t)section < (uintRL_t)mem_block_end) {
		// Check if we will overflow type size_t?
		if ((uintRL_t)section + size < (uintRL_t)section) {
			// Fail: We overflowed, the value of size is too large.
			return 0;
		}
		// Check if we will overflow type size_t?
		if ((uintRL_t)section + section->size + size < (uintRL_t)section) {
			// Fail: We overflowed, the value of size is too large.
			return 0;
		}
		// Have we iterated to the last previously allocated section?
		if (section->next == 0) {
			// Will the contiguous allocation fit inside remaining available contiguous space?
			if ((uintRL_t)section + section->size + size <= (uintRL_t)mem_block_end) {
				// Allocate Section
				section->next = (void*)section + section->size;
				section = section->next;
				section->size = size;
				section->next = 0;
				return (void*)section + sizeof(struct alloc_section);
			} else {
				// Fail: Not enough space to allocate
				return 0;
			}
		} else {
			// Will the contiguous allocation fit inside remaining available contiguous space?
			if ((uintRL_t)section + section->size + size <= (uintRL_t)section->next) {
				// Allocate Section
				register void* tmp_next = section->next;
				section->next = (void*)section + section->size;
				section = section->next;
				section->size = size;
				section->next = tmp_next;
				return (void*)section + sizeof(struct alloc_section);
			}
		}
		// Go onto the next section
		section = section->next;
	}
	// Should not be reachable
	return 0;
}

void kfree(void* ptr) {
	ptr -= sizeof(struct alloc_section);
	if ((uintRL_t)ptr < (uintRL_t)mem_block_start || (uintRL_t)ptr >= (uintRL_t)mem_block_end) {
		return;
	}
	struct alloc_section* section = mem_block_start;
	struct alloc_section* prev_section = 0;
	while ((uintRL_t)section < (uintRL_t)mem_block_end) {
		if (section == ptr) {
			if (section->size == 0) {
				return;
			}
			section->size = 0;
			if (prev_section != 0) {
				prev_section->next = section->next;
			}
			return;
		}
		if (section->next == 0) {
			return;
		}
		prev_section = section;
		section = section->next;
	}
	return;
}

void kallocinit(void* block_start, void* block_end) {
	if (block_start > block_end) {
		return;
	}
	if (((uintRL_t)block_end - (uintRL_t)block_start) < sizeof(struct alloc_section)) {
		return;
	}
	mem_block_start = block_start;
	mem_block_end = block_end;
	struct alloc_section* section = block_start;
	section->size = 0;
	section->next = 0;
	return;
}
