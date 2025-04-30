#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>

void createFile(char *filename){
    int fd = open(filename, O_WRONLY | O_CREAT, 0644);
    if(fd == -1){
        perror("Can't create file.");
        return;
    }
    close(fd);
}

void simpleCheck(){
    DIR *dir = opendir("."); //открываем каталог ".", в котором уже находятся файлы
    if(!dir){
        perror("opendir");
        return;
    }
    struct dirent *entry; //нужен для пробежки по всей директории и для вывода файлов в этой директории
    while((entry = readdir(dir)) != NULL){
        if(entry->d_name[0] != "."){
            printf("%s\n", entry->d_name);
        }
    }
    closedir(dir);
}

void coolCheck(){
    DIR *dir = opendir(".");
    if(!dir){
        perror("opendir");
        return;
    }
    struct dirent *entry;
    struct stat fileInfo;

    while((entry = readdir(dir)) != NULL){
        if(entry->d_name[0] == ".") continue;

        if(stat(entry->d_name, &fileInfo) == -1){
            perror(entry->d_name);
            continue;
        }
        
        printf((S_ISDIR(fileInfo.st_mode)) ? "d" : "-");
        printf((fileInfo.st_mode & S_IRUSR) ? "r" : "-");
        printf((fileInfo.st_mode & S_IWUSR) ? "w" : "-");
        printf((fileInfo.st_mode & S_IXUSR) ? "x" : "-");
        printf((fileInfo.st_mode & S_IRGRP) ? "r" : "-");
        printf((fileInfo.st_mode & S_IWGRP) ? "w" : "-");
        printf((fileInfo.st_mode & S_IXGRP) ? "x" : "-");
        printf((fileInfo.st_mode & S_IROTH) ? "r" : "-");
        printf((fileInfo.st_mode & S_IWOTH) ? "w" : "-");
        printf((fileInfo.st_mode & S_IXOTH) ? "x" : "-");
        printf(" %s\n", entry->d_name);
    }

}

void printBit(mode_t mode) {
    for (int i = 8; i >= 0; i--) {
        printf("%d", (mode >> i) & 1);
    }
    printf("\n");
}

void toString(mode_t mode, char *str) {
    strcpy(str, "----------");
    if (S_ISDIR(mode)) str[0] = 'd';
    if (mode & S_IRUSR) str[1] = 'r';
    if (mode & S_IWUSR) str[2] = 'w';
    if (mode & S_IXUSR) str[3] = 'x';
    if (mode & S_IRGRP) str[4] = 'r';
    if (mode & S_IWGRP) str[5] = 'w';
    if (mode & S_IXGRP) str[6] = 'x';
    if (mode & S_IROTH) str[7] = 'r';
    if (mode & S_IWOTH) str[8] = 'w';
    if (mode & S_IXOTH) str[9] = 'x';
}

void symbolicAccess(mode_t *mode, const char *symbolic) {
    mode_t who = 0;
    const char *ptr = symbolic;
    
    while (*ptr && (*ptr == 'u' || *ptr == 'g' || *ptr == 'o' || *ptr == 'a')) {
        switch (*ptr) {
            case 'u': who |= S_IRUSR | S_IWUSR | S_IXUSR; break;
            case 'g': who |= S_IRGRP | S_IWGRP | S_IXGRP; break;
            case 'o': who |= S_IROTH | S_IWOTH | S_IXOTH; break;
            case 'a': who |= S_IRWXU | S_IRWXG | S_IRWXO; break;
        }
        ptr++;
    }
    
    if (!who) who = S_IRWXU | S_IRWXG | S_IRWXO;
    
    char op = *ptr++;
    if (op != '+' && op != '-' && op != '=') {
        printf("Invalid operation: %c\n", op);
        return;
    }
    mode_t perm = 0;
    while (*ptr) {
        switch (*ptr) {
            case 'r': perm |= (S_IRUSR | S_IRGRP | S_IROTH); break;
            case 'w': perm |= (S_IWUSR | S_IWGRP | S_IWOTH); break;
            case 'x': perm |= (S_IXUSR | S_IXGRP | S_IXOTH); break;
            default: ptr++; continue;
        }
        ptr++;
    }
    
    switch (op) {
        case '=':
            *mode &= ~who;
            *mode |= (perm & who);
            break;
        case '+':
            *mode |= (perm & who);
            break;
        case '-':
            *mode &= ~(perm & who);
            break;
    }
}

void Chmod(char *access, char *filename) {
    if (!access || !filename) {
        printf("Incorrect usage: chmod <access> <filename>\n");
        return;
    }
    struct stat st;
    if (stat(filename, &st) == -1) {
        perror("stat");
        return;
    }
    mode_t mode = st.st_mode;
    
    if (isdigit(access[0])) {
        mode = strtol(access, NULL, 8);
    } else {
        symbolicAccess(&mode, access);
    }

    if (chmod(filename, mode) == -1) {
        perror("chmod");
    } else {
        char mode_str_result[11];
        toString(mode, mode_str_result);
        printf("Permissions of '%s' changed to:\n", filename);
        printf("Octal: %o\n", mode);
        printf("Symbolic: %s\n", mode_str_result);
        printf("Bitwise: ");
        printBit(mode);
    }
}

int main(){
    char input[100];
    while(1){
        printf(">>> ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input,"\n")] = '\0';

        char *command = strtok(input, " ");
        char *arg1 = strtok(NULL, " ");
        char *arg2 = strtok(NULL, " ");

        if(!command) continue;

        if(strcmp(command, "exit") == 0){
            break;
        } else if(strcmp(command, "touch") == 0){
            createFile(arg1);
        } else if(strcmp(command, "ls") == 0){
            if(arg1 && strcmp(arg1, "-l") == 0){
                coolCheck();
            } else simpleCheck();
        } else if(strcmp(command, "chmod") == 0){
            Chmod(arg1, arg2);
        } else printf("Unknown command: %s\n", command);
    }
}