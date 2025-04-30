#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX 20


//int contactCount = 0;
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

typedef struct TreeNode{
    contact con;
    struct TreeNode *left;
    struct TreeNode *right;
}TreeNode;

TreeNode* createNode(contact newCon){
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    if(node != NULL){
        node->con = newCon;
        node->left = node->right = NULL;
    }
    return node;
}

TreeNode* insertNode(TreeNode* root, contact newCon){
    if(root == NULL){
        return createNode(newCon);
    }
    if(newCon.id < root->con.id){
        root->left = insertNode(root->left, newCon);
    } else if(newCon.id > root->con.id){
        root->right = insertNode(root->right, newCon);
    }
    return root;
}

TreeNode* search(TreeNode* root, int id){
    if(root == NULL || root->con.id == id){
        return root;
    }
    if(id < root->con.id){
        return search(root->left, id);
    } else {
        return search(root->right, id);
    }
}

TreeNode* findMin(TreeNode* node){
    while(node->left != NULL){
        node = node->left;
    }
    return node;
}

TreeNode* delete(TreeNode* root, int id){
    if(root == NULL){
        return;
    }
    if(id < root->con.id){
        root->left = delete(root->left, id);
    } else if(id > root->con.id){
        root->right = delete(root->right, id);
    } else {
        if(root->left == NULL) {
            TreeNode* temp = root->right;
            free(root);
            return temp;
        } else if(root->right == NULL){
            TreeNode* temp = root->left;
            free(root);
            return temp;
        }

        TreeNode* temp = findMin(root->right);
        root->con = temp->con;
        root->right = delete(root->right, temp->con.id);
    }
    return root;
}

void postOrderTravel(TreeNode* root){
    if(root != NULL){
        postOrderTravel(root->left);
        postOrderTravel(root->right);
        printf("ID: %d\nLastname: %s\nFirstname: %s\nMiddlename: %s\nLink: %s\nNumber: %s\n", root->con.id, 
        root->con.lastname, root->con.firstname, root->con.middlename, root->con.soc.link, root->con.soc.number);
    }
}

void addContact(TreeNode** root){
    contact newCon;
    char input;
    printf("Input ID: ");
    scanf("%d", &newCon.id);
    printf("Input lastname: ");
    scanf("%s", newCon.lastname);
    printf("Input firstname: ");
    scanf("%s", newCon.firstname);
    printf("Input middlename: ");
    scanf("%s", newCon.middlename);
    printf("If you want to input social data, press Y/N: ");
    scanf(" %c", &input);
    if(input == 'Y'){
        printf("Input link: ");
        scanf("%s", newCon.soc.link);
        printf("Input number: ");
        scanf("%s", newCon.soc.number);
    }else{
        newCon.soc.link[0] = '\0';
        newCon.soc.number[0] = '\0';
    }
    // newCon.id = contactCount++;
    *root = insertNode(*root, newCon);
    printf("Your id: %d\n", newCon.id);
    printf("\nUser added\n");
}

void editContact(TreeNode** root){
    int idEdit;
    printf("Input id which contact you want to edit: ");
    scanf("%d", &idEdit);

    TreeNode* node = search(root, idEdit);

    char input[MAX];
    printf("Input new lastname(if needed): ");
    getchar();
    fgets(input, MAX, stdin);
    if(input[0] != '\n') {
        strncpy(node->con.lastname, input, MAX);
        node->con.lastname[strcspn(node->con.lastname, "\n")] = '\0';
    }
    printf("\nInput new firstname(if needed): ");
    fgets(input, MAX, stdin);
    if(input[0] != '\n'){
        strncpy(node->con.firstname, input, MAX);
        node->con.firstname[strcspn(node->con.lastname, "\n")] = '\0';
    }
    printf("\nInput new middlename(if needed): ");
    fgets(input, MAX, stdin);
    if(input[0] != '\n'){
        strncpy(node->con.middlename, input, MAX);
        node->con.middlename[strcspn(node->con.middlename, "\n")] = '\0';
    }
    printf("\nInput new link(if needed): ");
    fgets(input, MAX, stdin);
    if(input[0] != '\n'){
        strncpy(node->con.soc.link, input, MAX);
        node->con.soc.link[strcspn(node->con.soc.link, "\n")] = '\0';
    }
    printf("\nInput new number(if needed): ");
    fgets(input, MAX, stdin);
    if(input[0] != '\n'){
        strncpy(node->con.soc.number, input, MAX);
        node->con.soc.number[strcspn(node->con.soc.number, "\n")] = '\0';
    }
    printf("Contact updated. \n");
}

void deleteContact(TreeNode** root){
    int idDelete;
    printf("Input id which contact you want to delete: ");
    scanf("%d", &idDelete);

    if(search(*root, idDelete) != NULL){
        *root = delete(*root, idDelete);
        printf("Contact with id %d deleted.\n", idDelete);
    } else{
        printf("Contact not found.\n");
    }
}

void listContact(TreeNode* root){
    if(root == NULL){
        printf("Contact list is empty.\n");
        return;
    }
    printf("-----Contact list-----\n");
    postOrderTravel(root);
}

int main(){
    int choice;
    TreeNode* root = NULL;
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
            addContact(&root);
            break;
        case 2:
            editContact(root);
            break;
        case 3:
            deleteContact(&root);
            break;
        case 4:
            listContact(root);
            break;
        case 5:
            return 0;
        }
    } while (choice !=5);
}