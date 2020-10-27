#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mymem.h"
#include <time.h>

/* The main structure for implementing memory allocation.
 * You may change this to fit your implementation.
 */

struct memoryList *find_block(size_t requested);

struct memoryList
{
	// doubly-linked list
	struct memoryList *prev;
	struct memoryList *next;

	int size;	// How many bytes in this block?
	char alloc; // 1 if this block is allocated,
				// 0 if this block is free.
	void *ptr;	// location of block in memory pool.
};

strategies myStrategy = NotSet; // Current strategy

size_t mySize;
void *myMemory = NULL;

static struct memoryList *head;
static struct memoryList *currentnode;

/* initmem must be called prior to mymalloc and myfree.

   initmem may be called more than once in a given exeuction;
   when this occurs, all memory you previously malloc'ed  *must* be freed,
   including any existing bookkeeping data.

   strategy must be one of the following:
		- "best" (best-fit)
		- "worst" (worst-fit)
		- "first" (first-fit)
		- "next" (next-fit)
   sz specifies the number of bytes that will be available, in total, for all mymalloc requests.
*/

void initmem(strategies strategy, size_t sz)
{
	myStrategy = strategy;

	/* all implementations will need an actual block of memory to use */
	mySize = sz;

	if (myMemory != NULL)
		free(myMemory); /* in case this is not the first time initmem2 is called */

	/*if (head != NULL)
	{
		puts("something here?");
		struct memoryList *trav;
		for (trav = head; trav->next != head; trav = trav->next)
		{
			free(trav->prev);
		}
		free(trav);
	} */

	if (head)
		free(head);
	/* TODO: release any other memory you were using for bookkeeping when doing a re-initialization! */

	printf("Setup memory \n");
	myMemory = malloc(sz);

	/* TODO: Initialize memory management structure. */

	head = (struct memoryList *)malloc(sizeof(struct memoryList));
	head->size = sz;
	head->alloc = 0;
	head->ptr = myMemory;
	currentnode = head;

	head->prev = head;
	head->next = head;
}

/* Allocate a block of memory with the requested size.
 *  If the requested block is not available, mymalloc returns NULL.
 *  Otherwise, it returns a pointer to the newly allocated block.
 *  Restriction: requested >= 1 
 */

void *mymalloc(size_t requested)
{

	struct memoryList *start = currentnode;
	struct memoryList *matching_block = NULL;

	assert((int)myStrategy > 0);

	switch (myStrategy)
	{
	case NotSet:
		return NULL;
	case First:
		return NULL;
	case Best:
		return NULL;
	case Worst:
		return NULL;
	case Next:
		matching_block = find_block(requested);
	}

	if (!matching_block)
	{
		fprintf(stderr, "No suitable block");
		return NULL;
	}
	printf("alloc \n");

	if (matching_block->size > requested)
	{
		struct memoryList *newnode = malloc(sizeof(struct memoryList));

		// Setting up connection for the new node.

		newnode->prev = matching_block;
		newnode->next = matching_block->next;
		newnode->next->prev = newnode;
		matching_block->next = newnode;

		newnode->size = matching_block->size - requested;
		newnode->ptr = matching_block->ptr + requested;
		newnode->alloc = 0;

		currentnode = newnode;
		matching_block->size = requested;
	}
	else
	{
		currentnode = matching_block->next;
	}
	matching_block->alloc = 1;

	return matching_block->ptr;
}

struct memoryList *find_block(size_t requested)
{
	struct memoryList *start = currentnode;
	do
	{
		if ((currentnode->alloc == 0) && currentnode->size >= requested)
		{
			return currentnode;
		}

	} while ((currentnode = currentnode->next) != start);

	return NULL;
}
void myfree(void *block)
{
	struct memoryList *trav = head;
	struct memoryList *temp;
	for (trav = head; trav->next != head; trav = trav->next)
	{
		if (trav->ptr == block)
		{
			break;
		}
	}

	trav->alloc = 0;

	if ((trav != head) && !(trav->prev->alloc))
	{
		struct memoryList *prev = trav->prev;

		prev->size += trav->size;
		prev->next = trav->next;
		prev->next->prev = prev;

		// make sure currentnode still works

		if (currentnode == trav)
		{
			currentnode = prev;
		}

		free(trav);
		trav = prev;
	}

	if (trav->next != head && !(trav->next->alloc))
	{

		struct memoryList *second = trav->next;

		trav->next = second->next;
		second->next->prev = trav;
		trav->size += second->size;

		// make sure currentnode still works
		if (currentnode == second)
		{
			currentnode = trav;
		}

		free(second);
	}
}

/****** Memory status/property functions ******
 * Implement these functions.
 * Note that when refered to "memory" here, it is meant that the 
 * memory pool this module manages via initmem/mymalloc/myfree. 
 */

/* Get the number of contiguous areas of free space in memory. */
int mem_holes()
{
	puts("mem_holes");
	return mem_small_free(mySize + 1);
}

/* Get the number of bytes allocated */
int mem_allocated()
{
	puts("mem_allocated");
	return mySize - mem_free();
}

