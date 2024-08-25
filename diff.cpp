#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "frontend.h"
#include "diff.h"

static const int op_add = ADD;
static const int op_sub = SUB;
static const int op_mul = MUL;
static const int op_div = DIV;
static const int op_pow = POW;
//
static const int fn_sin = SIN;
static const int fn_cos = COS;
static const int fn_ln  = LN;
static const int fn_exp = EXP;

Node* Copy(Node* node)
{
	Node* newNode = (Node*) calloc(1, sizeof(struct Node));
	memcpy(newNode, node, sizeof(struct Node));
	//newNode->type = node->type;
	
	if (node->left != NULL)
	{
		newNode->left = Copy(node->left);
	}
	if (node->right != NULL)
	{
		newNode->right = Copy(node->right);
	}
	
	return newNode;
}

Node* CreateNode(int type, const void* info, Node* left, Node* right)
{
	Node* newNode = (Node*)calloc(1, sizeof(struct Node));

    newNode->type = type;

    switch (type)
    {
		case NIL:
		{
			break;
		}
        case NUM:
		{
            newNode->un.value = *(double*)info;
            break;
		}
        case OP:
		{
            newNode->un.op = *(char*)info;
            break;
		}
		case VAR: case FUNCNAME: //FUNCNAME- functions declared in lang file (e.g.: main, fact)
		{
			newNode->un.name = (char*) calloc(strlen((char*) info), sizeof(char));
			strcpy(newNode->un.name, (char*) info);
			break;
		}
		case FNC: // FNC- standard functions from difftor(e.g.: sin, ln)
		{
			newNode->un.value = *(int*)info;
			break;
		}
	}
	
	newNode->left = left;
	newNode->right = right;
	
	return newNode;
}

Node* Diff(Node* node)
{
	//printf("\n\n$$ CURRENT NODE: %lf %c %c$$\n\n\n", node->un.value, node->un.op, node->un.var);
	switch(node->type)
	{
		case NIL:
		{
			puts("D0");
			return NULLCR();
		}
		case NUM:
		{
			return CREATE(0.0);
		}
		case VAR:
		{
			Node* res = CREATE(1.0);
			return res;
		}
		case OP:
		{
			switch(node->un.op)
			{
				case ADD:
				{
					return CRADD(Diff(node->left), Diff(node->right));
				}
				case SUB:
				{
					Node* res = CRSUB(Diff(node->left), Diff(node->right));
					//printf("\n\n$$ DIFFED NODE: %lf %c %c$$\n\n\n", node->un.value, node->un.op, node->un.var);
					return res;
				}
				case MUL:
				{
					Node* du  = Diff(node->left);
					Node* cu  = Copy(node->left);
					Node* dv  = Diff(node->right);
					Node* cv  = Copy(node->right);
					Node* res = CRADD( CRMUL( du, cv), CRMUL( cu, dv));
					return res;
				}
				case DIV:
				{
					Node* du  = Diff(node->left);
					Node* cu  = Copy(node->left);
					Node* dv  = Diff(node->right);
					Node* cv  = Copy(node->right);
					Node* res = CRDIV( CRSUB( CRMUL( du, cv), CRMUL( cu, dv)), CRMUL( cv, cv));
					return res;
				}
				case POW:
				{
					if (SearchVar(node->left) && !SearchVar(node->right))
					{
						Node* degree    = CRSUB( Copy(node->right), CREATE(1.0));
						Node* base      = Copy(node->left);
						Node* dbase     = Diff(node->left);
						Node* res       = CRMUL( Copy(node->right), CRMUL( dbase, CRPOW( base, degree)));
						return res;
					}
					else if (!SearchVar(node->left))
					{
						return CRMUL(CRLN(Copy(node->left)), CRMUL(CRPOW(Copy(node->left), Copy(node->right)), Diff(node->right)));
					}
				}
			}	
		}
		case FNC:
		{
			printf("fnc: %lf %d \n", node->un.value, (int) node->un.value);
			switch( (int) node->un.value)
			{
				case SIN: 
				{
					Node* res = CRMUL( CreateNode(FNC, &fn_cos, NULLCR(), Copy(node->right)), Diff(node->right));
					//printf("fnc: %d %lf %d \n", res->left->type, res->left->un.value, (int) res->left->un.value);
					return res;
				}
				case COS: 
				{
					return CRMUL( CREATE(-1.0), CRMUL( CreateNode(FNC, &fn_sin, NULLCR(), Copy(node->right)), Diff(node->right)));
				}
				case TG:
				{
					return CRMUL( CRDIV( CREATE(1.0), CRPOW( CreateNode(FNC, &fn_cos, NULLCR(), Copy(node->right)), CREATE(2.0))), Diff(node->right));
				}
				case CTG:
				{
					return CRMUL( CRDIV( CREATE(-1.0),CRPOW( CreateNode(FNC, &fn_sin, NULLCR(), Copy(node->right)), CREATE(2.0))), Diff(node->right));
				}
				case LN:
				{
					return CRMUL( CRDIV( CREATE(1.0), Copy(node->right)), Diff(node->right));
				}
				case EXP:
				{
					return CRMUL( CreateNode(FNC, &fn_exp, NULLCR(), Copy(node->right)), Diff(node->right));
				}
			}
		}
		default:
		{
			printf("Something went wrong: %d\n", node->type);
			//printf("$$ CURRENT NODE: %lf %c %c$$\n\n\n", node->un.value, node->un.op, node->un.var);
			return NULL;
		}
	}
}

