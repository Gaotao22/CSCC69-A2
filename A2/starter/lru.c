#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int lru_evict() {
	int i =0;
	int ret_f;
	coremap[0].in_use = 0;
	coremap[0].pte = NULL;
	for (i = 0; i < memsize; i++){
		if (coremap[i + 1].pte == NULL){
			coremap[i].in_use = 0;
			coremap[i].pte = NULL;
			break;
		}
		coremap[i] = coremap[i + 1];		
	}
	i = 0;
	return ret_p->frame;
}

/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {
	int i;
	for (i = 0; i < memsize; i++){
		if (coremap[i].pte == p){
			int j;
			for (j = i; j < memsize; j++){
				if (coremap[j + 1].pte == NULL){
					coremap[j].pte = p;
					coremap[j].in_use = 1;
					break;
				}
				coremap[j] = coremap[j + 1];
			}
		}
		coremap[i - 1] = coremap[i];		
	}
	return;
}


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
	//put all page entry pointer to null
	int i;
	for (i = 0; i < memsize; i++){
		coremap[i].in_use = 0;
		coremap[i].pte = NULL;
		coremap[i].pin = 0;
	}
	coremap[0].pin = 1;
}
