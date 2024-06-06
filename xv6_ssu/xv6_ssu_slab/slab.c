#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"
#include "slab.h"

struct {
	struct spinlock lock;
	struct slab slab[NSLAB];
} stable;

// function of find next power of 2
void slabinit(){
	
	struct slab *s;

	acquire(&stable.lock);
	
	// initializing function
	// set slab size	
	int	slabsize = 16;
	for (s = stable.slab; s < &stable.slab[NSLAB]; s++) {
		s->size = slabsize;
		slabsize *= 2;
	}

	// initialize each slab
	for (s = stable.slab; s < &stable.slab[NSLAB]; s++) {
		s->num_pages = 1;
		s->num_free_objects = PGSIZE/s->size;
		s->num_used_objects = 0;
		s->num_objects_per_page = s->num_free_objects;
		
		// Allocate one page for bitmap by kalloc()
		s->bitmap = kalloc();
		memset(s->bitmap, 0, PGSIZE);

		// Allocate one page by kalloc()
		s->page[0] = kalloc();
		for (int i = 1; i < MAX_PAGES_PER_SLAB; i++) {
			s->page[i] = 0;
		}
	}

	release(&stable.lock);
}

char *kmalloc(int size){

	struct slab *s;

	acquire(&stable.lock);

	// choose slab s.t. fit with required size
	for (s = stable.slab; s < &stable.slab[NSLAB]; s++) {
		if (s->size >= size)
			break;
	}

	// Allocate one more page when free space runs out
	if (s->num_free_objects == 0) {
		if (s->num_pages == 100) {
			release(&stable.lock);
			return 0x00;
		}
		for (int i = 0; i < MAX_PAGES_PER_SLAB; i++) {
			if (s->page[i] == 0) {
				s->page[i] = kalloc();
				memset(s->page[i], 0, PGSIZE);
				s->num_free_objects = s->num_objects_per_page;
				s->num_pages++;
				break;
			}
		}
	}

	s->num_free_objects--;
	s->num_used_objects++; 

	int page_gap = s->num_objects_per_page/8; // {32,16,8,4,2,1,0,0}
	
	for (int i = 0; i < MAX_PAGES_PER_SLAB; i++) {
		if (page_gap != 0) {

			int st_pos = i * page_gap;
			unsigned char mask = 1;

			for (int j = st_pos; j < st_pos + page_gap; j++) {

				if (s->bitmap[j] == -1) {
					continue;
				}

				for (int k = 0; k < 8; k++) {

					if ((s->bitmap[j] & mask) != 0) {
						mask <<= 1;
						continue;
					}

					s->bitmap[j] = (s->bitmap[j]) | mask;

					//cprintf("i = %d, j = %d, k = %d\n", i, j, k);

					char* rtnaddr = s->page[i] + s->size * (k + ((j % page_gap) * 8));

					release(&stable.lock);
					return rtnaddr;			

				}
			}
		} 
		else {
			int page_per_bitmap = 8 / s->num_objects_per_page;
			int bitmap_of_page = i / page_per_bitmap;
			unsigned char mask = 1 << ((i % page_per_bitmap) * s->num_objects_per_page);

			for (int j = 0; j < s->num_objects_per_page; j++) {
				if ((s->bitmap[bitmap_of_page] & mask) != 0) {
					mask <<= 1;
					continue;
				}
				
				s->bitmap[bitmap_of_page] = s->bitmap[bitmap_of_page] | mask;

				char* rtnaddr = s->page[i] + s->size * j;

				release(&stable.lock);
				return rtnaddr;
			}
		}
	}

	release(&stable.lock);
	return 0x00;
}

void kmfree(char *addr, int size){

	struct slab *s;

	acquire(&stable.lock);
	
	for (s = stable.slab; s < &stable.slab[NSLAB]; s++) {
		if (s->size >= size)
			break;
	}
	
	memset(addr, 0, s->size);
	
	//cprintf("addr = %p\n", addr);

	int page_gap = s->num_objects_per_page/8; // {32,16,8,4,2,1,0,0}

	int idx = 0;
	for (idx = 0; idx < MAX_PAGES_PER_SLAB; idx++) {
		//cprintf("s->page[%d] = %p\n", idx, s->page[idx]);
		if ((0 <= (addr - s->page[idx]) && (addr - s->page[idx]) <= PGSIZE)) {
			//cprintf("idx = %d\n");
			break;
		}
	}
	int base = (addr - s->page[idx]) / s->size;

	if ((s->size == 1024) || (s->size == 2048)) {
		int page_per_bitmap = 8 / s->num_objects_per_page; // 1024->2, 2048->4
		int j = base;

		int bitmap_of_page = idx / page_per_bitmap;
		int pos_in_bitmap = idx % page_per_bitmap;

		int mask = 1 << (pos_in_bitmap * s->num_objects_per_page);
		
		s->bitmap[bitmap_of_page] = s->bitmap[bitmap_of_page] & ~(mask << j);
	/*	
		unsigned char mask2 = 0;
		if (s->size == 1024) {
			if (pos_in_bitmap == 0) {
				mask2 = 15;
			}
			else if (pos_in_bitmap == 1) {
				mask2 = -16;
			}
		} else {
			if (pos_in_bitmap == 0) {
				mask2 = 3;
			}
			else if (pos_in_bitmap == 1) {
				mask2 = 12;
			}
			else if (pos_in_bitmap == 2) {
				mask2 = 48;
			}
			else if (pos_in_bitmap == 3) {
				mask2 = -64;
			}
		}

		if ((s->bitmap[bitmap_of_page] | ~mask2) == -1) {
			kfree(s->page[idx]);
			s->page[idx] = 0;
			s->num_pages--;
			s->num_free_objects -= s->num_objects_per_page;
		}
		*/
	}
	else {
		int k = base % 8;
		int j = (base / 8) + (idx * page_gap);

		int mask = 1;
		s->bitmap[j]= s->bitmap[j] & ~(mask << k);

		//cprintf("idx = %d\n", idx);
		//cprintf("bitmap[%d] = %d\n", j, s->bitmap[j]);
		
		int page_gap = s->num_objects_per_page/8;

		int st_pos = idx*page_gap;
		if (idx != 0) {		
			int flag = 0;
			for (int r = st_pos; r < st_pos + page_gap; r++) {
				if (s->bitmap[r] != 0) {
					flag = 1;
					break;
				}
			}
			if (flag == 0) {
				kfree(s->page[idx]);
				s->page[idx] = 0;
				s->num_pages--;
				s->num_free_objects -= s->num_objects_per_page;
			}
		}
	}


	s->num_free_objects++;
	s->num_used_objects--;

	release(&stable.lock);
}

/* Helper functions */
/* Dump function for slab allocator */
void slabdump(){
	cprintf("__slabdump__\n");

	struct slab *s;

	cprintf("size\tnum_pages\tused_objects\tfree_objects\n");

	for(s = stable.slab; s < &stable.slab[NSLAB]; s++){
		cprintf("%d\t%d\t\t%d\t\t%d\n", 
			s->size, s->num_pages, s->num_used_objects, s->num_free_objects);
	}
}

int numobj_slab(int slabid)
{
	return stable.slab[slabid].num_used_objects;
}

int numpage_slab(int slabid)
{
	return stable.slab[slabid].num_pages;
}
