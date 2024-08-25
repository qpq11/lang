#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "spu.h"
#include <assert.h>
#include "stack.h"

struct spu
{
	struct stack* stk;
	struct stack* retStk;
	elem_t rax;
	elem_t rbx;
	elem_t rcx;
	elem_t rdx;
	//int jmx; //to operate jump with condition
	//int cmx; //flag variable
};

enum Commands
{
	chlt    =  0,
	cpush   =  1,
	cpow    = -10,
	cpop    = -9,
	cneg    = -8,
	csqrt   = -7,
	sadd    = -6,
	ssub    = -5,
	sdiv    = -4,
	smul    = -3,
	stout   = -2,
	stin    = -1,
	regpush = 16,
	regpop  = 17,
	jmp     = 33,
	jnl     = 34,
	jnm     = 35,
	jl      = 36,
	jm      = 37,
	je      = 38,
	jne     = 39,
	jz      = 40,
	pret    = 65,
	pcall   = 66,
	rampush = 67,
	rampop  = 68,
	relpush = 69,
	relpop  = 70,
};

const int RamSize = 2048;
const int SingleOffset = 64;
static int offsetCnt = 0;
static RAMcell ram[RamSize] = {};
static int scopeFrameUsage[RamSize/SingleOffset] = {0};

int main(int argc, char** argv)
{
	// elem_t* ram = (int*) calloc(sizeof(int), RamSize);
	FILE * fp = nullptr;
	if (argc>1)
		fp = fopen(argv[1], "r");
	else
		fp = fopen("asm.txt", "r");
	
	assert(fp != nullptr);
	
	//struct spu* proc = newSpu(InitCapacity);
	struct spu* proc = (struct spu*) calloc(sizeof(struct spu), 1);
	proc->stk    = (struct stack*) calloc(InitCapacity, 1);
	proc->retStk = (struct stack*) calloc(InitCapacity, 1);
	
	SpuCtor(proc);
	
	assert(proc != nullptr);
	
	int* reads = nullptr;
	reads = (int*) calloc(sizeof(int), InitCapacity);
	
	char *s = nullptr;
	s = (char*) calloc(sizeof(char), maxLen);
	
	int i = 0;

	while((MyFgets(s, maxLen, fp)) != NULL)
	{
		if (s[0] == '.')           //?
		{
			int n = atoi(s + 2) - 100* (maxInt + 1); //numbers of 9 digits are NOT expected to be assembled correctly (maxWord = 8), so a number being of 9 digits pretty obviously means that this isn't a NUMBER but a LABEL spotted by spu
			*(reads + i) = n;
			//printf("%d\n", n);
			i++;
			continue;
		}
		if(!strcmp("", s))
		{
			continue;
		}
		int n = atoi(s);
		//printf("%d\n", n);
		*(reads + i) = n;
		i++;
	}
	//printf("%d \n", i);
	DeLabel(reads, i-1);
	//printf("%d \n", i);
	
	int ip = 0;
	
	while(ip < i)
	{
		//printf("%d %d\n", ip, i);
		ip+= (1 + interpreter(&ip, proc, reads));
		
		//printf("ip changed to %d\n", ip);
	}
	//stackDump(proc->stk);
	SpuDtor(proc);
	return 0;
}
//function detecting labels
void DeLabel(int *reads, int i)
{
	while(i>=0)
	{
		int n = i;
		int lbl = *(reads + i);
		//printf("$%d %d %d %d\n", i, lbl, *(reads + i), *(reads + i - 1));
		if (lbl < minInt)
		{
			int defAddress = LabelDef(lbl, reads, n);
			//printf("$%d \n", defAddress);
			*(reads + i) = defAddress;
			//printf("$%d \n", *(reads + i));
			while(n>=0)
			{
				if (lbl == *(reads + n))
					*(reads + n) = defAddress;
				n--;
			}
		}
		i--;
	}
}