int SearchVar(Node* node)
{
    int varFound = 0;
    
    if (node->type == VAR)  
		varFound = 1;
    
    if ((node->left != NULL) && (varFound != 1))  
		varFound = SearchVar(node->left); 

    if ((node->right != NULL) && (varFound != 1))  
		varFound = SearchVar(node->right); 
    
    return varFound;
}

double Calc(Node* node)
{
	double retVal = 0;
	switch(node->type)
	{
		
		case NIL: case NUM: case FUNCNAME:
		{
			return node->un.value;
		}
		case VAR:
		{
			puts("in fnc Calc: Variable encountered.");
			//*isVar = 1;
			printf("  variable name: %s \n", node->un.name);
			return node->un.value;
		}
		case OP:
		{
			switch(node->un.op)
			{
				case ADD:
				{
					retVal = Calc(node->left) + Calc(node->right);
					return retVal;
				}
				case SUB:
				{
					//printf("S %lf, %lf\n", Calc(node->left), Calc(node->right));
					retVal = Calc(node->left) - Calc(node->right);
					return retVal;
				}
				case MUL:
				{
					retVal = Calc(node->left) * Calc(node->right);
					return retVal;
				}
				case DIV:
				{
					//printf("D %lf, %lf\n", Calc(node->left), Calc(node->right));
					ZERODIVERR(node);
					retVal = Calc(node->left) / Calc(node->right);
					//printf("$$ %lf\n", retVal);
					return retVal;
				}
				case POW:
				{
					//printf("P %lf, %lf\n", Calc(node->left), Calc(node->right));
					retVal = pow(Calc(node->left), Calc(node->right));
					return retVal;
					break;
				}
				default:
				{
					puts("unknown operator! what the fuck");
					return retVal;
				}
			}
		}
		case FNC:
		{
			//puts("fnc");
			switch( (int) node->un.value)
			{
				case SIN: 
				{
					return sin(Calc(node->right));
				}
				case COS: 
				{
					//double sign = -1.0;
					return cos(Calc(node->right));
				}
				case TG:
				{
					//double divided = 1.0;
					TGINFERR(node);
					return tan(Calc(node->right));
				}
				case CTG:
				{
					CTGINFERR(node);
					return tan(3.14/2 - Calc(node->right));
				}
				case LN:
				{
					BADLOGERR(node);
					return log(Calc(node->right));
				}
				case EXP:
				{
					return exp(Calc(node->right));
				}
			}
		}
	}
	//printf("\n%lf \n", retVal);
}
Node* FirstSimplify(Node* node, int* ChangeCount)
{
	//puts("1s");
	//int isVar = 0;
	if (node == NULL || node->type == VAR || node->type == NUM || node->type == NIL)
	{
		return node;
	}
	printf("$$fs %d %c %lf %d \n", node->type, node->un.op, node->un.value, node->tok);
	//double countRes = Calc(node, &isVar);
	node->left = FirstSimplify (node->left, ChangeCount);
	node->right = FirstSimplify(node->right,ChangeCount);
	
	if ((node->type == OP && node->left->type == NUM && node->right->type == NUM) || 
	    (node->type == FNC && (node->right->type == NUM || 
		(node->right->type == OP && FirstSimplify(node->right->right, ChangeCount)->type == NUM && FirstSimplify(node->right->left, ChangeCount)->type == NUM) || (node->right->type == FNC && FirstSimplify(node->right->right, ChangeCount)->type == NUM))))
	{
		node->un.value = Calc(node);
		ChangeCount += 1;
		node->type = NUM;
		//printf("$$ %lf \n", node->un.value);
		free(node-> left);
		free(node->right);
		node->left  = NULL;
		node->right = NULL;
		//puts("FIN1");
	}
	//printf("$$fs %d %c %lf %c %s\n", node->type, node->un.op, node->un.value, (node->un.name == NULL) ? "none" : node->un.name);
	return node;
}

