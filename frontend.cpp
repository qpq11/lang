//this .cpp is meant to create the tree for a program file
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "frontend.h"
#include "diff.h"
#include "graph.h"

static const int op_add 	= ADD;
static const int op_sub 	= SUB;
static const int op_mul 	= MUL;
static const int op_div 	= DIV;
static const int op_pow 	= POW;
static const int sg_smc 	= SMC;
static const int sg_asn 	= ASN;
static const int sg_equ 	= EQU;
static const int sg_abv     = ABV;
static const int sg_blw     = BLW;
static const char* lx_if    = (char*) "if";
static const char* lx_while = (char*) "while";
static const char* lx_in    = (char*) "in";
static const char* lx_out   = (char*) "out";
static const char* lx_ret   = (char*) "ret";
//

int FileSize(FILE* fp)
{
	if (fp ==  nullptr)
		return -1;
	
	int start = ftell(fp);
	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	fseek(fp, start, SEEK_SET);
	printf("from fnc FileSize: file size is %d\n", len);
	return len;
}

int ReadFile(Node** root, const char* filename)
{
	*root = (Node*) calloc(sizeof(Node), 1);
	FILE* fp = fopen(filename, "r");
	int fpSize = FileSize(fp);
	
	char* data = (char*) calloc(sizeof(char), fpSize + 1);
	int dataSize = (int) fread(data, sizeof(char), fpSize, fp);
	printf("from fnc ReadFile: read %d symbols(%d id file size)\n", dataSize, fpSize);
	//assert(dataSize == fpSize);
	fclose(fp);
	*root = GetG(data);
	puts("53");
	IfWhileFinal(*root);
	MainOnly(*root);
	if (*root == NULL)
	{
		puts("Warning from fnc ReadFile: created tree remained empty");
		return -1;
	}
	return 1;
}

void IfWhileFinal(Node* node)
{
	if (node != NULL && node->type != NIL)
	{
		if((node->un.op == SMC) && (node->right != NULL) && (node->right->un.op == SMC) && (node->right->right == NULL) && (node->right->left->type == FUNCNAME))
		{
			puts("65");
			memcpy(node->right, node->right->left, sizeof(Node));
		}
		IfWhileFinal(node->left);
		IfWhileFinal(node->right);
	}
	return;
}

void MainOnly(Node* node)
{
	if(node->un.op == SMC && node->left->type == FUNCNAME && node->right == NULL) memcpy(node, node->left, sizeof(Node));
	return;
}

void SymbSkip(char** s)
{
	while(**s == ' ' || **s == '\n' || **s == '\r' || **s == '\t')
		*s += 1;
}

Node* GetG(char* str)
{
	//puts("g");
	char** s = (char**) &str;
	//s = str;
	SymbSkip(s);
	//Node* val = GetEq(s);
	Node* val = GetFnc(s);
	SymbSkip(s);
	puts("89");
	REQUIRE('$');
	return val;
}