//function that finds given label's definition and returns its address
int LabelDef(int lbl, int* reads, int n)
{
	while (n>=0)
	{
		if ( *(reads + n) == lbl && (*(reads + n -1) != pcall) && ((*(reads + n - 1) < (int) jmp) || (*(reads + n -1) > (int) jz)))
		{
			//printf("$###$ %d %d\n", *(reads + n), *(reads + n - 1));
			//printf("$#$#$ %d %d\n", lbl, n);
			return n;
		}
		n--;
	}
}

int interpreter(int* ip, struct spu* pt, int* reads)
{
	//printf("%d element in reads is %d\n", *(ip), reads[*(ip)]);
	assert(pt != nullptr);
	if(*(ip) == 0 && reads[*(ip)] == 0)
	{
		//printf("skip main label\n");
		*ip = 1;
	}
	switch ((Commands) reads[*(ip)])
	{
		case chlt: 
		{
			//printf("stumbled onto halt; program terminated. position %d\n", *ip);
			break;
		}
		case cpush: 
		{
			//printf("push launched, gotta push %d\n", *(ip)+1);
			push(pt->stk, reads[*(ip)+1]);
			scopeFrameUsage[offsetCnt]++;
			//printf("frame %d usage:%d\n", offsetCnt, scopeFrameUsage[offsetCnt]);
			return 1;
		}
		case cpop: 
		{
			pop(pt->stk);
			scopeFrameUsage[offsetCnt]--;
			//printf("frame usage:%d\n", scopeFrameUsage[offsetCnt]);
			return 0;
		}
		case cpow:
		{
			int a1 = pop(pt->stk);
			int a2 = pop(pt->stk);
			int res = pow(a2, a1);
			if (a1 == 2) res = a2 * a2;
			//printf("%d %d %d\n", a2, a1, (int) pow(a2, a1)); //(-2, 5) = 24?????
			push(pt->stk, res);
			scopeFrameUsage[offsetCnt]--;
			//printf("frame %d usage:%d\n", offsetCnt, scopeFrameUsage[offsetCnt]);
			return 0;
		}
		case cneg:
		{
			neg(pt->stk);
			return 0;
		}
		case csqrt:
		{
			root(pt->stk);
			return 0;
		}
		case sadd: 
		{
			add(pt->stk);
			scopeFrameUsage[offsetCnt]--;
			//printf("frame %d usage:%d\n", offsetCnt, scopeFrameUsage[offsetCnt]);
			return 0;
		}
		case ssub: 
		{
			sub(pt->stk);
			scopeFrameUsage[offsetCnt]--;
			//printf("frame %d usage:%d\n", offsetCnt, scopeFrameUsage[offsetCnt]);
			return 0;
		}
		case sdiv: 
		{
			ddiv(pt->stk);
			scopeFrameUsage[offsetCnt]--;
			//printf("frame %d usage:%d\n", offsetCnt, scopeFrameUsage[offsetCnt]);
			return 0;
		}
		case stout: 
		{
			printf("---------------\nPRINTING OUT %d\n---------------\n", top(pt->stk));
			//pop(pt->stk);
			return 0;
		}
		case smul:
		{
			mul(pt->stk);
			scopeFrameUsage[offsetCnt]--;
			//printf("frame %d usage:%d\n", offsetCnt, scopeFrameUsage[offsetCnt]);
			return 0;
		}
		case stin:
		{
			in(pt->stk);
			scopeFrameUsage[offsetCnt]++;
			//printf("frame %d usage:%d\n", offsetCnt, scopeFrameUsage[offsetCnt]);
			return 0;
		}
		case regpush:
		{
			//printf("rpush launched, boutta push %d into stack\n", pt->rax);
			rpush(pt, reads[*(ip)+1]);
			scopeFrameUsage[offsetCnt]++;
			//printf("frame %d usage:%d\n", offsetCnt, scopeFrameUsage[offsetCnt]);
			return 1;
		}
		case regpop:
		{
			rpop(pt, reads[*(ip)+1]);
			scopeFrameUsage[offsetCnt]--;
			//printf("frame %d usage:%d\n", offsetCnt, scopeFrameUsage[offsetCnt]);
			return 1;
		}
		case jmp:
		{
			///puts("received jump");
			
			*ip = reads[*(ip)+1];
			printf("Jumping to %d\n", *ip);
			return 0;
		}
		case jnl:					//[rsp] <= [rsp + 4]
		{
			//puts("JNL");
			elem_t a1 = pop(pt->stk);
			elem_t a2 = pop(pt->stk);
			scopeFrameUsage[offsetCnt] -= 2;
			//printf("frame %d usage:%d\n", offsetCnt, scopeFrameUsage[offsetCnt]);
			//printf("%d %d\n", a1, a2);
			//push(pt->stk, a2);
			//push(pt->stk, a1);
			if (a1 >= a2)
			{
				*ip = reads[*(ip)+1];
				return 0;
			}
			return 1;
		}
		case jnm:
		{
			elem_t a1 = pop(pt->stk);
			elem_t a2 = pop(pt->stk);
			scopeFrameUsage[offsetCnt] -= 2;
			//printf("frame %d usage:%d\n", offsetCnt, scopeFrameUsage[offsetCnt]);
			//push(pt->stk, a2);
			//push(pt->stk, a1);
			if (a1 <= a2)
			{
				*ip = reads[*(ip)+1];
				return 0;
			}
			return 1;
		}
		case jl:
		{
			elem_t a1 = pop(pt->stk);
			elem_t a2 = pop(pt->stk);
			scopeFrameUsage[offsetCnt] -= 2;
			//printf("frame %d usage:%d\n", offsetCnt, scopeFrameUsage[offsetCnt]);
			//push(pt->stk, a2);
			//push(pt->stk, a1);
			if (a1 > a2)
			{
				puts("JL");
				*ip = reads[*(ip)+1];
				return 0;
			}
			return 1;
		}
		case jm:
		{
			elem_t a1 = pop(pt->stk);
			elem_t a2 = pop(pt->stk);
			scopeFrameUsage[offsetCnt] -= 2;
			//printf("frame %d usage:%d\n", offsetCnt, scopeFrameUsage[offsetCnt]);
			//push(pt->stk, a2);
			//push(pt->stk, a1);
			if (a1 < a2)
			{
				*ip = reads[*(ip)+1];
				return 0;
			}
			return 1;
		}
		case je:
		{
			elem_t a1 = pop(pt->stk);
			elem_t a2 = pop(pt->stk);
			scopeFrameUsage[offsetCnt] -= 2;
			//printf("frame %d usage:%d\n", offsetCnt, scopeFrameUsage[offsetCnt]);
			//push(pt->stk, a2);
			//push(pt->stk, a1);
			if (a1 == a2)
			{
				puts("JE");
				*ip = reads[*(ip)+1];
				return 0;
			}
			return 1;
		}
		case jne:
		{
			elem_t a1 = pop(pt->stk);
			elem_t a2 = pop(pt->stk);
			scopeFrameUsage[offsetCnt] -= 2;
			//printf("frame %d usage:%d\n", offsetCnt, scopeFrameUsage[offsetCnt]);
			//push(pt->stk, a2);
			//push(pt->stk, a1);
			if (a1 != a2)
			{
				puts("JNE");
				*ip = reads[*(ip)+1];
				return 0;
			}
			return 1;
		}
		case jz:
		{
			elem_t a1 = pop(pt->stk);
			scopeFrameUsage[offsetCnt]--;
			//printf("frame %d usage:%d\n", offsetCnt, scopeFrameUsage[offsetCnt]);
			//push(pt->stk, a1);
			if (!a1)
			{
				*ip = reads[*(ip)+1];
				return 0;
			}
			return 1;
		}
		case pret:
		{
			printf("ret launched!\n");
			//if (pt->rax > ExitRec)
			//{
			*ip = top(pt->retStk);
			pop(pt->retStk);
			printf(">> %d\n", scopeFrameUsage[offsetCnt]);
			// int sum = 0;
			// for (int k = 0; k < offsetCnt; k++)
			// {
				// sum+=scopeFrameUsage[k];
			// }
			for(int i = 0; i < SingleOffset-1; i++)
			{
				pop(pt->stk);
				printf("%d\n", pt->stk->size);
			}
			for(int j = RamSize/2 + offsetCnt*SingleOffset; j < RamSize/2 + offsetCnt*SingleOffset + scopeFrameUsage[offsetCnt]; j++)
			{
				if (ram[j].declScope == offsetCnt)
				{
					//printf("ramcler: %d %d\n", j, ram[j].val);
					ram[j].val = 0;
					ram[j].declScope = 0;
					//localCnt--;
				}
			}
			scopeFrameUsage[offsetCnt] = 0;
			offsetCnt--;
			printf(">>>%d %d\n", pt->stk->size, offsetCnt);
			return -1;
			//}
			//printf("going back to the main body\n");
			// *ip = top(pt->retStk) + 2; 			// return -1;
			
		}
		case pcall:
		{
			printf("called pcall, will save %d to retStk\n", *ip + 2);
			push(pt->retStk, *ip + 2); //return to the command right after "call x"
			*ip = reads[*(ip)+1];
			(pt->stk)->size = (pt->stk->size / SingleOffset + 1) * SingleOffset;
			offsetCnt++;
			printf(">>>%d %d\n", pt->stk->size, offsetCnt);
			printf("$$$$%d \n", *ip);
			return 0;
		}
		case rampush:
		{
			//puts("RAMPUSH");
			push(pt->stk, ram[reads[*(ip) + 1]].val);
			scopeFrameUsage[0]++;
			//printf("frame %d usage:%d\n", 0, scopeFrameUsage[0]);
			if(reads[*(ip) + 1] >= RamSize/2) puts("WARNING: ENTERED LOCAL ALLOC'D MEMORY ZONE");
			return 1;
		}
		case rampop:
		{
			//puts("RAMPOP");
			ram[reads[*(ip) + 1]].val = pop(pt->stk);
			ram[reads[*(ip) + 1]].status = (int) TAKEN;
			scopeFrameUsage[0]--;
			//printf("frame %d usage:%d\n", 0, scopeFrameUsage[0]);
			if(reads[*(ip) + 1] >= RamSize/2) puts("WARNING: ENTERED LOCAL ALLOC'D MEMORY ZONE");
			return 1;
		}
		case relpush:
		{
			//puts("RELPUSH");
			scopeFrameUsage[offsetCnt]++;
			//printf("frame %d usage:%d\n", offsetCnt, scopeFrameUsage[offsetCnt]);
			push(pt->stk, ram[reads[*(ip) + 1] + offsetCnt*SingleOffset + RamSize/2].val);
			return 1;
		}
		case relpop:
		{
			//puts("RELPOP");
			scopeFrameUsage[offsetCnt]--;
			//printf("frame %d usage:%d\n", offsetCnt, scopeFrameUsage[offsetCnt]);
			ram[reads[*(ip) + 1] + offsetCnt*SingleOffset + RamSize/2].val = pop(pt->stk);
			ram[reads[*(ip) + 1] + offsetCnt*SingleOffset + RamSize/2].status = (int) TAKEN;
			ram[reads[*(ip) + 1] + offsetCnt*SingleOffset + RamSize/2].declScope = offsetCnt;
			//localCnt++;
			return 1;
		}
		default:
		{
			printf("unknown command: %d\n", reads[*(ip)]);
		}
	}
}

