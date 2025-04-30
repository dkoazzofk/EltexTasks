#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX 20

typedef struct Social{
    char number[20];
    char link[20];
}social;

typedef struct Contact{
    int id;
    char lastname[20];
    char firstname[20];
    char middlename[20];
    social soc;
}contact;

int contactCount = 0;

void addContact(contact *Con){
    char input;
    printf("Input lastname: ");
    scanf("%s", &Con[contactCount].lastname);
    printf("Input firstname: ");
    scanf("%s", &Con[contactCount].firstname);
    printf("Input middlename: ");
    scanf("%s", &Con[contactCount].middlename);
    printf("If you want to input social data, press Y/N: ");
    scanf(" %c", &input);
    if(input == 'Y'){
        printf("Input link: ");
        scanf("%s", &Con[contactCount].soc.link);
        printf("Input number: ");
        scanf("%s", &Con[contactCount].soc.number);
        printf("Your id: %d\n", contactCount);
        Con[contactCount].id = contactCount;
    }else{
        printf("Your id: %d\n", contactCount);
        Con[contactCount].id = contactCount;
        Con[contactCount].soc.link[0] = '\0';
        Con[contactCount].soc.number[0] = '\0';
    }
    printf("\nUser added\n");
    contactCount++;
}

void listContact(contact *Con){
    for(int i = 0; i < contactCount; i++){
        printf("Id: %d\nLastname: %s\n", Con[i].id,Con[i].lastname);
        printf("Firstname: %s\nMiddlename: %s\n", Con[i].firstname,Con[i].middlename);
        printf("Link: %s\nNumber: %s\n",Con[i].soc.link,Con[i].soc.number);
    }
}
void deleteContact(contact *Con){
    int idDelete;
    printf("Input id which contact you want to delete: ");
    scanf("%d", &idDelete);
    for(int i = 0; i < contactCount; i++){
        if(idDelete == Con[i].id){
            for(int j = i; j < contactCount - 1; j++){
                Con[j] = Con[j + 1];
            }
            contactCount--;
            printf("User deleted\n");
            break;
        }
    }
}

void editContact(contact *Con){
    int idEdit;
    printf("Input id which contact you want to edit: ");
    scanf("%d", &idEdit);
    for(int i = 0; i < contactCount; i++){
        if(idEdit == Con[i].id){
            char input[MAX];
            printf("Input new lastname(if needed): ");
            getchar();
            fgets(input, MAX, stdin);
            if(input[0] != '\n') {
                strncpy(Con[i].lastname, input, MAX);
                Con[i].lastname[strcspn(Con[i].lastname, "\n")] = '\0';
            }
            printf("\nInput new firstname(if needed): ");
            fgets(input, MAX, stdin);
            if(input[0] != '\n'){
                strncpy(Con[i].firstname, input, MAX);
                Con[i].firstname[strcspn(Con[i].lastname, "\n")] = '\0';
            }
            printf("\nInput new middlename(if needed): ");
            fgets(input, MAX, stdin);
            if(input[0] != '\n'){
                strncpy(Con[i].middlename, input, MAX);
                Con[i].middlename[strcspn(Con[i].middlename, "\n")] = '\0';
            }
            printf("\nInput new link(if needed): ");
            fgets(input, MAX, stdin);
            if(input[0] != '\n'){
                strncpy(Con[i].soc.link, input, MAX);
                Con[i].soc.link[strcspn(Con[i].soc.link, "\n")] = '\0';
            }
            printf("\nInput new number(if needed): ");
            fgets(input, MAX, stdin);
            if(input[0] != '\n'){
                strncpy(Con[i].soc.number, input, MAX);
                Con[i].soc.number[strcspn(Con[i].soc.number, "\n")] = '\0';
            }
            printf("Contact updated. \n");
        }
    }
}


int main(){
    int choice;
    contact Con[MAX];
    do
    {
        printf("----Main Menu----\n");
        printf("1. Add Contact\n");
        printf("2. Edit Contact\n");
        printf("3. Delete Contact\n");
        printf("4. List Contact\n");
        printf("5. Exit\n");
        scanf("%d",&choice);
        switch (choice)
        {
        case 1:
            addContact(Con);
            break;
        case 2:
            editContact(Con);
            break;
        case 3:
            deleteContact(Con);
            break;
        case 4:
            listContact(Con);
            break;
        case 5:
            return 0;
        }
    } while (choice !=5);
}