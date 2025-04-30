#include "contactList.h"
#include <stdlib.h>

int main() {
    void *handle;
    handle = dlopen("./liblinkedList.so", RTLD_LAZY);
    if(!handle){
        fprintf(stderr, "Error lib: %s\n", dlerror());
        exit(1);
    }

    Node* (*createNode)(Contact) = dlsym(handle, "createNode");
    void (*insert)(Node**, Contact) = dlsym(handle, "insert");
    void (*removeNode)(Node**) = dlsym(handle, "removeNode");
    void (*printNode)(Node*) = dlsym(handle, "printNode");
    void (*addContact)(Node**) = dlsym(handle, "addContact");
    void (*editContact)(Node*) = dlsym(handle, "editContact");

    Node* head = NULL;
    int choice;
    do {
        printf("----Main Menu----\n");
        printf("1. Add Contact\n");
        printf("2. Edit Contact\n");
        printf("3. Delete Contact\n");
        printf("4. List Contact\n");
        printf("5. Exit\n");
        scanf("%d", &choice);
        switch (choice) {
        case 1:
            addContact(&head);
            break;
        case 2:
            editContact(head);
            break;
        case 3:
            removeNode(&head);
            break;
        case 4:
            printNode(head);
            break;
        case 5:
            return 0;
        default:
            printf("Invalid choice!\n");
        }
    } while (choice != 5);
    dlclose(handle);
    return 0;
}