Node* GetFnc(char** s)
{
	printf("GetFnc: remaining file read is now %s \n", *s);
	char* oldS = *s;
	if (**s != '$')
	{
		int nameLen = 0;
		char name[maxLen] = "";
		SymbSkip(s);
		while(('a' <= **s && 'z' >= **s) || ('A' <= **s && 'Z' >= **s))
		{
			printf("%c \n", **s);
			name[nameLen] = **s;
			puts("1");
			*s += 1;
			nameLen++;
		}
		printf("from fnc GetFnc: %s %d \n", name, nameLen);
		SymbSkip(s);
		if (!nameLen)
		{
			puts("no func here");
			return NULL;
		}
		
		if (**s == '(') 
		{
			*s += 1;
		}
		else 
		{
			*s = oldS;
			printf("GetFnc: remaining file read is now %s \n", *s);
			return NULL;
		}
		SymbSkip(s);
		Node* argNode = NULL;
		if(**s != ')')
		{
			puts("126");
			argNode = GetFncarg(s);
			if (argNode == NULL) 
				return NULL;
			REQUIRE(')');
		}
		else
		{
			*s += 1;
		}
		SymbSkip(s);
		int fncArgCnt = 0;
		if ('0' <= **s && **s <= '9') 
		{
			printf("%c\n", **s);
			fncArgCnt = **s - '0';
			*s += 1;
		}
		printf("%s is supposed to take %d arguments\n", name, fncArgCnt);
		if (fncArgCnt >= 4) puts("WARNING: proc doesn't have that much registers");
		SymbSkip(s);
		
		if (**s == '{') 
		{
			puts("start reading body");
			*s += 1;
		}
		else if (**s == ';')
		{
			Node* res = CreateNode(FUNCNAME, name, NULL, argNode);
			if(!strcmp(name, lx_in)) res->tok = TOK_IN;
			else if(!strcmp(name, lx_out)) res->tok = TOK_OUT;
			else if(!strcmp(name, lx_ret)) res->tok = TOK_RET;
			else res->tok = TOK_OWNFNC;
			return res;
		}
		else
		{
			*s = oldS;
			free(argNode);
			argNode = NULL;
			printf("GetFnc: remaining file read is now %s \n", *s);
			return NULL;
		}
		SymbSkip(s);
		Node* bodyNode = GetBody(s);
		//puts("body read");
		REQUIRE('}');
		puts("function read fully");
		Node* res = CreateNode(OP, &sg_smc, CreateNode(FUNCNAME, name, bodyNode, argNode), GetFnc(s));
		puts("node created");
		res->left->tok = (Tokens) (fncArgCnt + (int) TOK_OWNFNC);
		res->tok = TOK_SMC;
		
		return res;
	}
	puts("end of file");
	return NULL;
}

Node* GetFncarg (char** s)
{
    Node* arg = GetE(s);
	puts("190");
	if (arg == NULL || (**s != ',' && **s != ')')) return NULL;
	puts("177");
    if (arg != NULL && arg->type != NIL)
        arg->right = NULLCR();
	//printf("from fnc GetFncarg: remaining fileread is %s\n", *s);
    if (**s == ',' && arg->type != OP)
    {
        *s += 1;
        arg->left = GetFncarg(s);
    }
    return arg;
}

Node* GetBody(char** s)
{
	CONSTRGEN("if", lx_if);
	CONSTRGEN("while", lx_while);
	
	return GetEq(s);
}

Node* GetEq(char** s)
{
	if (**s == '$') return NULL;
	
	Node* val = GetAs(s);
	SymbSkip(s);
	
	while (**s == ';')
	{
		*s += 1;
		SymbSkip(s);
		Node* val2 = GetBody(s);
		if (val2 == NULL)
		{
			puts("from fnc GetEq: GetBody returned null");
			val2 = NULLCR();
		}
		// printf("$$ NODE: %lf %c %c$$\n", val->un.value, val->un.op, val->un.name);
		// printf("$$ NODE: %lf %c %c$$\n", val2->un.value, val2->un.op, val2->un.name);
		// printf("from fnc GetEq: ready to create a semicolon node\n");
		SymbSkip(s);
		val = CreateNode(OP, &sg_smc, val, val2);
		val->tok = TOK_SMC;
		// printf("$$$ NODE: %lf %c %c$$\n", val->un.value, val->un.op, val->un.name);
		// printf("$$$ LNODE: %lf %c %c$$\n", val->left->un.value, val->left->un.op, val->left->un.name);
		// printf("$$$ RNODE: %lf %c %c$$\n", val->right->un.value, val->right->un.op, val->right->un.name);
	}
	SymbSkip(s);
	return val;
}

