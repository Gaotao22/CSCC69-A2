#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

/* Page to evict is chosen using the clock algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int clock_evict() {
	int i;
	int j;
	int ret_f;
	for (j = 0; j < memsize; j++){
		if (coremap[j].pin == 1){
			coremap[j].pin = 0;
			break;
		}
	}
		
	for (i = j; i < memsize; i++){
		if (coremap[i].pte->frame & PG_REF){
			coremap[i].pte->frame &= ~PG_REF;
		}else{
			ret_f = i;
			coremap[i].pte -> in_use = 0;
			coremap[i + 1].pin = 1;
			break;
		}
	}
	return ret_f;
	
}

/* This function is called on each access to a page to update any information
 * needed by the clock algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void clock_ref(pgtbl_entry_t *p) {
	
	return;
}

/* Initialize any data structures needed for this replacement
 * algorithm. 
 */
void clock_init() {
	coremap = malloc(memsize * sizeof(struct frame));
	//put all page entry pointer to null
	int i;
	for (i = 0; i < memsize; i++){
		coremap[i].in_use = 0;
		coremap[i].pte = NULL;
		coremap[i].pin = 0;
	}
	coremap[0].pin = 1;
}
