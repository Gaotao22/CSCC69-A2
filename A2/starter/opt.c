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

/* typedefs for storing addrs stuff */
typedef struct Number number;
typedef struct Number {
        int num_ref;
        number *next;
        number *tail;
} number;

typedef struct VaddrTracker {
        addr_t vaddr;
        number *num_fut_ref;
} vaddr_tracker;

typedef struct LinkedList linked_list;
typedef struct LinkedList {
	vaddr_tracker *item;
	linked_list *next;
} linked_list;

linked_list **tracker;
/* end of typedefs */

addr_t* addList;

int count;
int frame_num;

FILE* tfile;

/* Hashmap stuff */
unsigned int bucket_size;

unsigned int get_hash(addr_t vaddr) {
	unsigned long hashing = (long) (vaddr);

	// pray to god this is actually a somewhat decent hash
	// I have no idea if it is
	printf("hash: bitshifting\n");
	hashing = ((hashing << 5) ^ (hashing >> 10)) % (hashing == 0 ? 1 : hashing);
	printf("%d-> %ld\n", (int)vaddr, hashing);
	return hashing == 0 ? hashing : (unsigned int)(hashing) % bucket_size;
}
/* end of hashmap stuff */

/* vaddr tracking funcs */
linked_list *search_vaddr(addr_t vaddr) {
	printf("search: hashing vaddr\n");
	int hashed = get_hash(vaddr);
	printf("search: hashed vaddr\n");
	linked_list *ll = tracker[hashed];
	printf("search: searching %d\n", (int) vaddr);
	if(ll == NULL) {
		printf("ll is null, about to crash\n");
	}
	vaddr_tracker *curr = ll->item;
	printf("search: item vaddr %d\n", (int) curr->vaddr);

	while(curr != NULL && curr->vaddr != vaddr && ll->next != NULL) {
		ll = ll->next;
		curr = ll->item;
	}

	if(curr == NULL || curr->vaddr != vaddr) {
		fprintf(stderr, "Cannot find vaddr entry\n");
		return NULL;
	}

	return ll;
}

int init_ll(linked_list **ll) {
	*ll = (linked_list *)malloc(sizeof(linked_list));
	if(ll == NULL) {
		perror("malloc");
		return -1;
	}
	return 0;
}

int init_vaddr(addr_t vaddr, linked_list *ll) {
	ll->item = (vaddr_tracker *)malloc(sizeof(vaddr_tracker));
	if(ll->item == NULL) {
		perror("malloc");
		return -1;
	}

	ll->item->vaddr = vaddr;
	return 0;
}

int init_number(number **num) {
	*num = (number *)malloc(sizeof(number));
	if(num == NULL) {
		perror("malloc");
		return -1;
	}
	return 0;
}

int add_vaddr(linked_list **tracker, addr_t vaddr, int count) {
	vaddr_tracker *curr;
	number *next_num;
	int status;

	// let's find the right vaddr linked list element
	int hashed = get_hash(vaddr);
	linked_list *ll = tracker[hashed];

	if(ll == NULL) { // if no entry in linked list
		if((status = init_ll(&ll)) != 0) {
			return status;
		}

		if((status = init_vaddr(vaddr, ll)) != 0) {
			return status;
		}
		tracker[hashed] = ll;
	}

	while(ll->item->vaddr != vaddr && ll->next != NULL) {
		ll = ll->next;
	}

	if(ll->item->vaddr != vaddr) { // if entries exist but no vaddr entry
		if((status = init_ll(&(ll->next))) != 0) {
			return status;
		}

		if((status = init_vaddr(vaddr, ll->next)) != 0) {
			return status;
		}
		ll = ll->next;
	}

	// now we create a new entry for that vaddr
	curr = ll->item;

	if((status = init_number(&next_num)) != 0) {
		return status;
	}

	next_num->num_ref = count;

	// if no entries exist for the num of frame counts yet
	if(curr->num_fut_ref == NULL) {
		curr->num_fut_ref = next_num;
		curr->num_fut_ref->tail = next_num;
	} else {
		curr->num_fut_ref->tail->next = next_num;
	}

	curr->num_fut_ref->tail = next_num;

	return 0;
}

int next_num(linked_list *ll) {
	int num = ll->item->num_fut_ref->num_ref;
	number *remove = ll->item->num_fut_ref;
	ll->item->num_fut_ref = ll->item->num_fut_ref->next;
	free(remove);
	return num;
}
/* end of vaddr tracking funcs */

/* Page to evict is chosen using the optimal (aka MIN) algorithm. 
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {
	printf("Evict: starting\n");
	int evict = 0;
	int i;
        for(i = 0; i < memsize; i++) {
                if(coremap[i].num_to_ref < coremap[evict].num_to_ref) {
                        evict = i;
                }
        }

    printf("Evict: Evicting frame %d\n", evict);
    printf("Evict: opt completed\n\n\n");
	return evict;
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {
	printf("ref: frame %d:\n", p->frame >> PAGE_SHIFT);
	int frame_i = p->frame >> PAGE_SHIFT;
	struct frame *f = &coremap[frame_i];
	addr_t vaddr = f->vaddr;
	printf("\tin use %c, vaddr %d, num_to_ref %d\n", f->in_use, (int)f->vaddr, f->num_to_ref);

	printf("ref: %d has num_to_ref %d on frame %d\n", (int) vaddr, f->num_to_ref, frame_num);
	linked_list *ll = search_vaddr(vaddr);
	if(ll == NULL) {
		exit(1);
	}

	int num = next_num(ll);
	f->num_to_ref = num;

	frame_num++;

	printf("ref: ref ended\n\n\n");
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
	bucket_size = 0;

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
	addList = malloc(count * sizeof(addr_t));
	if (count >= 0){
		rewind(tfile);
		bucket_size = count > 100000 ? count / 100 : count; // random guess of a good bucket size tbh
		printf("Init: start, count: %d\n", count);
		tracker = (linked_list **)malloc(sizeof(linked_list *) * bucket_size);
		printf("Init: Bucket initialized, %d\n",bucket_size);
		int i = 0;
		int ref = 0;
		number *debug;
		while(fgets(buf2, 256, tfile) != NULL) {
			if(buf2[0] != '=') {
				printf("Init: reading line %d\n", i);
				sscanf(buf2, "%c %lx", &type, &vaddr);
				printf("Init: alloc addr %d\n", i);
				addList[i] = vaddr;

				// vaddr tracker stuff
				printf("vaddr: %d\n", (int) vaddr);
				if(add_vaddr(tracker, vaddr, i) == -1) {
					exit(-1);
				}

				i++;
				debug = tracker[get_hash(vaddr)]->item->num_fut_ref;
				while(debug->next != NULL) { debug = debug->next; }
				printf("Init: num_to_ref %d for vaddr %d\n", debug->num_ref, (int)vaddr);
				printf("Init: round %d of %d complete\n\n\n", i - 1, count);
			} else {
			continue;
			}

		}
		printf("Init: complete\n");
		fclose(tfile);
		printf("Continue?\n");
		scanf("c", NULL, NULL);
		printf("\n\n\n\n");
	}

	frame_num = 0;
	
	
	//put all page entry pointer to null
	for (int i = 0; i < memsize; i++){
		coremap[i].in_use = 0;
		coremap[i].pte = NULL;
	}

}

