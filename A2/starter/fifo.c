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

int pin; // define the variable to label the frame with the oldest page 

int fifo_evict() {
	int ret_frame;
	ret_frame = pin;
	pin ++;
	// reset the pin to 0 if it reaches the capacity of the physical memory
	if (pin >= memsize){
		pin = 0;
	}
	return ret_frame;
}

/* This function is called on each access to a page to update any information
 * needed by the fifo algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void fifo_ref(pgtbl_entry_t *p) {
	return;
}

/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void fifo_init() {
	//put the pin on where the oldest page is.
	pin = 0;
}