char* MyFgets (char* str, size_t n, FILE * stream)
{
	int i = 0;
	int ch;
	
	while (((ch = fgetc(stream)) != '\n') && (ch != EOF) && (ch) && (i < n) && (ch != ' '))
	{
		*(str + i++) = ch;
	}
	
	/*if (i < n)
	{
		*(str + i++) = '\n';
	}*/
	
	*(str + i) = '\0';
	
	if (ch == EOF)
		return NULL;
	
	return str;
	
}

void rpush(struct spu* pt, int regn)
{
	assert((regn > 0) && (regn < 5));
	switch(regn)
	{
		case 1:
		{
			puts("rpush ax");
			push(pt->stk, pt->rax);
			//pt->rax = 0;
			return;
		}
		case 2:
		{
			puts("rpush bx");
			push(pt->stk, pt->rbx);
			//pt->rbx = 0;
			return;
		}
		case 3:
		{
			puts("rpush cx");
			push(pt->stk, pt->rcx);
			//pt->rcx = 0;
			return;
		}
		case 4:
		{
			puts("rpush dx");
			push(pt->stk, pt->rdx);
			//pt->rdx = 0;
			return;
		}
		default:
		{
			printf("%d is an incorrect register number\n", regn);
			//rpop(pt, regn-1);
		}
	}
}

