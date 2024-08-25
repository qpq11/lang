#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "frontend.h"
#include "diff.h"
#include "graph.h"

int main(int argc, char *argv[])
{
	const char* srcname = nullptr;
	const char* resname = "treeprint.txt";
	if (argc < 2)
	{
		printf("Incorrect args number\n");
		return 1;
	}
	else if (argc == 2)
	{
		srcname = argv[1];
	}
	else if (argc > 2)
	{
		srcname = argv[1];
		resname = argv[2];
	}
	FILE* fp = fopen(resname, "w+");
	Node* root = NULL;
	ReadFile(&root, srcname);
	puts("---lang tree created---");
	Graphviz(root, "graphresult.dot");
	// printf("--------------TREE PRINT STARTED----------------\n");
	// TreePrint(root, fp);
	// fprintf(fp, "\n");
	// printf("--------------TREE PRINT IS OVER----------------\n");
	Node* res = Simpify(root);
	Graphviz(res, "smplres.dot");
	printf("--------------TREE PRINT STARTED----------------\n");
	TreePrint(res, fp);
	fprintf(fp, "\n");
	printf("--------------TREE PRINT IS OVER----------------\n");
	fclose(fp);
	TreeDtor(root);
	return 0;
}