#include "linkedList.h"

int main() {
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
}