/* Number of non-allocated bytes */
int mem_free()
{
	puts("mem_free");

	int count = 0;

	/* Iterate over memory list */
	struct memoryList *index = head;
	do
	{
		/* Add to total only if this block isn't allocated */
		if (!(index->alloc))
		{
			puts("test index");
			count += index->size;
		}
	} while ((index = index->next) != head);

	return count;
	/*int free_size = 0;
	struct memoryList *trav;
	for (trav = head; trav->next != head; trav = trav->next)
	{
		if (trav->alloc == 0)
		{
			free_size += trav->size;
		}
	}
	return free_size; */
}

/* Number of bytes in the largest contiguous area of unallocated memory */
int mem_largest_free()
{
	puts("mem_largest_free");

	int max_size = 0;

	/* Iterate over memory list */
	struct memoryList *index = head;
	do
	{
		if (!(index->alloc) && index->size > max_size)
		{
			puts("max_test");
			max_size = index->size;
		}
	} while ((index = index->next) != head);

	return max_size;
	/*int max_size = 0;
	struct memoryList *trav;
	for (trav = head; trav->next != head; trav = trav->next)
	{
		puts("alt er kaos");
		printf("pointer: %p \n", trav);
		if (trav->alloc == 0 && trav->size > max_size)
		{
			max_size = trav->size;
		}
	}
	return max_size; */
}

/* Number of free blocks smaller than "size" bytes. */
int mem_small_free(int size)
{
	puts("mem_smallest_free");
	int count = 0;

	struct memoryList *trav = head;

	do
	{
		if (trav->size <= size)
		{
			puts("test");
			count += !(trav->alloc);
		}
	} while ((trav = trav->next) != head);

	return count;
}

char mem_is_alloc(void *ptr)
{
	puts("mem_is_alloc");

	/* Iterate over the memory list */
	struct memoryList *index = head;
	while (index->next != head)
	{
		/* If the next block's ptr is after the target,
       the target must be in this block */
		if (ptr < index->next->ptr)
		{
			return index->alloc;
		}
		index = index->next;
	}

	/* Iterator is now at the last block, so we assume the target is here */
	return index->alloc;
	/*struct memoryList *trav;
	for (trav = head; trav->next != head; trav = trav->next)
	{
		if (trav->next->ptr > ptr)
		{

			return trav->alloc;
		}
	}
	return trav->alloc; */
}

/* 
 * Feel free to use these functions, but do not modify them.  
 * The test code uses them, but you may find them useful.
 */

//Returns a pointer to the memory pool.
void *mem_pool()
{
	return myMemory;
}

// Returns the total number of bytes in the memory pool. */
int mem_total()
{
	return mySize;
}

// Get string name for a strategy.
char *strategy_name(strategies strategy)
{
	switch (strategy)
	{
	case Best:
		return "best";
	case Worst:
		return "worst";
	case First:
		return "first";
	case Next:
		return "next";
	default:
		return "unknown";
	}
}

// Get strategy from name.
strategies strategyFromString(char *strategy)
{
	if (!strcmp(strategy, "best"))
	{
		return Best;
	}
	else if (!strcmp(strategy, "worst"))
	{
		return Worst;
	}
	else if (!strcmp(strategy, "first"))
	{
		return First;
	}
	else if (!strcmp(strategy, "next"))
	{
		return Next;
	}
	else
	{
		return 0;
	}
}

/* 
 * These functions are for you to modify however you see fit.  These will not
 * be used in tests, but you may find them useful for debugging.
 */

/* Use this function to print out the current contents of memory. */
void print_memory()
{
	/* Iterate over memory list */
	struct memoryList *index;
	for (index = head; index->next != head; index = index->next)
	{
		printf("\tBlock %p,\tsize %d,\t%s\n",
			   index->ptr,
			   index->size,
			   (index->alloc ? "[ALLOCATED]" : "[FREE]"));
	}

	printf("}\n");
}

/* Use this function to track memory allocation performance.  
 * This function does not depend on your implementation, 
 * but on the functions you wrote above.
 */
void print_memory_status()
{
	printf("%d out of %d bytes allocated.\n", mem_allocated(), mem_total());
	printf("%d bytes are free in %d holes; maximum allocatable block is %d bytes.\n", mem_free(), mem_holes(), mem_largest_free());
	printf("Average hole size is %f.\n\n", ((float)mem_free()) / mem_holes());
}

/* Use this function to see what happens when your malloc and free
 * implementations are called.  Run "mem -try <args>" to call this function.
 * We have given you a simple example to start.
 */
void try_mymem(int argc, char **argv)
{
	strategies strat;
	void *a, *b, *c, *d, *e;
	if (argc > 1)
		strat = strategyFromString(argv[1]);
	else
		strat = First;

	/* A simple example.  
	   Each algorithm should produce a different layout. */

	initmem(strat, 500);

	a = mymalloc(100);
	b = mymalloc(100);
	c = mymalloc(100);
	d = mymalloc(100);
	e = mymalloc(100);
	myfree(b);
	myfree(d);
	myfree(c);
	myfree(e);

	print_memory();
	print_memory_status();
}