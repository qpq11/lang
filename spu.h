#ifndef CPU_H
#define CPU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "clearBuffer.h"

const int minInt = -99999999;

const int maxInt = 9999999;

const int minVal = -9;

const int maxVal = 66;

const int maxLen = 18; //the biggest amount of digits possible in a valid command; if surpassed, program will read the first maxLen digits as one command and then go on to the new one

const int InitCapacity = 1000;

//const int ExitRec = 1; //the "beginner" condition for a recursion; if this or lesser value appears in stack, we return to main body

char* MyFgets (char* str, size_t n, FILE * stream);

void DeLabel(int *reads, int i);

int LabelDef(int lbl, int* reads, int n);

struct spu* newSpu(int capacity);

void SpuCtor(struct spu* pt);

void SpuDtor(struct spu* pt);

int interpreter(int* j, struct spu* pt, int* reads);

void neg(struct stack* pt);

void root(struct stack* pt);

void add(struct stack* pt);

void ddiv(struct stack* pt);

void sub(struct stack* pt);

void mul(struct stack* pt);

void in(struct stack* pt);

void rpush(struct spu* pt, int regn);

void rpop(struct spu* pt, int regn);

enum RAMst
{
	FREE = 0,
	TAKEN = 1,
};

struct RAMcell
{
	int val = 0;
	int status = FREE;
	int declScope = 0;
};

#endif