#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "rec.h"
#include "diff.h"
#include "graph.h"

static const int op_add = ADD;
static const int op_sub = SUB;
static const int op_mul = MUL;
static const int op_div = DIV;
static const int op_pow = POW;
static const int sg_smc = SMC;
static const int sg_asn = ASN;
//
static const int fn_sin = SIN;
static const int fn_cos = COS;
static const int fn_ln  = LN;
static const int fn_exp = EXP;


// int main()
// {
	// FILE* fp = fopen("text.txt", "r");
	// FILE *fpr= fopen("dtorout.txt", "w+");
	// char* reads = (char*) calloc(sizeof(char), (maxLen + 1));
	// assert(reads != nullptr);
	// const int len = FileGet(reads, fp);
	// Node* res= GetG(reads);
	// free(reads);
	// puts("\n\nstart of tree print\n-------------");
	// TreePrint(res, fpr);
	// fprintf(fpr, "\n");
	// puts("---");
	
	// res= Simpify(res);
	// TreePrint(res, fpr);
	// fprintf(fpr, "\n");
	// puts("---");
	
	// Node* diffroot = Diff(res);
	// TreePrint(diffroot, fpr);
	// fprintf(fpr, "\n");
	// puts("---");
	// diffroot = Simpify(diffroot);
	// //printf("%c %c %c\n", diffroot->un.op, diffroot->left->un.op, diffroot->right->un.op);
	// TreePrint(diffroot, fpr);
	
	// // Simpify(res);
	// // TreePrint(res, fpr);
	
	// puts("\nend of tree print");
	// fprintf(fpr, "\n");
	// fclose(fp);
	// fclose(fpr);
	// Graphviz(diffroot, "resfile.dot");
	// //free(diffroot);
	// //free(res);
	// TreeDtor(res);
	// res = NULL;
	// TreeDtor(diffroot);
	// diffroot = NULL;
	// //printf("%d \n", res);
	// return 0;
// }

int FileGet(char* reads, FILE *fp)
{
	
	int len  = 0;
	int ch   = 0;
	
	fscanf(fp, "%s%n", reads, &len);
	printf("%d symbols in file\n", len);
	return len;
}

void SyntaxError()
{
	printf("Syntax Error.\n");
	return;
}

Node* GetG(char* str)
{
	//puts("g");
	char** s = (char**) &str;
	//s = str;
	SPACESKIP(s);
	Node* val = GetE(s);
	Require('$');
	return val;
}

// Node* GetSmc(char** s)
// {
	// if (**s == ';') return NULL;
	
	// Node* val = GetAs(s);
	
	// if (**s == ';')
	// {
		// *s += 1;
		// SPACESKIP(s);
		// val = CreateNode(OP, &sg_smc, val, GetSmc(s));
	// }
	// SPACESKIP(s);
	// return val;
// }

// Node* GetAs(char** s)
// {
	// Node* val = GetE(s);
	// SPACESKIP(s);
	
	// if (**s == '=')
	// {
		// *s += 1;
		// SPACESKIP(s);
		// Node* 
	// }
// }

Node* GetE(char** s)
{
	//puts("e");
	SPACESKIP(s);
	Node* val = GetT(s);
	while (**s == '+' || **s == '-')
	{
		char op = **s;
		*s += 1;
		SPACESKIP(s);
		Node* val2 = GetT(s);
		val = CreateNode(OP, &op, val, val2);
	}
	//printf("%d\n", val);
	return val;
}

Node* GetT(char** s)
{
	//puts("t");
	Node* val = GetPw(s);
	while (**s == '*' || **s == '/')
	{
		char op = **s;
		//printf("%c \n", *s);
		*s += 1;
		SPACESKIP(s);
		Node* val2 = GetPw(s);
		// if (op == '*') val = CreateNode(OP, &op_mul, val, val2);
		// else           val = CreateNode(OP, &op_div, val, val2);
		val = CreateNode(OP, &op, val, val2); 
	}
	return val;
}

Node* GetPw(char** s)
{
	puts("PW");
	Node* val = GetFn(s);
	while (**s == '^')
	{
		char op = **s;
		*s += 1;
		SPACESKIP(s);
		Require('(');
		*s -= 1;
		SPACESKIP(s);
		Node* val2 = GetFn(s);
		printf("125: %s\n", *s);
		val = CreateNode(OP, &op, val, val2);
	}
	return val;
}

