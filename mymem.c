#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mymem.h"
#include <time.h>

/* The main structure for implementing memory allocation.
 * You may change this to fit your implementation.
 */

//initialize prototype functions
struct memoryList *find_block_next(size_t requested);
struct memoryList *find_block_worst(size_t requested);
struct memoryList *find_block_first(size_t requested);
struct memoryList *find_block_best(size_t requested);
void *free_adjacent(struct memoryList *trav);
void insertBlock(struct memoryList *block, size_t requested);

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
static struct memoryList *largestFree;

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

	// clear memory used by previous iterations
	if (myMemory)
		free(myMemory); /* in case this is not the first time initmem2 is called */

	if (head)
		free(head);

	printf("Setup memory \n");
	myMemory = malloc(sz);
	head = malloc(sizeof(struct memoryList));
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

	// Set up a pointer to the block that we will allocate this memory to
	struct memoryList *matching_block = NULL;

	assert((int)myStrategy > 0);

	switch (myStrategy)
	{
	case NotSet:
		return NULL;
		break;
	case First:
		return NULL;
		break;
	case Best:
		matching_block = find_block_best(requested);
		break;
	case Worst:
		matching_block = find_block_worst(requested);
		break;
	case Next:
		matching_block = find_block_next(requested);
		break;
	}

	// Our search didn't yield a compatible block, log this and do not allocate any memory.
	if (!matching_block)
	{
		fprintf(stderr, "No suitable block found \n");
		return NULL;
	}

	// If request is smaller than this blocks current size, then we will have leftover memory. Thus we need to create a new node in the list to contain this leftover memory
	if (matching_block->size > requested)
	{
		insertBlock(matching_block, requested);
	}
	// Since we will only enter this part of the code if the block that was found is exactly the size of the request -
	// We do not need to make a new node, since the current list structure can facilitate the allocattion of the request, with memory leftover.
	// simply update the currentnode to point to the current nodes next node. Before allocating it and returning the pointer for the matched block

	// This could also be seen as (if block->size == requested)
	else
	{
		currentnode = matching_block->next;
	}
	// Indicate that the matched block has been allocated and return a pointer to it.
	matching_block->alloc = 1;

	return matching_block->ptr;
}

void myfree(void *block)
{
	// Iniate a pointer to traverse the list
	struct memoryList *trav;
	// Since its a circular list, make sure we dont loop forever, by stopping at the last node.
	for (trav = head; trav->next != head; trav = trav->next)
	{
		if (trav->ptr == block)
		{
			break;
		}
	}
	// Mark the block as freed. If no adjacent blocks are also free, then do nothing else.
	trav->alloc = 0;

	// If the block isnt the head of the list, and the previous node isn't allocated merge into one block
	if ((trav != head) && !(trav->prev->alloc))
	{
		// set up helper pointer
		struct memoryList *previous = trav->prev;
		free_adjacent(trav);
		// since we are merging the contents of this block into the adjacent block, move the trav pointer space back in the list
		trav = previous;
	}

	// likewise for the next block
	if (trav->next != head && !(trav->next->alloc))
	{
		free_adjacent(trav->next);
	}
}

void *free_adjacent(struct memoryList *blockToMerge)
{

	// Merge the matching blocks memory into the previous block
	blockToMerge->prev->size += blockToMerge->size;

	// Setup the new connection after removal from the list
	blockToMerge->prev->next = blockToMerge->next;
	blockToMerge->next->prev = blockToMerge->prev;

	// If our currentnode is pointing to the block to be freed, make sure currentnode doesnt point to a freed node.
	if (currentnode == blockToMerge)
	{
		currentnode = currentnode->prev;
	}

	// free the node
	free(blockToMerge);
}

