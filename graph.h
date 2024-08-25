#ifndef GRAPH_H
#define GRAPH_H

int Graphviz (struct Node* root, const char* fout);
void AddNode(Node* root, FILE* file, int* NodeNum);
void AddCon(Node* root, FILE* file);

#endif