Node* SecondSimplify(Node* node, int* ChangeCount)
{
	puts("2s");
	if (node == NULL || node->type == NIL) return node;
	//printf("$2$ CURRENT NODE: %lf %c %c %d$$\n", node->un.value, node->un.op, node->un.var, node->type);
	if (node->type == OP)
	{
		node->left = SecondSimplify(node->left, ChangeCount);
		node->right= SecondSimplify(node->right,ChangeCount);
		
		if(node->left->type==NUM || node->right->type==NUM)
		{
			if (node->un.op == ADD)
			{
				if(node->left->type == NUM && !node->left->un.value)
				{
					TreeDtor(node->left);
					memcpy(node, node->right, sizeof(struct Node));
					*ChangeCount +=   1;
					return node;
				}
				else if (node->right->type == NUM && !node->right->un.value)
				{
					TreeDtor(node->right);
					memcpy(node, node->left, sizeof(struct Node));
					*ChangeCount +=   1;
					return node;
				}
				return node;
			}
			
			else if (node->un.op == SUB)
			{
				if (node->right->type == NUM && !node->right->un.value)
				{
					TreeDtor(node->right);
					memcpy(node, node->left, sizeof(struct Node));
					*ChangeCount +=   1;
					return node;
				}
				else if (node->left->type == NUM && !node->left->un.value)
				{
					TreeDtor(node->left);
					*ChangeCount += 1;
					return CRMUL( CREATE(-1.0), Copy(node->right));
				}
				return node;
			}
			
			else if (node->un.op == MUL)
			{
				if(node->left->type == NUM && node->left->un.value == 0)
				{
					puts("mul by 0");
					node->type = NUM;
					node->un.value = 0;
					TreeDtor(node->left);
					TreeDtor(node->right);
					*ChangeCount +=  1;
					return node;
				}
				else if (node->right->type == NUM && node->right->un.value == 0)
				{
					puts("mul by 0");
					node->type = NUM;
					node->un.value = 0;
					TreeDtor(node->left);
					TreeDtor(node->right);
					*ChangeCount +=  1;
					return node;
				}
				else if (node->left->type == NUM && node->left->un.value == 1)
				{
					puts("mul by 1");
					TreeDtor(node->left);
					memcpy(node, node->right, sizeof(struct Node));
					*ChangeCount +=   1;
					return node;
				}
				else if (node->right->type == NUM && node->right->un.value == 1)
				{
					puts("mul by 1");
					TreeDtor(node->right);
					memcpy(node, node->left, sizeof(struct Node));
					*ChangeCount +=   1;
					return node;
				}
				else if (node->left->un.op == DIV)
				{
					*ChangeCount += 1;
					Node* newnode = CRDIV( CRMUL(Copy(node->left->left), Copy(node->right)), Copy(node->left->right));
					TreeDtor(node);
					return newnode;
				}
				//return node;
			}
			
			else if (node->un.op == DIV)
			{
				if(node->left->type == NUM && node->left->un.value == 0)
				{
					node->type = NUM;
					node->un.value = 0;
					TreeDtor(node->right);
					TreeDtor(node->left);
					*ChangeCount +=  1;
					return node;
				}
				
				else if (node->right->type == NUM && node->right->un.value == 1)
				{
					TreeDtor(node->right);
					memcpy(node, node->left, sizeof(struct Node));
					*ChangeCount +=   1;
					return node;
				}
				else if (node->right->un.op == POW && node->left->type == NUM && node->left->un.value == 1)
				{
					node->right->right = CRMUL(CREATE(-1.0), Copy(node->right->right));
					TreeDtor(node->left);
					memcpy(node, node->right, sizeof(Node));
					return node;				
				}
				return node;
			}
			
			else if (node->un.op == POW)
			{
				if ((node->left->type == NUM && (node->left->un.value == 0 || node->left->un.value == 1)) || (node->right->type == NUM && node->right->un.value == 1))
				{
					//puts("11");
					TreeDtor(node->right);
					memcpy(node, node->left, sizeof(Node));
					*ChangeCount += 1;
					//printf("$$$ CURRENT NODE: %lf %c %c %d$$\n", node->un.value, node->un.op, node->un.var, node->type);
					return node;
				}
				
				else if (node->right->type == NUM && node->right->un.value == 0)
				{
					//puts("22");
					node->type = NUM;
					node->un.value = 1;
					TreeDtor(node->right);
					TreeDtor(node->left);
					*ChangeCount += 1;
					return node;
				}
			}
		}
	}
	else if (node->type == FNC && node->un.value == LN && node->right->un.op == POW)
	{
		//puts("509");
		node->right= SecondSimplify(node->right, ChangeCount);
		//TreeDtor(node->left);
		if (node->right->un.op == POW)
		{
			memcpy(node, CRMUL(Copy(node->right->right), CRLN(Copy(node->right->left))), sizeof(Node));
			*ChangeCount += 1;
		}
		return node;
	}
	else if (node->type == FNC && node->un.value == LN && node->right->un.op == DIV)
	{
		node->right = SecondSimplify(node->right, ChangeCount);
		if (node->right->un.op == DIV)
		{
			Node* newnode = CRMUL( Copy(node->right->left), CRPOW( Copy(node->right->right), CREATE(-1.0)));
			*ChangeCount += 1;
			TreeDtor(node);
			return CRLN(newnode);
		}
		return node;
	}
	//printf("$$-$ CURRENT NODE: %lf %c %c %d$$\n", node->un.value, ((node->type == 2) ? node->right->un.var : node->un.op), node->un.var, node->type);
	
	else if ((node->type == FNC && node->un.value == LN && node->right->type == FNC && node->right->un.value == EXP) || (node->type == FNC && node->un.value == EXP && node->right->type == FNC && node->right->un.value == LN))
	{
		Node* newnode = Copy(node->right->right);
		*ChangeCount += 1;
		TreeDtor(node);
		return newnode;
	}
	puts("533");
	//printf("$$$ CURRENT NODE: %lf %s %c %d$$\n", node->un.value, node->un.name, node->un.op, node->type);
	return node;
}

