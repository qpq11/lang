#ifndef FRONTEND_H
#define FRONTEND_H

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

const Command TokFnc[] =
{
	{"SIN", TOK_SIN},
	{"COS", TOK_COS},
	{"TG",  TOK_TG },
	{"CTG", TOK_CTG},
	{"LN",  TOK_LN },
	{"EXP", TOK_EXP},
};

const Command TokLgc[] =
{
	{"=",   TOK_ASN},
	{"==",  TOK_EQU},
	{">",   TOK_ABV},
	{"<",   TOK_BLW},
	{"=!", TOK_NEQU},
	{"=>", TOK_EABV},
	{"=<", TOK_EBLW},
};

const Command TokOp[] =
{
	{"+",   TOK_ADD},
	{"-",   TOK_SUB},
	{"*",   TOK_MUL},
	{"/",   TOK_DIV},
	{"^",   TOK_POW},
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
	{"RET", TOK_RET},
	{"", TOK_OWNFNC},
	{"",   TOK_1ARG},
	{"",   TOK_2ARG},
	{"",   TOK_3ARG},
	{"",   TOK_4ARG},
};
const int TokCnt = sizeof(TokArr) / sizeof(TokArr[0]);

const int maxLen = 9;

typedef union 
	{
		unsigned char op;
		char* name;
		double value;
	}nodeUnion;

struct Node
{
	int type = 0;
	nodeUnion un;
	struct Node*  left    = NULL;
	struct Node*  right   = NULL;
	int order = 0;
	Tokens tok = EMPTY;
};

void SyntaxError();

#define REQUIRE(ch) \
if (**s == ch) *s += 1; \
else SyntaxError();

void IfWhileFinal(Node* node); 

void MainOnly(Node* node);

#define CONSTRGEN(str, lex) \
if (!strncmp(str, *s, strlen(str))) \
{ \
		*s += strlen(str);\
		SymbSkip(s);\
		printf("CONSTRGEN %c", **s);\
		REQUIRE('(');\
		SymbSkip(s);\
		Node* condition = GetAs(s);\
		REQUIRE(')');\
		SymbSkip(s);\
		REQUIRE('{');\
		SymbSkip(s);\
		Node* instruction = GetBody(s);\
		REQUIRE('}');\
		printf("remaining fileread is now {%s}\n", *s);\
		printf("CONSTRGEN FINISHED\n");\
		SymbSkip(s); \
		Node* res = CreateNode(OP, &sg_smc, CreateNode(FUNCNAME, lex, condition, instruction), GetBody(s));\
		res->tok = TOK_SMC;\
		if (!strcmp(lex, lx_if)) res->left->tok = TOK_IF;\
		if (!strcmp(lex, lx_while)) res->left->tok = TOK_WHILE;\
		return res;\
}

/*Node* res = CreateNode(OP, &sg_smc, CreateNode(FUNCNAME, lex, NULL, argNode), GetBody(s));*/
int FileSize(FILE* fp);

int ReadFile(Node** root, const char* filename);

Node* GetFnc (char** s);

Node* GetFncarg (char** s);

Node* GetBody (char** s);

Node* GetEq(char** s);

Node* GetAs(char** s);

Node* GetE(char** s);

Node* GetN(char** s);

Node* GetP(char** s);

Node* GetT(char** s);

Node* GetG(char* str);

Node* GetPw(char** s);

Node* GetFn(char** s);

void TreePrint(struct Node* pnode, FILE* fpr);

void ArgPrint(Node* node, FILE* fpr);

const int maxAmount = 1000;

int FileGet(char* reads, FILE *fp);

void TreeDtor(struct Node* node);

void SymbSkip(char** s);

static const char* Fncs[] = {
	"SIN",
	"COS",
	"TG",
	"CTG",
	"LN",
	"EXP",
};

const int FncCnt = sizeof(Fncs)/sizeof(Fncs[0]);

#endif