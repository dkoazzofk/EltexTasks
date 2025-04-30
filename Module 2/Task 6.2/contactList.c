#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include "contactList.h"

int contactCount = 0;

Node* createNode(Contact con) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->con = con;
    newNode->next = NULL;
    newNode->prev = NULL;
    return newNode;
}

void insert(Node** head, Contact con) {
    Node* newNode = createNode(con);
    if (*head == NULL) {
        *head = newNode;
        return;
    }
    Node* current = *head;
    while (current != NULL && current->con.id < con.id) {
        current = current->next;
    }
    if (current == *head) {
        newNode->next = *head;
        (*head)->prev = newNode;
        *head = newNode;
    }
    else if (current == NULL) {
        Node* last = *head;
        while (last->next != NULL) {
            last = last->next;
        }
        last->next = newNode;
        newNode->prev = last;
    }
    else {
        newNode->next = current;
        newNode->prev = current->prev;
        current->prev->next = newNode;
        current->prev = newNode;
    }
}

void removeNode(Node** head) {
    int id;
    Node* current = *head;
    printf("Input ID to delete contact: ");
    scanf("%d", &id);
    while (current != NULL && current->con.id != id) {
        current = current->next;
    }
    if (current == NULL) {
        printf("Contact not found!\n");
        return;
    }
    if (current == *head) {
        *head = current->next;
        if (*head != NULL) (*head)->prev = NULL;
    }
    else {
        if (current->next != NULL) {
            current->next->prev = current->prev;
        }
        if (current->prev != NULL) {
            current->prev->next = current->next;
        }
    }
    free(current);
    printf("Contact deleted.\n");
}

void printNode(Node* head) {
    Node* current = head;
    while (current != NULL) {
        printf("ID: %d\nFirstname: %s\nLastname: %s\nMiddlename: %s\nPhone: %s\nLink: %s\n\n",
            current->con.id, current->con.firstname, current->con.lastname, current->con.middlename,
            current->con.soc.number, current->con.soc.link);
        current = current->next;
    }
}

void addContact(Node** head) {
    Contact newCon;
    char input;
    printf("Input lastname: ");
    scanf("%s", newCon.lastname);
    printf("Input firstname: ");
    scanf("%s", newCon.firstname);
    printf("Input middlename: ");
    scanf("%s", newCon.middlename);
    printf("If you want to input social data, press Y/N: ");
    scanf(" %c", &input);
    if (input == 'Y' || input == 'y') {
        printf("Input link: ");
        scanf("%s", newCon.soc.link);
        printf("Input number: ");
        scanf("%s", newCon.soc.number);
        printf("Your id: %d\n", contactCount);
        newCon.id = contactCount;
    }
    else {
        printf("Your id: %d\n", contactCount);
        newCon.id = contactCount;
        newCon.soc.link[0] = '\0';
        newCon.soc.number[0] = '\0';
    }
    printf("\nUser added\n");
    contactCount++;
    insert(head, newCon);
}

void editContact(Node* head) {
    int idEdit;
    printf("Input id which contact you want to edit: ");
    scanf("%d", &idEdit);

    Node* current = head;
    while (current != NULL && current->con.id != idEdit) {
        current = current->next;
    }
    if (current == NULL) {
        printf("Contact not found!\n");
        return;
    }
    char input[MAX];
    printf("Input new lastname(if needed): ");
    getchar();
    fgets(input, MAX, stdin);
    if (input[0] != '\n') {
        strncpy(current->con.lastname, input, MAX);
        current->con.lastname[strcspn(current->con.lastname, "\n")] = '\0';
    }
    printf("\nInput new firstname(if needed): ");
    fgets(input, MAX, stdin);
    if (input[0] != '\n') {
        strncpy(current->con.firstname, input, MAX);
        current->con.firstname[strcspn(current->con.firstname, "\n")] = '\0';
    }
    printf("\nInput new middlename(if needed): ");
    fgets(input, MAX, stdin);
    if (input[0] != '\n') {
        strncpy(current->con.middlename, input, MAX);
        current->con.middlename[strcspn(current->con.middlename, "\n")] = '\0';
    }
    printf("\nInput new link(if needed): ");
    fgets(input, MAX, stdin);
    if (input[0] != '\n') {
        strncpy(current->con.soc.link, input, MAX);
        current->con.soc.link[strcspn(current->con.soc.link, "\n")] = '\0';
    }
    printf("\nInput new number(if needed): ");
    fgets(input, MAX, stdin);
    if (input[0] != '\n') {
        strncpy(current->con.soc.number, input, MAX);
        current->con.soc.number[strcspn(current->con.soc.number, "\n")] = '\0';
    }
    printf("Contact updated. \n");
}