Node* ThirdSimplify(Node* node, int* ChangeCount)
{
	if (node == NULL) return node;
	puts("3s");
	if ((node->un.op == MUL || node->un.op == DIV) && (node->right != NULL && node->left != NULL))
	{
		//printf("$$-$ CURRENT NODE: %lf %c %c %d$$\n", node->un.value, ((node->type == VAR) ? node->un.var : node->un.op), node->un.var, node->type);
		printf("$$$$ LEFT NODE: %lf %c %s %d$$\n", node->left->un.value, node->left->un.op, node->left->un.name, node->left->type);
		printf("$$$$ RIGHT NODE: %lf %c %s %d$$\n", node->right->un.value, node->right->un.op, node->right->un.name, node->right->type);
		node->left = ThirdSimplify(node->left, ChangeCount);
		node->right= ThirdSimplify(node->right,ChangeCount);
		if (node->right->un.op == MUL || node->left->un.op == MUL || node->right->un.op == DIV || node->left->un.op == DIV)
		{
			if (node->left->type == NUM || node->left->type == VAR)
			{
				//puts("case1");
				Node* newnode = NULL;
				int tp = node->left->type;
				if (node->right->right->type != tp && node->right->left->type == tp) // (NUM)-.-((NUM)-.-(?))
				{	
					if (node->un.op == MUL && node->right->un.op == MUL)
						newnode = CRMUL(CRMUL(Copy(node->left), Copy(node->right->left)), Copy(node->right->right));
					
					else if (node->un.op == MUL && node->right->un.op == DIV)
						newnode = CRDIV(CRMUL(Copy(node->left), Copy(node->right->left)), Copy(node->right->right));
					
					else if (node->un.op == DIV && node->right->un.op == DIV)
						newnode = CRMUL(CRDIV(Copy(node->left), Copy(node->right->left)), Copy(node->right->right));
					
					else if (node->un.op == DIV && node->right->un.op == MUL)
						newnode = CRDIV(CRDIV(Copy(node->left), Copy(node->right->left)), Copy(node->right->right));
					
					if (newnode == NULL)
						return node;
					
					*ChangeCount += 1;
					TreeDtor(node);
					return newnode;
				}
				else if (node->right->right->type == tp && node->right->left->type != tp) // (NUM)-.-((?)-.-(NUM))
				{
					if (node->un.op == MUL && node->right->un.op == MUL)
						newnode = CRMUL(CRMUL(Copy(node->left), Copy(node->right->right)), Copy(node->right->left));
					
					else if (node->un.op == MUL && node->right->un.op == DIV)
						newnode = CRMUL(CRDIV(Copy(node->left), Copy(node->right->right)), Copy(node->right->left));
					
					else if (node->un.op == DIV && node->right->un.op == DIV)
						newnode = CRDIV(CRMUL(Copy(node->left), Copy(node->right->right)), Copy(node->right->left));
					
					else if (node->un.op == DIV && node->right->un.op == MUL)
						newnode = CRDIV(CRDIV(Copy(node->left), Copy(node->right->right)), Copy(node->right->left));
					
					if (newnode == NULL)
						return node;
					
					*ChangeCount += 1;
					TreeDtor(node);
					return newnode;
				}
			}
			else if (node->right->type == NUM || node->right->type == VAR)
			{
				//puts("case2");
				Node* newnode = NULL;
				int tp = node->right->type;
				if (node->left->right->type != tp && node->left->left->type == tp) // ((NUM)-.-(?))-.-(NUM)
				{
					if (node->un.op == MUL && node->left->un.op == MUL)
						newnode = CRMUL(CRMUL(Copy(node->left->left), Copy(node->right)), Copy(node->left->right));
					
					else if (node->un.op == MUL && node->left->un.op == DIV)
						newnode = CRDIV(CRMUL(Copy(node->left->left), Copy(node->right)), Copy(node->left->right));
					
					else if (node->un.op == DIV && node->left->un.op == DIV)
						newnode = CRDIV(CRDIV(Copy(node->left->left), Copy(node->right)), Copy(node->left->right));
					
					else if (node->un.op == DIV && node->left->un.op == MUL)
						newnode = CRMUL(CRDIV(Copy(node->left->left), Copy(node->right)), Copy(node->left->right));
					
					if (newnode == NULL)
						return node;
					
					*ChangeCount += 1;
					TreeDtor(node);
					return newnode;
				}
				else if (node->left->right->type == tp && node->left->left->type != tp) // ((?)-.-(NUM))-.-(NUM)
				{
					if (node->un.op == MUL && node->left->un.op == MUL)
						newnode = CRMUL(CRMUL(Copy(node->left->right), Copy(node->right)), Copy(node->left->left));
					
					else if (node->un.op == MUL && node->left->un.op == DIV)
						newnode = CRMUL(CRDIV(Copy(node->right), Copy(node->left->right)), Copy(node->left->left));
					
					else if (node->un.op == DIV && node->left->un.op == DIV)
						newnode = CRDIV(Copy(node->left->left), CRMUL(Copy(node->left->right), Copy(node->right)));
					
					else if (node->un.op == DIV && node->left->un.op == MUL)
						newnode = CRDIV(Copy(node->left->left), CRDIV(Copy(node->right), Copy(node->left->left)));
					
					if (newnode == NULL)
						return node;
					
					*ChangeCount += 1;
					TreeDtor(node);
					return newnode;
				}
			}
			else if (node->left->un.op == DIV)
			{
				//puts("557");
				*ChangeCount += 1;
				//Node* tmp = CRMUL(node->left->left, 
				Node* newnode = CRDIV( CRMUL(Copy(node->left->left), Copy(node->right)), Copy(node->left->right));
				TreeDtor(node);
				return newnode;
			}
			return node;
		}
		else if (node->right->un.op == POW && node->right->left->type == VAR && node->left->type == VAR && !strcmp(node->left->un.name, node->right->left->un.name)) // x*(x^(...))
		{
			////puts("590");
			Node* newnode = NULL;
			if (node->un.op == MUL)
				newnode = CRPOW(Copy(node->right->left), CRADD(CREATE(1.0), Copy(node->right->right)));//CreateNode(OP, &op_add, Copy(node->left->right), CREATE(1.0));
			else if (node->un.op == DIV)
				newnode = CRPOW(Copy(node->right->left), CRSUB(CREATE(1.0), Copy(node->right->right)));//CreateNode(OP, &op_sub, Copy(node->left->right), CREATE(1.0));
			*ChangeCount += 1;
			TreeDtor(node);
			return newnode;
		}
		else if (node->left->un.op == POW && node->left->left->type == VAR && node->right->type == VAR && !strcmp(node->right->un.name, node->left->left->un.name)) // (x^(...))*x
		{
			//puts("603");
			Node* newnode = NULL;
			if (node->un.op == MUL)
				newnode = CRPOW(Copy(node->left->left), CRADD(Copy(node->left->right), CREATE(1.0)));//CreateNode(OP, &op_add, Copy(node->left->right), CREATE(1.0));
			else if (node->un.op == DIV)
				newnode = CRPOW(Copy(node->left->left), CRSUB(Copy(node->left->right), CREATE(1.0)));//CreateNode(OP, &op_sub, Copy(node->left->right), CREATE(1.0));
			*ChangeCount += 1;
			TreeDtor(node);
			return newnode;
		}
		else if (node->left->un.op == POW && node->right->un.op == POW && EqualSubEq(node->left->left, node->right->left)) // (...)^(x1)*(...)^(x2)
		{
			//puts("575");
			if (node->un.op == MUL)
			{
				*ChangeCount += 1;
				//memcpy(node, CreateNode(OP, &op_pow, Copy(node->left->left), CreateNode(OP, &op_add, node->left->right, node->right->right)), sizeof(Node));
				Node* newnode = CRPOW(Copy(node->left->left), CRADD(Copy(node->left->right), Copy(node->right->right)));
				TreeDtor(node);
				return newnode;
			}
			else
			{
				*ChangeCount += 1;
				Node* newnode = CRPOW(Copy(node->left->left), CRSUB(Copy(node->left->right), Copy(node->right->right)));
				TreeDtor(node);
				return newnode;
			}
		}
		else if(node->un.op == MUL && node->left->un.op == MUL && node->right->un.op == POW && (EqualSubEq(node->left->left, node->right->left) || EqualSubEq(node->left->right, node->right->left)))
		{
			//puts("637");
			*ChangeCount += 1;
			if (EqualSubEq(node->left->left, node->right->left))
				NodeSwap(&node->left->right, &node->right->left);
			else 
				NodeSwap(&node->left->left, &node->right->left);
			return node;
		}
	}
	else if (node->un.op == POW ) 
	{
		//printf("$$$ %d\n", *ChangeCount);
		//puts("592");
		node->left = ThirdSimplify(node->left, ChangeCount);
		node->right= ThirdSimplify(node->right,ChangeCount);
		if(node->left->un.op == POW) //((...)^(.))^(..)
		{
			//puts("596");
			*ChangeCount += 1;
			//memcpy(node, CreateNode(OP, &op_pow, Copy(node->left->left), CreateNode(OP, &op_mul, Copy(node->left->right), Copy(node->right))), sizeof(Node));
			Node* newnode = CRPOW(Copy(node->left->left), CRMUL(Copy(node->left->right), Copy(node->right)));
			TreeDtor(node);
			return newnode;
		}
		else if(node->right->un.op == POW) //(...)^((.)^(..))
		{
			//puts("603");
			*ChangeCount += 1;
			//memcpy(node, CreateNode(OP, &op_pow, Copy(node->left), CreateNode(OP, &op_mul, Copy(node->right->left), Copy(node->right->right))), sizeof(Node));
			Node* newnode = CRPOW(Copy(node->left), CRMUL(Copy(node->right->left), Copy(node->right->right)));
			TreeDtor(node);
			return node;
		}
		//printf("$$$ %d\n", *ChangeCount);
	}
	return node;
}

