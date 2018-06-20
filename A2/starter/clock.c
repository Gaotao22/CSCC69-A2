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
	pgtbl_entry_t *ret_p;
	for (j = 0; j < memsize; j++){
		if (coremap[j].pte->frame & PG_REF){
			coremap[j].pte->frame &= ~PG_REF;
		}else{
			ret_p = coremap[j].pte;
			break;
		}
	}
	for (i = j; i < memsize; i++){
		if (coremap[i + 1].pte == NULL){
			coremap[i].in_use = 0;
			coremap[i].pte = NULL;
			break;
		}
		coremap[i] = coremap[i + 1];		
	}
	return ret_p->frame;
	
}

/* This function is called on each access to a page to update any information
 * needed by the clock algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void clock_ref(pgtbl_entry_t *p) {
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

/* Initialize any data structures needed for this replacement
 * algorithm. 
 */
void clock_init() {
	coremap = malloc(memsize * sizeof(frame));
	//put all page entry pointer to null
	int i;
	for (i = 0; i < memsize; i++){
		coremap[i].in_use = 0;
		coremap[i].pte = NULL;
	}
}
