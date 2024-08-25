#include "string.h"
#include "ctype.h"
#include "stdio.h"
#include "stdlib.h"
#include "assert.h"
#include "backend.h"

#define MAXCNT 100

static const int op_add = ADD;
static const int op_sub = SUB;
static const int op_mul = MUL;
static const int op_div = DIV;
static const int op_pow = POW;
static const int sg_smc = SMC;
static const int sg_asn = ASN;
static const int sg_equ = EQU;
static const int sg_abv = ABV;
static const int sg_blw = BLW;
static const char* lx_if = (char*) "if";
static const char* lx_while = (char*) "while";
//

// static char declFnc[MAXVARCNT][MAXLEXSIZE] = {};
// static int funcsCnt = 0;
// static char declVar[MAXFNCCNT][MAXLEXSIZE] = {};
// static int funcsCnt = 0;

int main(int argc, char *argv[])
{
	const char* file_from = nullptr;
	const char* file_to   = "asmtext.txt";

	if (argc < 2)
		{
		printf("Incorrect args number\n");
		return 1;
		}
	else if (argc == 2)
		{
		file_from = argv[1];
		}
	else if (argc > 2)
		{
		file_from = argv[1];
		file_to   = argv[2];
		}

	Backend(file_from, file_to);
	return 0;
}

int Backend(const char* file_from, const char* file_to)
{
	assert(file_from);
	assert(file_to);

	Compiler cmp = {};
	if (CompilerCtor(&cmp, file_from, file_to))
	{
		return -1;
	}

	if (ReadTree(&cmp.tree, cmp.file_from))
	{
		CompilerDtor(&cmp);
		return -2;
	}
	puts("----TREE READ SUCCESSFULLY----");
	WriteAsm(&cmp);
	//TreeDump(&cmp.tree);
	CompilerDtor(&cmp);

	return 0;
}

int CompilerCtor(Compiler* cmp, const char* file_from, const char* file_to)
{
    assert(cmp);
    assert(file_from);
    assert(file_to);

    cmp->file_from = fopen(file_from, "r+");
    if (cmp->file_from == NULL)
    {
        puts("Cannot open file\n");
        return -1;
    }

    cmp->file_to = fopen(file_to, "w+");
    if (cmp->file_to == NULL)
    {
        puts("Cannot open file\n");
        fclose(cmp->file_from);
        return -1;
    }

    cmp->tree = nullptr;

    return 0;
}

int CompilerDtor(Compiler* cmp)
{
    assert(cmp != NULL);

    fclose(cmp->file_from);
    fclose(cmp->file_to);

    int status = TreeDtor(cmp->tree);

    return status;
}

int TreeDtor(Node *node)
{
	if (node == NULL) return -1;
		
    if (node->left  != NULL)  TreeDtor(node->left);

    if (node->right != NULL) TreeDtor(node->right);

    free(node->un.name);
	node->un.name = NULL;
    free(node);
	node = NULL;
	return 0;
}

