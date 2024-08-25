#include <stdio.h>
#include <stdlib.h>
#include "stack.h"
#define LOG(...) fprintf(__VA_ARGS__)

FILE * logStream = stdout;

const int POISON = 0xDEADDEAD;
const int StackMultiplier = 2;
const int InitCapacity = 1000;
const int EmptyCoeff = 4;

// // int main()
// // {
	// // //struct stack* pt = newStack(78);
	// // struct stack* pt = (struct stack*)calloc(sizeof(struct stack), 1);
	// // stackCtor(pt);
	// // printf("%d\n", isEmpty(pt));
    // // push(pt, 1);
    // // push(pt, 2);
    // // push(pt, 3);
	// // push(pt, POISON);
	// // pt->size += 32;
	// // push(pt, 100);
	// // pt->size -= 32;
	// // stackDump(pt);
	// // //char s1[] = "";
	// // //char s2[] = "";
	// // //scanf("%s %s\n", s1, s2);
    // // printf("The top element is %d\n", top(pt));
    // // printf("The stack size is %d\n", pt->size);
	// // pop(pt);
	// // // pt-
	// // pop(pt);
	// // printf("The top element is %d\n", top(pt));
    // // printf("The stack size is %d\n", pt->size);
    // // pop(pt);
    // // pop(pt);

    // // if (isEmpty(pt)) {
        // // printf("The stack is empty");
    // // }
    // // else {
        // // printf("The stack is not empty");
    // // }
// // }

/*struct stack* newStack(int maxsize)
{
    struct stack* pt = (struct stack*)calloc(sizeof(struct stack), 1);

    pt->capacity = maxsize;
    pt->top = -1;
    pt->items = (int*)calloc(sizeof(int), capacity);

    return pt;
}*/

void stackDtor (struct stack* pt)
{
	assert(pt != nullptr);
	pt->capacity = 0;
	//pt->top = 0;
	pt->data = nullptr;
	pt->size = 0;
}

//doesn't return the stack pointer, unlike newStack
void stackCtor (struct stack* pt)
{
	assert(pt != nullptr);
	pt->capacity = InitCapacity;
	pt->size = 0;
	pt->data = (elem_t*) calloc(InitCapacity, sizeof(elem_t));
}

/*int size(struct stack* pt) {
    return pt->top + 1;
}*/

int isEmpty(struct stack* pt) 
{
    return pt->size == 0;
}

int isFull(struct stack* pt) 
{
    return pt->size == pt->capacity;
}

int considerablyEmpty(struct stack* pt)
{
	
	return (pt->capacity / EmptyCoeff >= pt->size);
}

void push(struct stack* pt, elem_t x)
{
    if (isFull(pt))
    {
        LOG(logStream, "Stack overflow detected\nWill try increasing stack's capacity\n");
		stackIncrease(pt);
        //exit(EXIT_FAILURE);
    }

    //LOG(logStream, "Inserting %d\n", x);

    pt->data[pt->size++] = x;
}

elem_t top(struct stack* pt)
{
    assert(pt != nullptr);
    if (!isEmpty(pt)) {
        return pt->data[pt->size - 1];
    }
    return POISON;
}

elem_t pop(struct stack* pt, elem_t* x)
{
	//assert(pt != nullptr);
	assert(!isEmpty(pt));

    //LOG(logStream, "Removing %d\n", top(pt));
	
    elem_t popElem = pt->data[pt->size - 1]; //e.g.: data = [1,2,3] => will make size=2 and print data[2] = 3
	pt->size--;
	pt->data[pt->size] = POISON;
	return popElem;
}

void stackDump (struct stack* pt)
{
	if (pt->data == nullptr) 
	{
		LOG (logStream, "Dump started.\n");
		LOG (logStream, " Empty stack: %17p\n", pt);
		LOG (logStream, " Size: %24u\n", pt->size);
		LOG (logStream, " Capacity: %20u\n", pt->capacity);
		LOG (logStream, " Address start: nullptr\n\n");
	}
	
	else
	{
		LOG (logStream, "Dump started.\n");
		LOG (logStream, " Empty stack: %17p\n", pt);
		LOG (logStream, " Size: %24u\n", pt->size);
		LOG (logStream, " Capacity: %20u\n", pt->capacity);
		LOG (logStream, " Address start: %15p\n", pt->data);
		LOG (logStream, " Address   end: %15p\n", pt->data + sizeof (elem_t) * pt->capacity);
	}
	
	for (size_t i = 0; i < pt->capacity; i++)
	{
		if (i >= pt->size)
			LOG (logStream, "| stack[%7d] =         POISON |\n", i);
		else
			LOG (logStream, "| stack[%7d] = %18d |\n", i,  pt->data[i]);
	}
	
	LOG(logStream, "End of dump.\n");
}

elem_t* recallocStack (struct stack* pt, const size_t capacity)
{
    assert(pt != nullptr);

    elem_t* data = pt->data;

    if (data == nullptr) 
    {
        LOG(logStream, "RECALLOC FAILED\n");
        return nullptr;
    }

    nullValueSet (data + pt->size, capacity - pt->size);

    return data;
}

void nullValueSet (elem_t* data, size_t size)
{   
    for (size_t i = 0; i < size; i++)
    {
        data[i] = POISON;
    }
}

void stackIncrease (struct stack* pt)
{
    if (isFull (pt))
    {
        size_t newCapacity = pt->capacity;
        newCapacity = newCapacity * StackMultiplier;
        
        elem_t* data = recallocStack (pt, newCapacity);

        assert (data != nullptr);

        pt->data = (elem_t*) data;
        pt->capacity = newCapacity;
    }
}

void stackDecrease(struct stack* pt)
{
	if (considerablyEmpty(pt))
	{
		size_t newCapacity = (int) pt->capacity / EmptyCoeff;
		elem_t* items = recallocStack(pt, newCapacity);
		
		assert(items != nullptr);
		
		pt->data = items;
		pt->capacity = newCapacity;
	}
}