void rpop(struct spu* pt, int regn)
{
	assert(pt != nullptr);
	assert(regn > 0);
	//printf("rpop launched, going to pop %d into register %d\n", peek(pt->stk), regn);
	switch(regn)
	{
		case 1:
		{
			pt->rax = pop(pt->stk);
			//printf("register has %d\n", pt->rax);
			return;
		}
		case 2:
		{		
			pt->rbx = pop(pt->stk);
			return;
		}
		case 3: 
		{
			pt->rcx = pop(pt->stk);
			return;
		}
		case 4: 
		{
			pt->rdx = pop(pt->stk);
			return;
		}
		default:
		{
			printf("%d is an incorrect register number, cannot rpop there\n", regn);
			//rpop(pt, regn);
		}
	}
}

void SpuCtor(struct spu* pt)
{
	//pt->stk    = (int*) calloc(InitCapacity, sizeof(int));
	stackCtor(pt->stk);
	stackCtor(pt->retStk);
	pt->rax    = 0;
	pt->rbx    = 0;
	pt->rcx    = 0;
	pt->rdx    = 0;
	//pt->jmx    = 0;
}

void SpuDtor(struct spu* pt)
{
	assert(pt != nullptr);
	stackDtor(pt->stk);
	stackDtor(pt->retStk);
	pt->rax = 0;
	pt->rbx = 0;
	pt->rcx = 0;
	pt->rdx = 0;
	//pt->jmx = 0;
	free(pt);
}