Node* GetAs(char** s)
{
	Node* val = GetN(s);
	if (val == NULL)
	{
		puts("Warning in fnc GetAs: GetE failed to operate");
		return NULL;
	}
	SymbSkip(s);
	
	// char op = **s;
	// *s += 1;
	// if (op == '=') && **s == '=')
	// {
		// op = EQU;
		// *s += 1;
	// }
	
	char op = **s;
	*s += 1;
	Tokens tk = EMPTY;
	if (op == '=')
	{
		tk = TOK_ASN;
		switch(**s)
		{
			case '=':
			{
				tk = TOK_EQU;
				*s += 1;
				break;
			}
			case '!':
			{
				tk = TOK_NEQU;
				*s += 1;
				break;
			}
			case '>':
			{
				tk = TOK_EABV;
				*s += 1;
				break;
			}
			case '<':
			{
				tk = TOK_EBLW;
				*s += 1;
				break;
			}
		}		
	}
	else if (op == '>') tk = TOK_ABV;
	else if (op == '<') tk = TOK_BLW;
	
	SymbSkip(s);
	Node* val2 = GetE(s);
	if (val2 == NULL)
	{
		puts("Warning in fnc GetAs: GetE failed to operate");
		return NULL;
	}
	printf("from fnc GetAs: left and right subs were created, op is %c \n", op);
	printf("from fnc GetAs: got token %d\n", tk);
	switch(tk)
	{
		case TOK_ASN:
		{
			val = CreateNode(OP, &sg_asn, val, val2);
			break;
		}
		case TOK_EQU: case TOK_NEQU: case TOK_EABV: case TOK_EBLW:
		{
			val = CreateNode(OP, &sg_equ, val, val2);
			break;
		}
		case TOK_ABV:
		{
			val = CreateNode(OP, &sg_abv, val, val2);
			break;
		}
		case TOK_BLW:
		{
			val = CreateNode(OP, &sg_blw, val, val2);
			break;
		}
	}
	val->tok = tk;
	printf("GetAs resulted with node %d %c\n", val->tok, val->un.op);
	return val;
}

Node* GetE(char** s)
{
	//puts("e");
	Node* val = GetT(s);
	SymbSkip(s);
	while (**s == '+' || **s == '-')
	{
		char op = **s;
		*s += 1;
		SymbSkip(s);
		Node* val2 = GetT(s);
		val = CreateNode(OP, &op, val, val2);
		if(op == ADD) val->tok = TOK_ADD;
		else val->tok = TOK_SUB;
		SymbSkip(s);
	}
	//printf("%d\n", val);
	return val;
}

Node* GetT(char** s)
{
	//puts("t");
	Node* val = GetPw(s);
	SymbSkip(s);
	while (**s == '*' || **s == '/')
	{
		char op = **s;
		//printf("%c \n", *s);
		*s += 1;
		SymbSkip(s);
		Node* val2 = GetPw(s);
		val = CreateNode(OP, &op, val, val2);
		if(op == MUL) val->tok = TOK_MUL;
		else val->tok = TOK_DIV;
		SymbSkip(s);
	}
	return val;
}

Node* GetPw(char** s)
{
	//puts("PW");
	Node* val = GetFn(s);
	SymbSkip(s);
	while (**s == '^')
	{
		char op = **s;
		*s += 1;
		SymbSkip(s);
		REQUIRE('(');
		*s -= 1;
		SymbSkip(s);
		Node* val2 = GetFn(s);
		SymbSkip(s);
		//printf("157: %s\n", *s);
		val = CreateNode(OP, &op, val, val2);
		val->tok = TOK_POW;
	}
	return val;
}

Node* GetFn(char** s)
{
	//printf("FN %c\n", **s);
    char buf[maxLen] = "";
	int readSymb = 0;
    sscanf(*s, "%5[^ +-*/(\n]%n", buf, &readSymb);
	char upcasebuf[maxLen] = "";
	for(int i = 0; i < readSymb; i++) //why do that: check Fncs[]
	{
		upcasebuf[i] = toupper(buf[i]);
	}
	printf("from fnc GetFn: %s is to be checked on being a fucktion name\n", buf);
    if ('A' <= upcasebuf[0] && upcasebuf[0] <= 'Z')
	{
		for (int i = 0; i < FncCnt; i++)
		{
			printf("# %s$ %s$ %d\n", upcasebuf, Fncs[i], strcmp(buf, Fncs[i]));
			if(!strcmp(upcasebuf, Fncs[i]))
			{
				*s += readSymb;
				SymbSkip(s);
				printf("fn %c\n", **s);
				Node* val = GetP(s);
				SymbSkip(s);
				val->tok = TokFnc[i].tok;
				val = CreateNode(FNC, &i, NULLCR(), val);
				return val;
			}
		}
	}
	
	printf("from fnc GetFn: %s is not a function name\n", buf);
	CONSTRGEN("if", lx_if);
	CONSTRGEN("while", lx_while);
	Node* val = GetFnc(s);
	if (val == NULL) 
		val = GetP(s);		
	SymbSkip(s);
	return val;
}