int ReadTree(Node** node, FILE* fp)
{
	assert(node != NULL);
	assert(fp != NULL);
	
	char c = ' ';
	SPACESKIP;
	
	switch(c)
	{
		case '_':
		{
			puts("empty node");
			return 0;
		}
		case '{': 
		{
			break;
		}
		case EOF:
		{
			puts("ReadTree reached EOF");
			return -1;
		}
		default:
		{
			return -2;
		}
	}
	Tokens nodeTok = EMPTY;
	fscanf(fp, "%d", &nodeTok);
	printf("ReadTree received %d\n", nodeTok);
	//printf("%d %d\n", TOK_IN <= nodeTok, nodeTok <= TOK_4ARG);
	SPACESKIP;
	
	double numBuf = 0.0;
	char txtBuf[MAXLEXSIZE] = "";
	
	//if (tok == EMPTY) return 0;
	if (nodeTok == TOK_NUM)
	{
		fscanf(fp, "%lf", &numBuf);
		// printf("%s\n", txtBuf);
		// numBuf = atof(txtBuf);
		printf("number: %lf\n", numBuf);
		*node = CreateNode(TOK_NUM, NULL, NULL);
		(*node)->un.value = numBuf;
		printf("value: %lf\n", (*node)->un.value);
	}
	else
	{
		fscanf(fp, "%s", txtBuf);
		printf("{%s}\n", txtBuf);
		if(TOK_IN <=  nodeTok && nodeTok <= TOK_4ARG)
		{
			*node = CreateNode(nodeTok, NULL, NULL);
			(*node)->un.name = (char*) calloc(strlen(txtBuf), sizeof(char));
			strcpy((*node)->un.name, txtBuf);
		}
		else 
		{
			printf("Failed to recognize the token: %d\n", nodeTok);
			return -3;
		}
	}
	puts("write left.");
	ReadTree(&(*node)->left, fp);
	puts("write right.");
	ReadTree(&(*node)->right, fp );
	//puts("1");
	// if(ReadTree(&(*node)->left, fp) || ReadTree(&(*node)->left, fp ))
	// {
		// puts("problems occured when descending to subnodes");
		// return -4;
	// }
	printf("Node created: %d %lf %s\n", (*node)->tok, (*node)->un.value, (((*node)->tok == TOK_NUM) ? "(num_node)" : (*node)->un.name));
	if ( (*node)->left != NULL)  printf("left node: %d %lf %s\n", (*node)->left->tok, (*node)->left->un.value, (((*node)->left->tok == TOK_NUM) ? "(num_node)" : (*node)->left->un.name));
	if ( (*node)->right != NULL) printf("right node: %d %lf %s\n", (*node)->right->tok, (*node)->right->un.value, (((*node)->right->tok == TOK_NUM) ? "(num_node)" : (*node)->right->un.name));
	
	c = ' ';
	SPACESKIP;
	
	if (c == '}')
	{
		puts("node end reached");
		return 0;
	}
	else
	{
		puts("from fnc ReadTree: closing bracket is missing");
		return -5;
	}
}

Node* CreateNode(Tokens tk, Node* left, Node* right)
{
	Node* newNode = (Node*)calloc(1, sizeof(Node));
	newNode->tok   = (int) tk;
	newNode->left  = left;
	newNode->right = right;
	return newNode;
}

int WriteAsm(Compiler* cmp)
{
    assert(cmp != nullptr);

    Node* command = cmp->tree;

    while (command != NULL && command->tok == TOK_SMC)
	{
		if (WriteCommand(command->left, cmp->file_to) != 0)
		{
			SyntaxError();
		}
		puts("\n");
		command = command->right;
		if(command == NULL) puts("NULL");
	}
	WriteCommand(command, cmp->file_to);
	fprintf(cmp->file_to, "hlt\n");
    return 0;
}

static int ifCnt = 0;
static int whileCnt = 0;
static int isMain = 0;
static int argCnt = 0;
static char argList[MAXCNT][MAXLEXSIZE+1];

int ArgListNumber(char* s)
{
	for (int i = 0; i < argCnt; i++)
	{
		if(!strcmp(s, argList[i]))
			return i;
	}
	return -1;
}

