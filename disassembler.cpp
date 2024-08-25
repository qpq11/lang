#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

const int minVal = -9;

const int maxVal = 66;

char* MyFgets (char* str, size_t n, FILE * stream);

const int CmdCnt = 23;

enum Commands
{
	chlt    =  0,
	cpush   =  1,
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
	pcall   = 66
};

/*struct Codewords
{
	const char text[maxWord];   //will work, but is not entirely correct as the max length of a command is less than maxLen
	Commands code;
};*/

const int MaxLen = 12;

Commands codes[] = {chlt, cpush, cpop, cneg, csqrt, sadd, ssub, sdiv, smul, stout, stin, regpush, regpop, jmp, jnl, jnm, pret, pcall};

const char* texts[] = {"hlt", "push", "pop", "neg", "sqrt", "add", "sub", "div", "mul", "out", "in", "rpush", "rpop", "jump", "jnl", "jnm", "ret", "call"};

/*const struct Codewords CodeArray[] = {  //CodeArray[CmdCnt]
	{"hlt",  chlt    },
	{"push", cpush   },
	{"pop",  cpop    },
	{"add",  sadd    },
	{"sub",  ssub    },
	{"div",  sdiv    },
	{"mul",  smul    },
	{"out",  stout   },
	{"in",   stin    },
	{"rpush", regpush},
	{"rpop", regpop  },
	{"jump", jmp     },
	{"jnl",  jnl     },
	{"jnm",  jnm     },
	{"ret",  pret    },
	{"call", pcall   }
};*/

const char* MyDisassembler (int n, int NoNewLine);

int main(int argc, char** argv)
{
	FILE * fp  = nullptr;
	FILE * res = nullptr;
	if (argc>1)
		fp = fopen(argv[1], "r");
	else
		fp = fopen("asm.txt", "r");
	
	int NoNewLine = 0; //condition not to use the '\n' symbol
	
	res = fopen("disasm.txt", "w");
	
	assert(fp != nullptr && res != nullptr && fp != res);
	
	char *s;
	s = (char*) calloc(sizeof(char), MaxLen);
	
	const char* dis = NULL;
	
	while((MyFgets(s, MaxLen, fp)) != NULL)
	{
		if(!strcmp(s, ""))
		{
			fprintf(res, "\n");
			continue;
		}
		if (s[0] == '.')
		{
			printf("%s\n", s);
			fprintf(res, "%s\n", s);
			NoNewLine = 0;
			continue;
		}
		int n = atoi(s);
		printf("%d ", n);
		//char *buf;
		//buf = (char*) calloc(sizeof(char), 16);
		dis = MyDisassembler(n, NoNewLine);
		//free(buf);
		printf("%s\n", dis);
		if ((n>0) && (!NoNewLine))
		{
			fprintf(res, "%s ", dis);
			NoNewLine = 1;
		}
		else if (NoNewLine)
		{
			fprintf(res, "%d\n", n);
			NoNewLine = 0;
		}
		else
		{
			fprintf(res, "%s\n", dis);
			NoNewLine = 0;
		}
	}
	fclose(fp);
	fclose(res);
	free(s);
	//int i = 0;
}

const char* MyDisassembler(int n, int NoNewLine)
{
	if (NoNewLine)
	{
		return "";
	}
	else
	{
		switch((Commands) n)
		{
			case chlt:     return "hlt";
			case cpush:    return "push";
			case cpop:     return "pop";
			case cneg:     return "neg";
			case csqrt:    return "sqrt";
			case sadd:     return "add";
			case ssub:     return "sub";
			case smul:     return "mul";
			case sdiv: 	   return "di";
			case stout:    return "out";
			case stin:     return "in";
			case regpush:  return "rpush";
			case regpop:   return "rpop";
			case jmp:      return "jump";
			case jnl:      return "jnl";
			case jnm:      return "jnm";
			case jl:       return "jl";
			case jm:       return "jm";
			case je:       return "je";
			case jne:      return "jne";
			case jz :      return "jz";
			case pret:     return "ret";
			case pcall:    return "call";
			default: puts("boi what the hell boi");
		}
		/**
			for (int i = 0; i < CmdCnt; i++)
			{
				if (CodeArray[i].code == (Commands) n)
					return (CodeArray[i].text);
			}
			return "something went wrong, try again";
		*/
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