Node* GetP(char** s)
{
	printf("p %c\n", **s);
	SymbSkip(s);
    if (**s == '(')
    {
        if (**s == '(') *s += 1;
        Node* val = GetE(s);
		SymbSkip(s);
		//printf("209: %s\n", *s);
        REQUIRE(')');
		//printf("211: %s\n", *s);
        return val;
    }
	Node* val = GetN(s);
	SymbSkip(s);
	return val;
}

Node* GetN(char** s)
{
	SymbSkip(s);
	if (**s == '\0' || **s == ')' || **s == '}') return NULL;
	printf("n %c\n", **s);
	char* oldS = *s;
	
	if (('a' <= **s && 'z' >= **s) || ('A' <= **s && 'Z' >= **s))
	{
		printf("from fnc GetN: found variable name ");
		char buf[maxLen] = "";
		int readSymb = 0;
		sscanf(*s, "%5[^, ^=;+-*/()]%n", buf, &readSymb);
		printf("%s\n", buf);
		Node* val = CreateNode(VAR, buf, NULL, NULL);
		val->tok = TOK_VAR;
		*s += readSymb;
		SymbSkip(s);
		printf("  remaining read is {%s\n}", *s);
		return val;
	}
	
	char sign = '+';
	if (**s == '-')
	{
		sign = '-';
		//printf("min\n");
		*s += 1;
		SymbSkip(s);
	}
	else if (**s == '+')
	{
		*s += 1;
		SymbSkip(s);
	}
	
	if (sign == '-') 
	{
		if (**s == '(' || isdigit(**s))
		{
			Node* expr = GetP(s);
			Node* res = CRMUL( CREATE(-1.0), expr);
			res->tok = TOK_MUL;
			return res;
		}
		if ('a' <= **s && 'z' >= **s)
		{
			Node* expr = GetN(s);
			Node* res = CRMUL( CREATE(-1.0), expr);
			res->tok = TOK_MUL;
			return res;
		}
	}
	
	Node* node = CREATE(0.0);
	
	while('0' <= **s && **s <= '9')
	{
		if (sign == '+')
		{
			node->un.value = node->un.value * 10 + (**s - '0');
			*s += 1;
		}
		else
		{
			node->un.value = node->un.value * 10 - (**s - '0');
			*s += 1;
		}
	}
	
	if(**s == '.')
	{
		*s += 1;
		printf(">> %c\n", **s);
		SymbSkip(s);
		double dec = 10.0;
		while('0' <= **s && **s <= '9')
		{
			if (sign == '+')
			{
				node->un.value = node->un.value  + (**s - '0') / dec;
				dec *= 10.0;
				*s += 1;
			}
			else
			{
				node->un.value = node->un.value  - (**s - '0') / dec;
				dec *= 10.0;
				*s += 1;
			}
		}
	}
	if (oldS == *s)
	{
		free(node);
		node = NULL;
		puts("GetN returns null");
		//SyntaxError();
		return NULL;
	}
	printf("fnc GetN set node value = %lf\n", node->un.value);
	printf("%s\n", *s);
	node->tok = TOK_NUM;
	return node;
}

void SyntaxError()
{
	printf("Syntax error occured. Line: %d\n", __LINE__);
	assert(0);
}