void NodeSwap(Node** node1, Node** node2)
{
	if (*node1 != NULL && *node2 != NULL)
	{
		Node* temp = *node1;
		*node1 = *node2;
		*node2 = temp;
	}
}

int EqualSubEq(Node* node1, Node* node2)
{
	if(node1 == NULL && node2 == NULL)
		return 1;
	if((node1 == NULL && node2 != NULL) || (node1 != NULL && node2 == NULL))
		return 0;
	if(node1->type != node2->type || node1->un.value != node2->un.value || node1->un.op != node2->un.op)
		return 0;
	if(!EqualSubEq(node1->left, node2->left))
		return 0;
	if(!EqualSubEq(node1->right, node2->right))
		return 0;
	
	return 1;
}

Node* CREATE(double val)
{
	double tmp = val;
	return CreateNode(NUM, &tmp, NULL, NULL);
}

Node* CRADD(Node* left, Node* right)
{
	return CreateNode(OP, &op_add, left, right);
}

Node* CRSUB(Node* left, Node* right)
{
	return CreateNode(OP, &op_sub, left, right);
}

Node* CRMUL(Node* left, Node* right)
{
	return CreateNode(OP, &op_mul, left, right);
}

Node* CRDIV(Node* left, Node* right)
{
	return CreateNode(OP, &op_div, left, right);
}