int WriteCommand(Node* node, FILE* fp)
{
    assert(node);
    assert(fp);

    //fprintf(fp, "\n");
	printf("Node: %d %lf %s\n", node->tok, node->un.value, node->un.name);
	if(node->left != NULL) printf("Left: %d %lf %s\n", node->left->tok, node->left->un.value, (node->left->tok == TOK_NUM) ? "(num_node)" : node->left->un.name);
	if(node->right != NULL) printf("Right: %d %lf %s\n", node->right->tok, node->right->un.value, (node->right->tok == TOK_NUM) ? "(num_node)" : node->right->un.name);
	
    if (InConstrList(node->tok) )
	{
        switch ((Tokens) node->tok)
        {
            case TOK_ASN:
            {
                return WriteAssigment(node, fp);
            }
            case TOK_WHILE:
            {
                whileCnt += 1;
                int n = WriteWhile(node, fp);
                fprintf(fp, ":whend%d\n", n);
                return 0;
            }
            case TOK_IF:
            {
                ifCnt += 1;
                int n = WriteIf(node, fp);
                fprintf(fp, ":ife%d\n", n);
                return 0;
            }
            case TOK_SMC:
            {
				printf("Semicolon: %d %lf %s\n", node->tok, node->un.value, node->un.name);
				if(node->left != NULL) printf("Smc left: %d %lf %s\n", node->left->tok, node->left->un.value, (node->left->tok == TOK_NUM) ? "(num_node)" : node->left->un.name);
				if(node->right != NULL) printf("Smc right: %d %lf %s\n", node->right->tok, node->right->un.value, (node->right->tok == TOK_NUM) ? "(num_node)" : node->right->un.name);
                return WriteBody(node, fp);
            }
            // case TOK_VAR:
            // {
                // return WriteDefineVariable(node, fp);
            // }
            case TOK_OWNFNC: case TOK_1ARG: case TOK_2ARG: case TOK_3ARG: case TOK_4ARG:
            {
                return WriteDefineFunction(node, fp);
            }
        }
	}
    int status = WriteEquation(node, fp);
    //fprintf(fp, "pop trash\n");
    return status;
}

int InConstrList(int tok)
{
	for (int i = 0; i < ConstrSize; i++)
	{
		if ((Tokens) tok == ConstrList[i])
			return 1;
	}
	return 0;
}

