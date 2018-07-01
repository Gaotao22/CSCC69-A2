#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"



extern int memsize;

extern int debug;

extern struct frame *coremap;

extern char *tracefile;

addr_t* addList;

int count;

FILE* tfile;

/* Page to evict is chosen using the optimal (aka MIN) algorithm. 
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {
	
	return 0;
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {
	
	return;
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
//initialize the frames and get the virtual addresses that will 
//be used to form the frame. It is helpful for prediction
void opt_init() {
	char type;
	addr_t vaddr;
	char buf1[256];
	char buf2[256];
	if(tracefile == NULL){
		perror("tracefile not found");
		exit(1);
	}
	if((tfile = fopen(tracefile, "r")) == NULL){
		perror("Error: Cannot open tracefile:");
		exit(1);
	}else{
		
		addr_t vaddr = 0;
		char type;
		//read the virtual addresses in the trace file
		while(fgets(buf1, 256, tfile) != NULL) {
			if(buf1[0] != '=') {
				count ++;
			} else {
			continue;
			}

		}
	}
	addList = malloc(count);
	if (count >= 0){
		int i = 0;
		while(fgets(buf2, 256, tfile) != NULL) {
			if(buf2[0] != '=') {
				sscanf(buf2, "%c %lx", &type, &vaddr);
				addList[i] = vaddr;
				i ++;
			} else {
			continue;
			}

		}
	}
	
	//put all page entry pointer to null
	int i;
	for (i = 0; i < memsize; i++){
		coremap[i].in_use = 0;
		coremap[i].pte = NULL;
	}

}

