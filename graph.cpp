#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "frontend.h"
#include "diff.h"
#include "graph.h"

int Graphviz (struct Node* root, const char* fout)
{
    int NodeNum = 0;
    if (root == NULL) return 1;
    FILE* fp = fopen(fout, "w+");

    fprintf (fp, "digraph LANGTREE{\n"
                         "label = < LANGTREE >;\n"
                         "node [shape = record ];\n"
                         "edge [style = filled ];\n");
    AddNode(root, fp, &NodeNum);
    AddCon (root, fp);
    fprintf (fp, "}");
    fclose (fp);
    return 0;
}

void AddNode(Node* node, FILE* fp, int* NodeNum)
{
	if (node == NULL)
		return;
	
	printf("address: %d\n", &node);
	
    if (node->type == VAR )
        fprintf (fp, " %d [shape = Mrecord, style = filled, fillcolor = Green, label = \"%s\" ];\n", *NodeNum, node->un.name);

	if (node->type == FUNCNAME)
		fprintf (fp, " %d [shape = Mrecord, style = filled, fillcolor = Blue, label = \"%s\" ];\n", *NodeNum, node->un.name);
	
    else if (node->type == NUM)
        fprintf (fp, " %d [shape = Mrecord, label = \"%lf\" ];\n", *NodeNum, node->un.value);
	
	else if (node->type == OP)
		if(node->un.op != ABV && node->un.op != BLW)  //ebany graphviz tupit na < i >
			fprintf(fp, " %d [shape = Mrecord, style = filled, fillcolor = Yellow, label = \"%c\" ];\n", *NodeNum, node->un.op);
		else
			fprintf(fp, " %d [shape = Mrecord, style = filled, fillcolor = Yellow, label = \"\\%c\" ];\n", *NodeNum, node->un.op);
	
	else if (node->type == FNC)
		fprintf(fp, " %d [shape = Mrecord, style = filled, fillcolor = YellowGreen, label = \"%s\" ];\n", *NodeNum, Fncs[(int) node->un.value]);
	
	node->order = *NodeNum;
	if (node->left != NULL && node->left->type != NIL)
	{
		*NodeNum += 1;
		AddNode(node->left, fp, NodeNum);
	}
	if (node->right != NULL)
	{
		*NodeNum += 1;
		AddNode(node->right, fp, NodeNum);
	}
	return;
}

void AddCon(Node* node, FILE* fp)
{
	if (node == NULL)
		return;
	if (node->left != NULL && node->left->type != NIL)
	{
		fprintf(fp, "%d -> %d;\n", node->order, node->left->order);
		AddCon(node->left, fp);
	}		
	
	if (node->right != NULL && node->right->type != NIL)
	{
		fprintf(fp, "%d -> %d;\n", node->order, node->right->order);
		AddCon(node->right, fp);
	}
	return;	
}