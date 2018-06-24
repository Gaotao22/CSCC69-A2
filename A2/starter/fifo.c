#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

/* Page to evict is chosen using the fifo algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int fifo_evict() {
	int i;
	int ret_f;
	coremap[i].in_use = 0;
	coremap[i].pte = NULL;
	for (i = 0; i < memsize; i++){
		if (coremap[i + 1].pte == NULL){
			coremap[i].in_use = 0;
			coremap[i].pte = NULL;
			break;
		}
		coremap[i] = coremap[i + 1];		
	}
	ret_f = i;
	return ret_f;
}

/* This function is called on each access to a page to update any information
 * needed by the fifo algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void fifo_ref(pgtbl_entry_t *p) {
	int i;
	for (i = 0; i < memsize; i++){
		if (coremap[i].pte == NULL){
			coremap[i].in_use = 1;
			coremap[i].pte = p;
			
			break;
		}
	}
	return;
}

/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void fifo_init() {
	//coremap = malloc(memsize * sizeof(struct frame));
	//put all page entry pointer to null
	int i;
	for (i = 0; i < memsize; i++){
		coremap[i].in_use = 0;
		coremap[i].pte = NULL;
		coremap[i].pin = 0;
	}
	coremap[0].pin = 1;
}
