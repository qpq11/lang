#ifndef DIFF_H
#define DIFF_H

struct Node* Copy(struct Node* node);

struct Node* CreateNode(int type, const void* info, struct Node* left, struct Node* right);

Node* CREATE(double val);

Node* NULLCR();

Node* CRADD(Node* left, Node* right);

Node* CRSUB(Node* left, Node* right);

Node* CRMUL(Node* left, Node* right);

Node* CRDIV(Node* left, Node* right);

Node* CRPOW(Node* left, Node* right);

Node* CRLN(Node* val);

struct Node* Diff(struct Node* node);

int SearchVar(Node* node);

double Calc(struct Node* node);

void ZERODIVERR(Node* node);

void TGINFERR(Node* node);

void CTGINFERR(Node* node);

void BADLOGERR(Node* node);

Node* FirstSimplify(struct Node* node, int* ChangeCount);

Node* SecondSimplify(struct Node* node, int* ChangeCount);

Node* ThirdSimplify(struct Node* node, int* ChangeCount);

void NodeSwap(Node** node1, Node** node2);

int EqualSubEq(Node* node1, Node* node2);

Node* Simpify (struct Node* node);

#endif