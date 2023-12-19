#include "memalloc.h"
#include <string.h>
#include <inc/types.h>

#define PAGE_SIZE (0x1000ul)
#define PAGE_ENTRY_SIZE sizeof(uintRL_t)
#define PAGEARRAY_SIZE (PAGE_SIZE * (PAGE_SIZE / PAGE_ENTRY_SIZE))

struct alloc_section {
	size_t size;
	void* next;
};

struct alloc_pagearray_section {
	uintRL_t used_pages[PAGE_SIZE / PAGE_ENTRY_SIZE];
};

void* mem_block_start;
void* mem_block_end;

void* mem_page_start;
void* mem_page_end;
//uintRL_t mem_pagearray_offset;
uintRL_t mem_page_num_allocated;
uintRL_t mem_page_next_free;
uintRL_t mem_page_length;
uintRL_t mem_page_max_length;

void kalloc_pageinit(void* block_start, void* block_end) {
	mem_page_next_free = 0;
	uintRL_t tmp_block_start = (uintRL_t)block_start;
	if (((uintRL_t)block_start) % PAGE_SIZE) {
		tmp_block_start += PAGE_SIZE - (((uintRL_t)block_start) % PAGE_SIZE);
	}
	tmp_block_start += PAGE_SIZE;
	if (tmp_block_start < ((uintRL_t)block_start)) {
		// Datatype overflow/Wrapping occurred
		return;
	}
	block_end = (void*)(((uintRL_t)block_end) & (~(((uintRL_t)PAGE_SIZE) - 1)));
	if (tmp_block_start >= ((uintRL_t)block_end)) {
		/// Too small of space
		return;
	}
	block_start = (void*)tmp_block_start;
	mem_page_start = block_start;
	mem_page_end = block_end;
	//mem_pagearray_offset = mem_page_start - ((~(PAGEARRAY_SIZE - 1)) & mem_page_start);
	//mem_pagearray_offset = ((uintRL_t)mem_page_start) & (PAGEARRAY_SIZE - 1);
	struct alloc_pagearray_section* section = block_start;
	mem_page_num_allocated = 0;
	mem_page_next_free = 1;
	mem_page_length = 1;
	mem_page_max_length = (tmp_block_start - ((uintRL_t)block_end)) / PAGE_SIZE;
	section->used_pages[0] = 1;
	for (uintRL_t i = 1; i < (PAGE_SIZE / PAGE_ENTRY_SIZE); i++) {
		section->used_pages[i] = 0;
	}
	return;
}

void* kalloc_page() {
	if (mem_page_next_free == 0) {
		return 0;
	}
	void* page_location = mem_page_start + (mem_page_next_free * PAGE_SIZE);
	uintRL_t pagearray_index = mem_page_next_free / (PAGE_SIZE / PAGE_ENTRY_SIZE);
	uintRL_t page_index = mem_page_next_free % (PAGE_SIZE / PAGE_ENTRY_SIZE);
	struct alloc_pagearray_section* section = (struct alloc_pagearray_section*)mem_page_start;
	uintRL_t page_entry = section[pagearray_index].used_pages[page_index];
	section[pagearray_index].used_pages[page_index] = 1;
	if (page_entry == 0) {
		if (mem_page_length == mem_page_max_length) {
			section[pagearray_index].used_pages[page_index] = 0;
			return 0;
		}
		mem_page_length++;
		mem_page_next_free++;
		section[pagearray_index].used_pages[0]++;
		if ((mem_page_next_free % (PAGE_SIZE / PAGE_ENTRY_SIZE)) == 0) {
			section[pagearray_index].used_pages[0] = 0;
			if (mem_page_length == mem_page_max_length) {
				mem_page_num_allocated++;
				mem_page_next_free = 0;
				memset(page_location, 0, PAGE_SIZE);
				return page_location;
			}
			mem_page_length++;
			mem_page_next_free++;
			pagearray_index++;
			section[pagearray_index].used_pages[0] = 1;
			for (uintRL_t i = 1; i < (PAGE_SIZE / PAGE_ENTRY_SIZE); i++) {
				section[pagearray_index].used_pages[i] = 0;
			}
		}
	} else {
		mem_page_next_free = page_entry;
		if ((page_entry / (PAGE_SIZE / PAGE_ENTRY_SIZE)) == pagearray_index) {
			section[pagearray_index].used_pages[0] = page_entry % (PAGE_SIZE / PAGE_ENTRY_SIZE);
		} else {
			section[pagearray_index].used_pages[0] = 0;
		}
	}
	mem_page_num_allocated++;
	memset(page_location, 0, PAGE_SIZE);
	return page_location;
}

