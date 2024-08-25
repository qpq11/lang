#ifndef BACKEND_H
#define BACKEND_H

#define RAM_SIZE 1024

#define SPACESKIP \
while(c == ' ')\
		c = fgetc(fp);\

typedef union 
	{
		//unsigned char op;
		char* name;
		double value;
	}nodeUnion;

struct Node
{
	int tok = 0;
	nodeUnion un;
	int number;
	struct Node*  left    = NULL;
	struct Node*  right   = NULL;
	//int order = 0;
};


struct Compiler
{
    FILE* file_from;
    FILE* file_to;
    Node*  tree;
};

enum  DataTypes
{
	NIL = 0,
	NUM = 1,
	OP  = 2,
	VAR = 3,
	FNC = 4,
	FUNCNAME = 5,
};

enum OpNums
{
	ADD = '+',
	SUB = '-',
	MUL = '*',
	DIV = '/',
	POW = '^',
	SMC = ';',
	ASN = '=',
	EQU = '#',
	ABV = '>',
	BLW = '<',
};

enum FncNums
{
	SIN = 0,
	COS = 1,
	TG  = 2,
	CTG = 3,
	LN  = 4,
	EXP = 5,
};

enum Tokens
{
	EMPTY   = 0,
	TOK_NUM = 1,
	TOK_IN  = 2,
	TOK_OUT = 3,
	
	TOK_SIN = 4,
	TOK_COS = 5,
	TOK_TG  = 6,
	TOK_CTG = 7,
	TOK_LN  = 8,
	TOK_EXP = 9,
	
	TOK_ADD    = 10,
	TOK_SUB    = 11,
	TOK_MUL    = 12,
	TOK_DIV    = 13,
	TOK_POW    = 14,
	TOK_SMC    = 15,
	TOK_ASN    = 16,
	TOK_EQU    = 17,
	TOK_ABV    = 18,
	TOK_BLW    = 19,
	TOK_NEQU   = 20,
	TOK_EABV   = 21,
	TOK_EBLW   = 22,
	TOK_IF     = 23,
	TOK_WHILE  = 24,
	TOK_VAR    = 25,
	TOK_RET    = 26,
	TOK_OWNFNC = 27,
	TOK_1ARG   = 28,
	TOK_2ARG   = 29,
	TOK_3ARG   = 30,
	TOK_4ARG   = 31,
};

const int MAXLEXSIZE = 20;

struct Command
{
	char name[MAXLEXSIZE];
	Tokens tok;
};

const char* regList[] = {"ax", "bx", "cx", "dx"};

const Command TokCmn[] =
{
	{"",    TOK_NUM},
	{"IN",  TOK_IN },
	{"OUT", TOK_OUT},
	{"IF", TOK_IF  },
	{"WHILE",TOK_WHILE},
	{"",    TOK_VAR},
	{";",   TOK_SMC},
};

const Command TokArr[] =
{
	{"nil", EMPTY  },  
	{"",    TOK_NUM},
	{"IN",  TOK_IN },
	{"OUT", TOK_OUT},
	{"SIN", TOK_SIN},
	{"COS", TOK_COS},
	{"TG",  TOK_TG },
	{"CTG", TOK_CTG},
	{"LN",  TOK_LN },
	{"EXP", TOK_EXP},
	{"+",   TOK_ADD},
	{"-",   TOK_SUB},
	{"*",   TOK_MUL},
	{"/",   TOK_DIV},
	{"^",   TOK_POW},
	{";",   TOK_SMC},
	{"=",   TOK_ASN},
	{"==",  TOK_EQU},
	{">",   TOK_ABV},
	{"<",   TOK_BLW},
	{"=!", TOK_NEQU},
	{"=>", TOK_EABV},
	{"=<", TOK_EBLW},
	{"IF",   TOK_IF},
	{"WHILE",TOK_WHILE},
	{"",    TOK_VAR},
	{"ret", TOK_RET},
	{"", TOK_OWNFNC},
	{"",   TOK_1ARG},
	{"",   TOK_2ARG},
	{"",   TOK_3ARG},
	{"",   TOK_4ARG},
};
const int TokCnt = sizeof(TokArr) / sizeof(TokArr[0]);

const Tokens ConstrList[] = {TOK_ASN, TOK_WHILE, TOK_IF, TOK_SMC, TOK_VAR, TOK_OWNFNC, TOK_1ARG, TOK_2ARG, TOK_3ARG, TOK_4ARG};
const int ConstrSize = sizeof(ConstrList) / sizeof(ConstrList[0]);

Node* CreateNode(Tokens tok, Node* left, Node* right);

int TreeDtor(Node *node);

int Backend(const char* file_from, const char* file_to);

int CompilerCtor(Compiler* cmp, const char* file_from, const char* file_to);

int CompilerDtor(Compiler* cmp);

int ReadTree(Node** node, FILE* fp);

int WriteAsm(Compiler* cmp);

int WriteCommand(Node* node, FILE* fp);

void SyntaxError();

int InConstrList(int tok);

int WriteEquation(Node* node, FILE* fp);

int WriteAssigment(Node* node, FILE* fp);

//int WriteDefineVariable(Node* node, FILE* fp);

int WriteDefineFunction(Node* node, FILE* fp);

int WriteBody(Node* node, FILE* fp);

int WriteWhile(Node* node, FILE* fp);

int WriteIf(Node* node, FILE* fp);

int ArgListNumber(char* s);
#define SYSEVAL(node) \
if (!strcmp(node->left->un.name, "eval:"))\
{\
	puts("SYSEVAL");\
	switch ((Tokens) node->right->tok)\
    {\
		case TOK_IN:\
		{\
			if (node->right->right->tok == (int) TOK_VAR)\
			{\
				int i = 0;\
				fprintf(fp, "in\n");\
				i = ArgListNumber(node->right->right->un.name);\
				if(i == -1) \
				{\
					i = argCnt;\
					strcpy( argList[argCnt], node->right->right->un.name);\
					printf("SYSEVAL wrote argument %s\n", argList[argCnt]);\
					argCnt++;\
				}\
				if (isMain) fprintf(fp, "vpop [%d]\n", i);\
				else fprintf(fp, "relpop [%d]\n", i);\
				return 0;\
			}\
		\
			else\
			{\
				printf("Syntax error: input type is not variable\n");\
				return -1;										\
			}\
			break;\
		}\
		case TOK_OUT:\
		{\
			WriteEquation(node->right->right, fp);\
			fprintf(fp, "out\n");\
			fprintf(fp, "pop\n");\
			return 0;\
		}\
\
		case TOK_RET:\
		{\
			puts("245h");\
			int retArgs = 0;\
			Node* retparam = node->right->right;\
			while (retparam != NULL)\
			{\
				WriteEquation(retparam, fp);\
				retArgs++;\
				retparam = retparam->right;\
				if(retArgs!= 1)\
				{\
					printf("warning from WriteEquation: malfunctioning ret encountered (%d)\n", retArgs);\
				}\
			}\
			if(!isMain)\
			{\
				fprintf(fp, "rpop ax\n");\
				fprintf(fp, "ret\n");\
			}\
			else fprintf(fp, "hlt\n");\
			return 0;\
		}\
	}\
}\
					


#endif