int WriteEquation(Node* node, FILE* fp)
{
    assert(node);
    assert(fp);

    switch ((Tokens) node->tok)
    {
        case TOK_NUM:
        {
            fprintf(fp, "push %lf\n", node->un.value);
            break;
        }
        case TOK_VAR:
        {
			int i = ArgListNumber(node->un.name);
			if(i == -1) 
			{
				i = argCnt;
				strcpy( argList[argCnt], node->un.name);
				printf("WEq wrote argument %s\n", argList[argCnt]);
				argCnt++;
			}
			if (isMain) fprintf(fp, "vpush [%d]\n", i);
			else fprintf(fp, "relpush [%d]\n", i);
            // if (isMain) fprintf(fp, "338vpush [%s]\n", node->un.name);
			// else fprintf(fp, "338vpush {%s}\n", node->un.name);
            break;
        }
        case TOK_OWNFNC: case TOK_1ARG: case TOK_2ARG: case TOK_3ARG: case TOK_4ARG:
        {
			//puts("335");
            Node* parametr = node->right;
            int   param_number = 0;
            while (parametr != NULL)
            {
				//puts("340");
                switch (parametr->tok)
                {
                    case TOK_NUM:
                        fprintf(fp, "push %lf\n", parametr->un.value);
                        break;
                    case TOK_VAR:
						//puts("347");
						int i = ArgListNumber(parametr->un.name);
						if(i == -1) 
						{
							i = argCnt;
							strcpy( argList[argCnt], parametr->un.name);
							printf("WEq wrote argument %s\n", argList[argCnt]);
							argCnt++;
						}
						if (isMain) fprintf(fp, "vpush [%d]\n", i);
						else fprintf(fp, "relpush [%d]\n", i);
                        // if (isMain) fprintf(fp, "356vpush [%s]\n", parametr->un.name);
						// else fprintf(fp, "356vpush {%s}\n", parametr->un.name);
						puts("349");
                        break;
                }
                fprintf(fp, "rpop %s\n", regList[param_number]);
				//fprintf(fp, "rpush %s\n", regList[param_number]);
				puts("352");
                parametr = parametr->right;
                param_number += 1;
            }
            fprintf(fp, "call :%s\n", node->un.name);
			//if (!strcmp(node->un.name, "main")) isMain = 0;
            fprintf(fp, "rpush ax\n");
            break;
        }
        case TOK_ABV:
		{
			WriteEquation(node->left, fp);
			WriteEquation(node->right, fp);
			fprintf(fp, "jnl ");
			break;
        }

		case TOK_BLW:
		{
			WriteEquation(node->left, fp);
			WriteEquation(node->right, fp);
			fprintf(fp, "jnm ");
			break;
		}

		case TOK_EABV:
		{
			WriteEquation(node->left, fp);
			WriteEquation(node->right, fp);
			fprintf(fp, "jl ");
			break;
		}

		case TOK_EBLW:
		{
			WriteEquation(node->left, fp);
			WriteEquation(node->right, fp);
			fprintf(fp, "jm ");
			break;
		}

		case TOK_EQU:
		{
			WriteEquation(node->left, fp);
			WriteEquation(node->right, fp);
			fprintf(fp, "jne ");
			break;
		}

		case TOK_NEQU:
		{
			WriteEquation(node->left, fp);
			if (node->right->tok == TOK_NUM && !node->right->un.value)
			{
				fprintf(fp, "jz ");
			}
			else
			{
				WriteEquation(node->right, fp);
				fprintf(fp, "je ");
			}
			break;
		}

		case TOK_ADD:
		{
			WriteEquation(node->left, fp);
			WriteEquation(node->right, fp);
			fprintf(fp, "add\n");
			break;
		}

		case TOK_SUB:
		{
			WriteEquation(node->left, fp);
			WriteEquation(node->right, fp);
			fprintf(fp, "sub\n");
			break;
		}

		case TOK_MUL:
		{
			WriteEquation(node->left, fp);
			WriteEquation(node->right, fp);
			fprintf(fp, "mul\n");
			break;
		}

		case TOK_DIV:
		{
			WriteEquation(node->left, fp);
			WriteEquation(node->right, fp);
			fprintf(fp, "div\n");
			break;
		}

		case TOK_POW:
		{
			WriteEquation(node->left, fp);
			if(node->right->tok == (int) TOK_NUM && node->right->un.value == 0.5)
			{
				fprintf(fp, "sqrt\n");
			}
			else
			{
				WriteEquation(node->right, fp);
				fprintf(fp, "pow\n");
			}
			break;
		}

		case TOK_SIN:
		{
			WriteEquation(node->right, fp);
			fprintf(fp, "sin\n");
			break;
		}

		case TOK_COS:
		{
			WriteEquation(node->right, fp);
			fprintf(fp, "cos\n");
			break;
		}

		case TOK_TG:
		{
			WriteEquation(node->right, fp);
			fprintf(fp, "tg\n");
			break;
		}
		
		case TOK_CTG:
		{
			WriteEquation(node->right, fp);
			fprintf(fp, "ctg\n");
			break;
		}

		case TOK_LN:
		{
			WriteEquation(node->right, fp);
			fprintf(fp, "ln\n");
			break;
		}

		case TOK_EXP:
		{
			WriteEquation(node->right, fp);
			fprintf(fp, "exp\n");
			break;
		}
		//IN, OUT and RET are processed by SYSEVAL
		default:
		{
			printf("Syntax error: wrong operation %d %lf %s\n", node->tok, node->un.value, node->un.name);
			return -1;
		}
    }
    return 0;
}