void insertBlock(struct memoryList *node, size_t requested)
{
	// Create a new node, this node is to be set adjacent to the matched node (current node)
	struct memoryList *newnode = malloc(sizeof(struct memoryList));

	// Setting up connection for the new node
	newnode->next = node->next;
	newnode->next->prev = newnode;
	newnode->prev = node;
	node->next = newnode;

	// set the values for the newnode
	// The size of the new (free) node will be whatever is remaining of the matched node after subtracting the requested memory space
	newnode->size = node->size - requested;
	newnode->ptr = node->ptr + requested;
	newnode->alloc = 0;

	// set the matched node to be equal the size of the request
	node->size = requested;

	// Make sure we start from this point when inserting new node
	currentnode = newnode;
}

// find a suitable block in memory
struct memoryList *find_block_next(size_t requested)
{
	// since im implementing next-fit make sure we start from currentnode, instead of head when searching through list.
	struct memoryList *start = currentnode;

	do
	{
		// If we find an unallocated node, with size equal to or greater than the requested memory space then return that node.
		if ((currentnode->alloc == 0) && currentnode->size >= requested)
		{
			return currentnode;
		}

		// Ensure we don't loop indefinitely
	} while ((currentnode = currentnode->next) != start);

	// if we dont find a node, that means that there are no suitable nodes in memory return null
	return NULL;
}

struct memoryList *find_block_worst(size_t requested)
{
	mem_largest_free();
	return largestFree;
}

struct memoryList *find_block_first(size_t requested)
{
}

struct memoryList *find_block_best(size_t requested)
{
	struct memoryList *lowest = NULL;
	struct memoryList *trav = head;
	int lowestSize = mySize + 1;

	do
	{
		if (trav->alloc == 0)
		{
			if (trav->size >= requested && trav->size < lowestSize)
			{
				lowest = trav;
				lowestSize = lowest->size;
			}
		}
	} while ((trav = trav->next) != head);

	if (lowest)
	{
		return lowest;
	}
	else
	{
		return NULL;
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

	struct memoryList *trav = head;
	int count = 0;

	do
	{
		// if trav->alloc == 0 then we have found a hole, add one to the count
		if (!trav->alloc)
		{
			count += 1;
		}
		// loop from start to end
	} while ((trav = trav->next) != head);

	return count;
}

/* Get the number of bytes allocated */
int mem_allocated()
{
	return mySize - mem_free();
}

/* Number of non-allocated bytes */
int mem_free()
{

	int count = 0;

	// iterate over list
	struct memoryList *trav = head;
	do
	{
		// If the block isnt allocated add its size to the total pool of free memory
		if (!(trav->alloc))
		{
			count += trav->size;
		}
	} while ((trav = trav->next) != head);

	return count;
}

/* Number of bytes in the largest contiguous area of unallocated memory */
int mem_largest_free()
{

	// Iterate over memory list and find the largest unallocated node

	largestFree = NULL;

	struct memoryList *trav = head;
	do
	{
		if (!trav->alloc)
		{
			if (!largestFree)
			{
				largestFree = trav;
			}
			else if (trav->size > largestFree->size)
			{
				largestFree = trav;
			}
		}
	} while ((trav = trav->next) != head);

	if (largestFree)
	{
		return largestFree->size;
	}
	else
	{
		return 0;
	}
}

/* Number of free blocks smaller than "size" bytes. */
int mem_small_free(int size)
{
	int count = 0;

	// iterate through the list and find the number of allocated bytes smaller than size

	for (struct memoryList *trav = head; trav->next != head; trav = trav->next)
	{
		if (trav->size <= size && !(trav->alloc))
		{
			count += 1;
		}
	}

	return count;
}

char mem_is_alloc(void *ptr)
{

	//  Iterate over the list
	struct memoryList *trav;

	for (trav = head; trav->next != head; trav = trav->next)
	{

		if (ptr < trav->next->ptr)
		{
			return trav->alloc;
		}
	}
	return trav->alloc;
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
	printf("Memory List {\n");
	/* Iterate over memory list */
	struct memoryList *index = head;
	do
	{
		printf("\tBlock %p,\tsize %d,\t%s\n",
			   index->ptr,
			   index->size,
			   (index->alloc ? "[ALLOCATED]" : "[FREE]"));
	} while ((index = index->next) != head);
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