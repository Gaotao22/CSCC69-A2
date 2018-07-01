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

int pin; // Define the variable that tracks the next frame

int clock_evict() {
	int i;
	int ret_f;
	// check the reference number of each element from where the process ended the last time
	for (i = pin; i < memsize; i++){
		// if the reference number is 1, set it to 0
		if (coremap[i].pte->frame & PG_REF){
			coremap[i].pte->frame &= ~PG_REF;
		// if the reference number is 0, then evict the frame
		}else{
			ret_f = i;
			break;
		}
	}
	// set the starting frame of the new round to the next frame
	pin = ret_f + 1;
	// reset the next frame to 0 if we reached the end of the physical memory list
	if (pin >= memsize){
		pin = 0;
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
	// Start to iterate the physical mem table from the beginning
	pin = 0;
}