void kfree_page(void* ptr) {
	if ((ptr == 0) || ((uintRL_t)ptr % PAGE_SIZE) || (((uintRL_t)ptr) <= ((uintRL_t)mem_page_start))) {
		return;
	}
	uintRL_t page_index_to_free = (((uintRL_t)ptr) - ((uintRL_t)mem_page_start)) / PAGE_SIZE;
	if (page_index_to_free == 0) {
		return;
	}
	uintRL_t pagearray_index = page_index_to_free / (PAGE_SIZE / PAGE_ENTRY_SIZE);
	uintRL_t page_index = page_index_to_free % (PAGE_SIZE / PAGE_ENTRY_SIZE);
	struct alloc_pagearray_section* section = (struct alloc_pagearray_section*)mem_page_start;
	if (page_index_to_free < mem_page_next_free) {
		section[pagearray_index].used_pages[page_index] = mem_page_next_free;
		section[pagearray_index].used_pages[0] = page_index;
		mem_page_next_free = page_index_to_free;
		mem_page_num_allocated--;
	} else {
		if (section[pagearray_index].used_pages[0] != 0 && section[pagearray_index].used_pages[0] < page_index) {
			uintRL_t final_page_index = section[pagearray_index].used_pages[0];
			for (uintRL_t i = final_page_index + 1; i < page_index; i++) {
				if (section[pagearray_index].used_pages[i] != 1) {
					final_page_index = i;
				}
			}
			section[pagearray_index].used_pages[page_index] = section[pagearray_index].used_pages[final_page_index];
			section[pagearray_index].used_pages[final_page_index] = page_index_to_free;
			mem_page_num_allocated--;
		} else {
			for (sintRL_t i = ((sintRL_t)pagearray_index) - 1; i >= 0; i--) {
				if (section[i].used_pages[0] != 0) {
					uintRL_t final_page_index = section[i].used_pages[0];
					for (uintRL_t j = final_page_index + 1; j < (PAGE_SIZE / PAGE_ENTRY_SIZE); j++) {
						if (section[i].used_pages[j] != 1) {
							final_page_index = j;
						}
					}
					section[pagearray_index].used_pages[page_index] = section[i].used_pages[final_page_index];
					section[pagearray_index].used_pages[0] = page_index;
					section[i].used_pages[final_page_index] = page_index_to_free;
					mem_page_num_allocated--;
					break;
				}
			}
		}
	}
	return;
}

void* kmalloc_stack(size_t size) {
	size_t over_alloc_size = size + 0x20;
	uintRL_t allocation = (uintRL_t)kmalloc(over_alloc_size);
	if (allocation == 0) {
		return 0;
	}
	uintRL_t* stack_pointer = (uintRL_t*)((allocation + size + 0x10) & (~((uintRL_t)0xF)));
	*stack_pointer = allocation;
	return ((void*)(stack_pointer)) - 0x10;
}

void kfree_stack(void* ptr) {
	if (ptr == 0) {
		return;
	}
	uintRL_t* stack_pointer = (uintRL_t*)(ptr + 0x10);
	kfree((void*)(*stack_pointer));
	return;
}

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
