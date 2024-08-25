#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

const int minVal = -9;

const int maxVal = 66;

char* MyFgets (char* str, size_t n, FILE * stream);

const int maxWord = 8; //max amount of symbols taken by MyFgets

const int maxLabels = 100; //max amount of labels supported

typedef struct
{
	char* name;
	int ip;
} label;

typedef struct
{
	label* larr;
	int capacity;
	int lsize;
} labelList;

int LabelSearch(labelList *Labels, char* s);

void LabelAdd(labelList *Labels, char* s, int ip);

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

struct Codewords
{
	const char text[maxWord];
	Commands code;
};

const struct Codewords CodeArray[] = {  //CodeArray[CmdCnt]
	{"hlt",    chlt    },
	{"push",   cpush   },
	{"pow",    cpow    },
	{"pop",    cpop    },
	{"neg",    cneg    },
	{"sqrt",   csqrt   },
	{"add",    sadd    },
	{"sub",    ssub    },
	{"div",    sdiv    },
	{"mul",    smul    },
	{"out",    stout   },
	{"in",     stin    },
	{"rpush",  regpush},
	{"rpop",   regpop  },
	{"jump",   jmp     },
	{"jnl",    jnl     },
	{"jnm",    jnm     },
	{"jl",     jl      },
	{"jm",     jm      },
	{"je",     je      },
	{"jne",    jne     },
	{"jz",     jz      },
	{"ret",    pret    },
	{"call",   pcall   },
	{"vpush",   rampush},
	{"vpop",   rampop  },
	{"relpush", relpush},
	{"relpop",   relpop}
};
	
int IsLabel(char* s);	

const size_t CmdCnt = sizeof(CodeArray) / sizeof(CodeArray[0]);

const char* regs[] = {"ax", "bx", "cx", "dx"}; //registers for common use

int MyOrg(int ip, int endOrg, FILE * resfp);

int IsReg(const char* s, FILE * resfp);

int MyAssembler (char* s);

const int maxLen = 18; //size of a buffer; is made much bigger than maxWord in order to avoid overflows

int main(int argc, char** argv)
{
	//char *w = "bx"; //98
	//printf("%d\n", w[0]); 
	FILE * fp = nullptr;
	FILE * resfp = nullptr;
	
	fclose(fp);
	
	if (argc>1)
		fp = fopen(argv[1], "r");
	else
		fp = fopen("normaltext.txt", "r");
	assert(fp != nullptr);
	
	char *s = (char*) calloc(sizeof(char), maxLen);
	//printf("%d\n", s[0]);
	int NewLineCondition = 0; //condition not to use the '\n' symbol
	
	labelList Labels = {(label*) calloc(maxLabels, sizeof(label)), maxLabels, 0};
	
	resfp = fopen("asm.txt", "w");
	
	assert(fp != nullptr);
	assert(resfp != nullptr);
	int ip = 0; //counts the amount of array cells taken by commands or nil strings
	
	while((MyFgets(s, maxWord, fp)) != NULL)
	{
		printf("got %s\n", s);
		if(!strcmp(s, ""))
		{
			//fprintf(resfp, "0\n");
			ip++;
			continue;
		}
		if (!strcmp(s, "org"))
		{
			int endOrg = atoi(MyFgets(s, maxWord, fp));
			ip = MyOrg(ip, endOrg, resfp);
			continue;
		}
		
		// // if (!strcmp(s, "jnl") || !strcmp(s, "jnm"))
		// // {
			// // fprintf(resfp, "%d ", MyAssembler(s));
			// // MyFgets(s, maxWord, fp);
			// // fprintf(resfp, "%d ", MyAssembler(s));
			// // NewLineCondition = 1;
			// // ip += 2;
			// // continue;
		// // }
		
		if(IsLabel(s))
		{
			ip++;
			int Exists = LabelSearch(&Labels, s);
			if (!Exists)
			{
				LabelAdd(&Labels, s, ip);
				fprintf(resfp, ".L%d\n", ip);
			}
			else
			{
				fprintf(resfp, ".L%d\n", Exists);
			}
			//fprintf(resfp, ".L%d\n", ip);
			NewLineCondition = 0;
			continue;
		}
		
		if (strlen(s) == 2 && s[1] == 'x')
		{
			int isregister = IsReg(s, resfp); //Checking whether or not inputed word is a name of existing register
			ip++;
			printf("assembled %d commands rn\n", ip);
			if (isregister)
			{
				NewLineCondition = 0;
				continue;
			}
		}
		int as = MyAssembler(s);
		ip++;
		//printf("turned into %d\n", as);
		if ((as>0) && (as!=pret) && (!NewLineCondition))
		{
			fprintf(resfp, "%d ", as);
			NewLineCondition = 1;
		}
		else
		{
			fprintf(resfp, "%d\n", as);
			NewLineCondition = 0;
		}
		
		printf("$$%d ", MyAssembler(s));
		//printf("from %s\n", s);
		printf("NewLineCondition = %d\n", NewLineCondition);
		printf("assembled %d commands rn\n", ip);
	}
	free(s);
	free(Labels.larr);
	fclose(fp);
	fclose(resfp);
	return 0;
}

