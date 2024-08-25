#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef int elem_t;

struct stack
{
    size_t capacity = 0;
    size_t size = 0;
    elem_t* data = nullptr;     
};

struct stack* newStack(int capacity);

void stackCtor (struct stack* pt);

void stackDtor (struct stack* pt);

void stackDump (struct stack* pt);

elem_t* recallocStack (struct stack* const pt, const size_t capacity);

int isEmpty(struct stack* pt);

int isFull(struct stack* pt);

void push(struct stack* pt, elem_t x);

elem_t top(struct stack* pt);

elem_t pop(struct stack* pt, elem_t* x = nullptr);

int notHalfFull(struct stack* pt);

void nullValueSet (elem_t* data, size_t size);

void stackIncrease (struct stack* pt);

void stackDecrease(struct stack* pt);

#endif