void TreePrint(struct Node* pnode, FILE* fpr)
{
	assert(fpr != NULL);
	if (pnode == NULL || pnode->type == NIL)
	{
		fprintf(fpr, "_ ");
		return;
	}
	fprintf(fpr, "{");
	switch (pnode->type)
	{
		case NUM:
		{
			fprintf(fpr, "%d %lf ", TOK_NUM, pnode->un.value);
			break;
		}
		case VAR:
		{
			fprintf(fpr, "%d %s ", TOK_VAR, pnode->un.name);
			break;
		}
		case FNC:
		{
			fprintf(fpr, "%d %s ", (int) TOK_SIN + (int) pnode->un.value, Fncs[(int) pnode->un.value]);
			break;
		}
		case OP:
		{
			fprintf(fpr, "%d %s ", pnode->tok, TokArr[pnode->tok].name);
			break;
		}
		case FUNCNAME:
		{
			fprintf(fpr, "%d %s ", (pnode->tok != EMPTY) ? pnode->tok : TOK_OWNFNC, pnode->un.name);
			break;
		}
	}
	TreePrint(pnode->left,  fpr);
	TreePrint(pnode->right, fpr);
	fprintf(fpr, "}");
	return;
}
	// //printf("address: %d\n", &pnode);
	// if (pnode != NULL && pnode->type != FUNCNAME && pnode->type != NIL)
	// {
		// puts("4");
		// if (pnode != NULL && pnode->type != NIL)
		// {
			// //fprintf(fpr, "(");
			// puts("5");
			// TreePrint(pnode->left, fpr);
		// }
		// else
		// {
			// puts("6");
			// fprintf(fpr, " ");
			// return;
		// }
		
		// if (pnode->type == NUM)
		// {
			// //puts("2");
			// printf("@%lf \n", pnode->un.value);
			// fprintf(fpr, "%lf", pnode->un.value);
		// }
		// else if (pnode->type == OP || pnode->type == NIL)
		// {
			// if(!(pnode->un.op == SMC && (pnode->left->type == FUNCNAME || pnode->right->type == FUNCNAME)))
			// {
				// printf("@%c \n", pnode->un.op    );
				// fprintf(fpr, "%c" , pnode->un.op   );
				// if(pnode->un.op == SMC)
					// fprintf(fpr, "\n");
			// }
			// if (pnode->un.op == SMC && pnode->left->type != FUNCNAME && pnode->right->type == FUNCNAME)
			// {
				// printf("@%c \n", pnode->un.op    );
				// fprintf(fpr, "%c" , pnode->un.op   );
				// if(pnode->un.op == SMC)
					// fprintf(fpr, "\n");
			// }
		// }
		// else if (pnode->type == VAR )
		// {
			// //puts("3");
			// printf("#%s \n", pnode->un.name    );
			// fprintf(fpr, "%s" , pnode->un.name   );
		// }
		
		// else if (pnode->type == FNC)
		// {
			// printf("@%s \n", Fncs[(int) pnode->un.value]);
			// fprintf(fpr, "%s(" , Fncs[(int) pnode->un.value]);
		// }
		
		// if (pnode != NULL)
		// {
			// TreePrint(pnode->right, fpr);
			// if(pnode->type == FNC)
				// fprintf(fpr, ")");
		// }
		// else
		// {
			// fprintf(fpr, " ");
			// return;
		// }
	// }
	// else if (pnode != NULL && pnode->type != NIL && (strcmp(pnode->un.name, lx_if)!= 0) && (strcmp(pnode->un.name, lx_while) != 0))
	// {
		// printf("#%s\n", pnode->un.name);
		// fprintf(fpr, "%s(", pnode->un.name);
		// //puts("1");
		// if (pnode->right != NULL && pnode->right->type != NIL)
		// {
			// ArgPrint(pnode->right, fpr);
		// }
		// fprintf(fpr, ")\n{\n");
		// TreePrint(pnode->left, fpr);
		// //puts("3");
		// fprintf(fpr, "}\n\n");
	// }
	// else if (pnode != NULL && pnode->type != NIL && (!strcmp(pnode->un.name, lx_if) || !strcmp(pnode->un.name, lx_while)))
	// {
		// printf("#%s\n", pnode->un.name);
		// fprintf(fpr, "%s(", pnode->un.name);
		// //puts("1");
		// if (pnode->left != NULL && pnode->left->type != NIL)
		// {
			// //puts("2");
			// TreePrint(pnode->left, fpr);
		// }
		// fprintf(fpr, ")\n{\n");
		// TreePrint(pnode->right, fpr);
		// //puts("3");
		// fprintf(fpr, "} \n");
	// }
	// return;
// }

void ArgPrint(Node* node, FILE* fpr)
{
	if (node != NULL)
	{
		switch(node->type)
		{
			case NUM:
			{
				printf("@%lf \n", node->un.value);
				fprintf(fpr, "%lf", node->un.value);
				break;
			}
			case VAR:
			{
				printf("#%s \n", node->un.name);
				fprintf(fpr, "%s" , node->un.name);
			}
		}
		if(node->left != NULL)
		{
			fprintf(fpr, ", ");
			ArgPrint(node->left, fpr);
		}
	}
	return;
}

void TreeDtor(Node *node)
{
    if (node->left  != NULL)  { TreeDtor(node->left); }

    if (node->right != NULL) { TreeDtor(node->right); }

    free(node->un.name);
	node->un.name = NULL;
    free(node);
	node = NULL;  // !!!!!!!!!!!!!!!!!!!!
}