Node* CRPOW(Node* left, Node* right)
{
	return CreateNode(OP, &op_pow, left, right);
}

Node* CRLN(Node* val)
{
	return CreateNode(FNC, &fn_ln, NULLCR(), val);
}

Node* NULLCR()
{
	double tmp = 0.0;
	return CreateNode(NIL, &tmp, NULL, NULL);
}

Node* Simpify (Node* node) ///
{
	if (node != NULL)
	{
		//if (node->un.op == SMC && node->left->type == FUNCNAME)
		if (node->type == OP && node->left->type == FUNCNAME)
		{
			puts("Function wrap");
			//if(node->left->right == NULL) puts("1");
			if(node->left->left != NULL) node->left->left = Simpify(node->left->left);
			if(node->left->right != NULL) node->left->right = Simpify(node->left->right);
			if(node->right != NULL) node->right = Simpify(node->right);
			return node;
		}
		int ChangeCount = 1;
		while(ChangeCount != 0)
		{
			ChangeCount = 0;
			printf("------\n %d\n -----\n", ChangeCount);
			node = FirstSimplify(node, &ChangeCount);
			printf("------\n %d\n -----\n", ChangeCount);
			node = SecondSimplify(node, &ChangeCount);
			//puts("2");
			printf("------\n %d\n -----\n", ChangeCount);
			node = ThirdSimplify(node, &ChangeCount);
			printf("------\n %d\n -----\n", ChangeCount);
		}	
	}
	return node;
}

void ZERODIVERR(Node* node)
{
	if(Calc(node->right) == 0.0 && node->un.op == DIV)
	{
		puts("ERROR: DIVISION BY ZERO");
		assert(0);
	}
}

void BADLOGERR(Node* node)
{
	if ( ((int)node->un.value == LN) && Calc(node->right) <= 0.0)
	{
		puts("ERROR: LOGARITHM OFF NON-POSITIVE NUMBER");
		assert(0);
	}
}

void TGINFERR(Node* node)
{
	if (((int)node->un.value == TG) && cos(Calc(node->right)) == 0)
	{
		puts("ERROR: TAN DOES NOT EXIST");
		assert(0);
	}
}

void CTGINFERR(Node* node)
{
	if (((int)node->un.value == CTG) && sin(Calc(node->right)) == 0)
	{
		puts("ERROR: COT DOES NOT EXIST");
		assert(0);
	}
}