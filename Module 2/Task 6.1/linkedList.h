#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX 20

extern int contactCount;
typedef struct Social {
    char number[20];
    char link[20];
} social;

typedef struct Contact {
    int id;
    char lastname[20];
    char firstname[20];
    char middlename[20];
    social soc;
} Contact;

typedef struct Node {
    Contact con;
    struct Node* next;
    struct Node* prev;
} Node;

Node* createNode(Contact con);
void insert(Node** head, Contact con);
void removeNode(Node** head);
void printNode(Node* head);
void addContact(Node** head);
void editContact(Node* head);

#endif