int WriteAssigment(Node* node, FILE* fp)
{
    assert(node);
    assert(fp);

    //int index = 0;
	
    if (node->left->tok == (int) TOK_VAR)
    {
        //index = node->left->number;
		SYSEVAL(node);
    }
	else
	{
		puts("only var assignments are allowed here");
		return -1;
	}

    //fprintf(fp, "push [%d]\n", index);
	//fprintf(fp, "push [%s]\n", node->left->un.name);
	puts("569");
    WriteEquation(node->right, fp);
	puts("571");
    int i = ArgListNumber(node->left->un.name);
	if(i == -1) 
	{
		i = argCnt;
		strcpy( argList[argCnt], node->left->un.name);
		printf("WAs wrote argument %s\n", argList[argCnt]);
		argCnt++;
	}
	if (isMain) fprintf(fp, "vpop [%d]\n", i);
	else fprintf(fp, "relpop [%d]\n", i);
	// if (isMain) fprintf(fp, "596vpop [%s]\n", node->left->un.name);
	// else fprintf(fp, "596vpop {%s}\n", node->left->un.name);
    return 0;
    }

// int WriteDefineVariable(Node* node, FILE* fp)
// {
    // assert(node);
    // assert(fp);

    // WriteEquation(node->right, fp);
    // //fprintf(fp, "pop [%d]\n", node->left->number);
	// fprintf(fp, "vppop [%s]\n", node->left->un.name);
    // return 0;
// }

int WriteDefineFunction(Node* node, FILE* fp)
{
    assert(node);
    assert(fp);
	//int isMain = 0;
	printf("%s\n", node->un.name);
	memset(argList, 0, sizeof(argList));
	argCnt = 0;
	if(!strcmp("main", node->un.name))
	{
		isMain = 1;
		fprintf(fp, ":main\n");
	}
	else
	{
		isMain = 0;
		fprintf(fp, "jump :_%s\n", node->un.name);
		fprintf(fp, "\n:%s\n", node->un.name);
	}

    Node* parametr = node->right;
    int   param_number = 0;
    while (parametr)
    {
        //fprintf(fp, "vpop [%s]\n", parametr->un.name);//
		fprintf(fp, "rpush %s\n", regList[param_number]);
		fprintf(fp, "relpop [%d]\n", argCnt);
		strcpy( argList[argCnt], parametr->un.name);
		printf("WDF wrote argument %s\n", argList[argCnt]);
		argCnt++;
        parametr = parametr->left;
        param_number += 1;
    }

    WriteBody(node->left, fp);
    //fprintf(fp, "ret\n");

	if (!isMain)
	{
		fprintf(fp, ":_%s\n", node->un.name);
	}
	else
	{
		fprintf(fp, "hlt\n");
	}

    return 0;
}

int WriteBody(Node* node, FILE* fp)
{
    assert(node);
    assert(fp);

    while (node)
    {
        if (WriteCommand(node->left, fp) != 0)
        {
            printf("Syntax error in program body\n");
            return -1;
        }
		if(node->right != NULL) printf("Right node: %d %lf %s\n", node->right->tok, node->right->un.value, (node->right->tok == TOK_NUM) ? "(num_node)" : node->right->un.name);
        node = node->right;
    }

    return 0;
}

int WriteWhile(Node* node, FILE* fp)
{
    assert(node);
    assert(fp);
	const int n = whileCnt;
	fprintf(fp, "jump :wh%d\n", n);
    fprintf(fp, ":wh%d\n", n);
    WriteEquation(node->left, fp);
	fprintf(fp, ":whend%d\n", n);
    WriteBody(node->right, fp);
    fprintf(fp, "jump :wh%d\n", n);

    return n;
}

int WriteIf(Node* node, FILE* fp)
{
    assert(node);
    assert(fp);

	const int n = ifCnt;
    if (node->tok == (int) TOK_IF)
    {
        WriteEquation(node->left, fp);
        fprintf(fp, ":ife%d\n", n);
        WriteBody(node->right, fp);
        fprintf(fp, "jump :ife%d\n", n);
    }
    else
    {
        WriteBody(node, fp);
        fprintf(fp, "jump :ife%d\n", n);
    }

    return n;
}

void SyntaxError()
{
	printf("Syntax error occured. Line: %d\n", __LINE__);
	assert(0);
}
