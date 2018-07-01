#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

int framestack[memsize];

/* This function shift the content behind the selected index
 * To the index right before it
 * Return the last available index
 */
int shift_index(int index){
	int i = index + 1;
	while(i < memsize){
		framestack[i - 1] = framestack[i];
		if (framestack[i] == NULL){
			break;
		}
		if (i == memsize - 1){
			framestack[i] = NULL;
		}
		i ++;
	}
	return i - 1;
}

int find_frame(int current_frame){
	int i = 0;
	while (framestack[i] != NULL && framestack[i] != current_frame && i < memsize){
		i++;
	}
	return i;
}

/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int lru_evict() {
	int ret_f = framestack[0];
	shift_index(0);
	return ret_f;
	
}



/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {
	int current_frame = p->frame >>  PAGE_SHIFT;
	//find the index of the current frame
	int index = find_frame(current_frame);
	//shift the index and get the last available index
	int last_index = shift_index(index);
	framestack[last_index] = current_frame;
	return;
}


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
	//put all page entry pointer to null
	int i;
	for (i = 0; i < memsize; i++){
		framestack[i] = NULL;
	}

}
