#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mymem.h"
#include <time.h>

/* The main structure for implementing memory allocation.
 * You may change this to fit your implementation.
 */

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
static struct memoryList *last;

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

	if (head != NULL)
	{
		struct memoryList *trav;
		for (trav = head; trav->next != NULL; trav = trav->next)
		{
			free(trav->prev);
		}
		free(trav);
	}
	/* TODO: release any other memory you were using for bookkeeping when doing a re-initialization! */

	printf("Setup memory \n");
	myMemory = malloc(sz);

	/* TODO: Initialize memory management structure. */

	head = (struct memoryList *)malloc(sizeof(struct memoryList));
	head->prev = NULL;
	head->next = NULL;
	head->size = sz;
	head->alloc = 0;
	head->ptr = myMemory;
	currentnode = head;
}

/* Allocate a block of memory with the requested size.
 *  If the requested block is not available, mymalloc returns NULL.
 *  Otherwise, it returns a pointer to the newly allocated block.
 *  Restriction: requested >= 1 
 */

void *mymalloc(size_t requested)
{

	struct memoryList *trav = currentnode;
	struct memoryList *start = currentnode;

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
		printf("alloc \n");
		while (trav->alloc && trav->size <= requested)
		{
			if (trav->next == NULL)
			{
				printf("hej \n");
				trav = head;
			}
			else if (trav->next == start)
			{
				printf("No suitable node \n");
				return NULL;
			}
			else
			{
				trav = trav->next;
			}
		}
		if (trav->size > requested)

		{
			struct memoryList *newnode = (struct memoryList *)malloc(sizeof(struct memoryList));

			// Setting up connection for the new node.

			newnode->prev = trav;
			newnode->next = trav->next;

			newnode->size = trav->size - requested;
			newnode->ptr = trav->ptr + requested;
			newnode->alloc = 0;

			trav->next = newnode;
			trav->size = requested;
		}
		trav->alloc = 1;

		if (trav->next != NULL)
		{
			currentnode = trav->next;
		}
		else
		{
			currentnode = head;
		}
	}
	return trav->ptr;
}

void myfree(void *block)
{
	struct memoryList *trav;
	struct memoryList *temp;
	printf("block to be freed: %p \n", block);
	for (trav = head; trav != NULL; trav = trav->next)
	{
		puts("hej \n");
		if (trav->ptr == block)
		{
			break;
		}
	}

	puts("found block \n");

	if (!trav)
	{
		puts("trav is null");
		return;
	}
	trav->alloc = 0;

	puts("trav");
	if ((trav != head) && (trav->prev->alloc == 0))
	{
		printf("prev \n");
		print_memory();

		trav->prev->size += trav->size;
		temp = trav;

		if (trav->next == NULL)
		{
			trav->prev->next = NULL;
		}
		else
		{
			trav->prev->next = trav->next;
			trav->prev->next->prev = trav->prev;
		}

		// make sure currentnode still works

		if (currentnode == trav)
		{
			currentnode = trav->prev;
		}

		printf("free temp: %p \n", *temp);
		printf("\tBlock %p,\tsize %d,\t%s\n",
			   temp->ptr,
			   temp->size,
			   (temp->alloc ? "[ALLOCATED]" : "[FREE]"));
		trav = trav->prev;
		free(temp);
	}

	if (trav->next != NULL && trav->next != head)
	{
		if (trav->next->alloc == 0)
		{

			struct memoryList *second = trav->next;
			printf("next \n");
			print_memory();

			if (second->next == NULL)
			{
				second->prev->next = NULL;
			}
			else
			{
				trav->next = second->next;

				second->next->prev = trav;
			}
			trav->size += second->size;

			// make sure currentnode still works
			if (currentnode == second)
			{
				currentnode = trav;
			}

			printf("free second: %p", *second);
			printf("\tBlock %p,\tsize %d,\t%s\n",
				   second->ptr,
				   second->size,
				   (second->alloc ? "[ALLOCATED]" : "[FREE]"));
			free(second);
		}
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
	int mem_holes = 0;
	int number_of_counts = 0;
	struct memoryList *trav;
	for (trav = head; trav != NULL; trav = trav->next)
	{
		number_of_counts++;
		if (trav->alloc == 0)
		{
			mem_holes += 1;
		}
		if ((trav->next != NULL) && (trav->next->next == trav))
		{
			break;
		}
	}

	return mem_holes;
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
	int free_size = 0;
	struct memoryList *trav;
	for (trav = head; trav != NULL; trav = trav->next)
	{
		if (trav->alloc == 0)
		{
			free_size += trav->size;
		}

		if ((trav->next != NULL) && (trav->next->next == trav))
		{
			break;
		}
	}
	return free_size;
}

/* Number of bytes in the largest contiguous area of unallocated memory */
int mem_largest_free()
{
	puts("mem_largest_free");
	int max_size = 0;
	struct memoryList *trav;
	for (trav = head; trav != NULL; trav = trav->next)
	{
		puts("alt er kaos");
		printf("pointer: %p \n", trav);
		if (trav->alloc == 0 && trav->size > max_size)
		{
			max_size = trav->size;
		}

		if ((trav->next != NULL) && (trav->next->next == trav))
		{
			break;
		}
	}
	return max_size;
}

/* Number of free blocks smaller than "size" bytes. */
int mem_small_free(int size)
{
	puts("mem_smallest_free");
	int count = 0;

	struct memoryList *trav;
	for (trav = head; trav != NULL; trav = trav->next)
	{
		if (trav->size <= size && trav->alloc == 0)
		{
			count += 1;
		}
		if ((trav->next != NULL) && (trav->next->next == trav))
		{
			break;
		}
	}

	return count;
}

char mem_is_alloc(void *ptr)
{
	puts("mem_is_alloc");
	struct memoryList *trav;
	for (trav = head; trav != NULL; trav = trav->next)
	{
		if (trav->ptr == ptr)
		{

			return trav->alloc;
		}
	}
	return 0;
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
	for (index = head; index != NULL; index = index->next)
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
	myfree(b);
	d = mymalloc(50);
	myfree(a);
	e = mymalloc(25);

	print_memory();
	print_memory_status();
}