int IsLabel(char* s)
{
	return (s[0] == ':') ? 1 : 0;
}

int LabelSearch(labelList *Labels, char* s)
{
	printf("#*# %d\n", Labels->lsize);
	for (int i = 0; i < Labels->lsize; i++)
	{
		printf("#*# %s %s\n", Labels->larr[i].name, s);
		if(!strcmp(Labels->larr[i].name, s))
		{
			printf("Not a new label: %s\n", s);
			return Labels->larr[i].ip;
		}
	}
	return 0;
}

void LabelAdd(labelList *Labels, char* s, int ip)
{
	 Labels->larr[Labels->lsize].name = strdup(s);
	//memcpy(Labels->larr[Labels->lsize]->name, s, maxWord);
	Labels->larr[Labels->lsize].ip = ip;
	printf("$$$%s %d\n", Labels->larr[Labels->lsize].name, Labels->larr[Labels->lsize].ip);
	Labels->lsize += 1;
	if(Labels->lsize == Labels->capacity)
		puts("Unexpected behaviour: Label limit reached \n"); 
}

int MyOrg (int ip, int endOrg, FILE * resfp)
{
	int beginOrg = ip;
	//int endOrg = atoi(MyFgets(s, 8, fp));
	for(int i = beginOrg; i < endOrg; i++)
	{
		fprintf(resfp, "%d\n", 0);
		printf("now on pos %d\n", i);
	}
	return endOrg;
}

int IsReg(const char* s, FILE * resfp)
{
	int flag = 1;
	if ((int) s[0] >= 97 && (int) s[0] <=100)
		fprintf(resfp, "%d\n", ((int) s[0]) - 96);
	else
		flag = 0;
	return flag;
}

int MyAssembler(char* s)
{
	//printf("%s \n", s);
	for (int i = 0; i < CmdCnt; i++)
	{	
		//printf("%s %s \n", s, CodeArray[i].text);
		if (!strcmp(CodeArray[i].text, s))
		{
			//printf("n %d\n", (int) CodeArray[i].code);
			return CodeArray[i].code;
		}
	}
	//printf("not a command\n");
	if (s[0] == '[')
	{
		puts("address of var");
		char buf[maxLen] = "";
		sscanf(s, "[%17[^]]", buf);
		printf("%s\n", buf);
		return atoi(buf);
	}
	return atoi(s);
}

/**
* Works like fgets() but also stops on each space symbol
*/
char* MyFgets (char* str, size_t n, FILE * stream)
{
	int i = 0;
	int ch;
	int moreThanEof = 0;
	//printf("-----");
	while (((ch = fgetc(stream)) != '\n') && (ch != EOF) && (ch) && (i < n) && (ch != ' '))
	{
		//printf("more than EOF; ch is %c\n", ch);
		*(str + i++) = ch;
		moreThanEof = 1;
	}
	
	/*if (i < n)
	{
		*(str + i++) = '\n';
	}*/
	
	*(str + i) = '\0';
	
	if (moreThanEof && ch == EOF)
	{
		return str;
	}
	
	if (!moreThanEof && ch == EOF)
	{
		return NULL;
	}
	
	
	return str;
	
}