void add(struct stack* pt)
{
	//puts("adding");
	elem_t a = pop(pt);
	elem_t b = pop(pt);
	push(pt, a+b);
	return;
}

void ddiv(struct stack* pt)
{
	elem_t a = pop(pt);
	elem_t b = pop(pt);
	if (!b)
		fprintf(stdout, "Attempted division by zero. Consequences are unpredictable\n");
	push(pt, b/a);
	return;
}

void sub(struct stack* pt)
{
	//puts("subing");
	elem_t a = pop(pt);
	elem_t b = pop(pt);
	push(pt, b-a);
	return;
}

void mul(struct stack* pt)
{
	elem_t a = pop(pt);
	elem_t b = pop(pt);
	push(pt, a*b);
	return;
}

void in(struct stack* pt)
{
	elem_t a = 0;
	scanf("%d", &a);
	push(pt, a);
	clearBuffer();
	return;
}

void root(struct stack* pt)
{
	//puts("doing sqrt\n");
	elem_t a = pop(pt);
	if (a < 0)
	{
		printf("CAN'T TAKE SQUARE ROOT OFF NEGATIVE NUMBERS\n");
		assert(a >= 0);
	}
	//puts("sqrt done successfully");
	push(pt, (elem_t) sqrt(a));
	return;
}

void neg(struct stack* pt)
{
	elem_t a = pop(pt);
	push(pt, -a);
	//puts("negation done");
	return;
}