Node* GetFn(char** s)
{
	//printf("FN %c\n", **s);
    char buf[maxLen] = "";
	int readSymb = 0;
    sscanf(*s, "%5[^ +-*/(]%n", buf, &readSymb);
    if ('A' <= buf[0] && buf[0] <= 'Z')
	{
		for (int i = 0; i < FncCnt; i++)
		{
			//printf("# %s$ %s$ %d\n", buf, Fncs[i], strcmp(buf, Fncs[i]));
			if(!strcmp(buf, Fncs[i]))
			{
				*s += readSymb;
				SPACESKIP(s);
				//printf("fn %c\n", **s);
				Node* val = GetP(s);
				//printf("%s\n", *s);
				//printf("@fn %c\n", **s);
				//printf("153: %s\n", *s);
				val = CreateNode(FNC, &i, NULLCR(), val);
				return val;
			}
		}
	}
	else 
	{
		Node* val = GetP(s);
		return val;
	}
}

void SPACESKIP(char** s)
{
	while (**s == ' ')
		*s += 1;
}

Node* GetP(char** s)
{
	printf("p %c\n", **s);
    if (**s == '(')
    {
        if (**s == '(') *s += 1;
        Node* val = GetE(s);
		printf("179: %s\n", *s);
        Require(')');
		printf("181: %s\n", *s);
        return val;
    }
	Node* val = GetN(s);
	return val;
}

Node* GetN(char** s)
{
    //Node* node = calloc(1, sizeof(Node));
	//node-> type = NUM;
	printf("n %c\n", **s);
	char* oldS = *s;
	
	if('a' <= **s && 'z' >= **s)
	{
		Node* val = CreateNode(VAR, *s, NULL, NULL);
		*s += 1;
		SPACESKIP(s);
		printf("n %c\n", **s);
		return val;
	}
	
	char sign = '+';
	if (**s == '-')
	{
		sign = '-';
		//printf("min\n");
		*s += 1;
		SPACESKIP(s);
	}
	else if (**s == '+')
	{
		*s += 1;
		SPACESKIP(s);
	}
	
	if (sign == '-') 
	{
		if (**s == '(' || isdigit(**s))
		{
			Node* expr = GetP(s);
			return CRMUL( CREATE(-1.0), expr);
		}
		if ('a' <= **s && 'z' >= **s)
		{
			Node* expr = GetN(s);
			return CRMUL( CREATE(-1.0), expr);
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
		int dec = 10;
		while('0' <= **s && **s <= '9')
		{
			if (sign == '+')
			{
				node->un.value = node->un.value  + (**s - '0') / dec;
				dec *= 10;
				*s += 1;
			}
			else
			{
				node->un.value = node->un.value  - (**s - '0') / dec;
				dec *= 10;
				*s += 1;
			}
		}
	}
	if (oldS == *s)
	{
		free(node);
		SyntaxError();
	}
	printf("n%lf\n", node->un.value);
	printf("%s\n", *s);
	return node;
}

void TreePrint(struct Node* pnode, FILE* fpr)
{
	
	if (pnode != NULL)
	{
		fprintf(fpr, "(");
		TreePrint(pnode->left, fpr);
	}
	else
	{
		fprintf(fpr, " ");
		return;
	}
	
	//printf("$$ CURRENT NODE: %lf %c $$\n", pnode->un.value, pnode->un.op);
	// if (pnode->un.op == '^')
		// printf("$$ LEFT NODE: %lf %c $$\n", pnode->left->un.value, pnode->left->un.op);
	
	if (pnode->type == NUM)
	{
		//puts("2");
		printf("@%lf \n", pnode->un.value);
		fprintf(fpr, "%lf", pnode->un.value);
	}
	else if (pnode->type == OP || pnode->type == NIL)
	{
		printf("@%c \n", pnode->un.op    );
		fprintf(fpr, "%c" , pnode->un.op   );
	}
	else if (pnode->type == VAR)
	{
		//puts("3");
		printf("2%c \n", pnode->un.var    );
		fprintf(fpr, "%c" , pnode->un.var   );
	}
	//printf("\n\n$$ CURRENT NODE: %lf %c $$\n\n\n", pnode->un.value, pnode->un.op);
	else if (pnode->type == FNC)
	{
		printf("@%s \n", Fncs[(int) pnode->un.value]);
		fprintf(fpr, "%s" , Fncs[(int) pnode->un.value]);
	}
	
	if (pnode != NULL)
	{
		TreePrint(pnode->right, fpr);
		fprintf(fpr, ")");
	}
	else
	{
		fprintf(fpr, " ");
		return;
	}
	return;
	//printf("done printing\n");
}

void TreeDtor(Node *node)
{
    if (node->left  != NULL)  { TreeDtor(node->left); }

    if (node->right != NULL) { TreeDtor(node->right); }

    
    free(node);
	node = NULL;  // !!!!!!!!!!!!!!!!!!!!
}