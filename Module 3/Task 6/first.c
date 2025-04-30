#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

struct message {
    long mtype;
    char mtext[100];
};

int main(){
    key_t key = ftok(".", 'A');
    int msgid = msgget(key, 0666 | IPC_CREAT);

    struct message msg;
    int running = 1;

    while (running){
        msgrcv(msgid, &msg, sizeof(msg.mtext), 2, 0);
        printf("Catched from second: %s", msg.mtext);

        if (msg.mtype == 255) {
            printf("Second terminated trade\n");
            break;
        }

        printf("Enter message for Second: ");
        fgets(msg.mtext, 100, stdin);
        msg.mtype = 1;
        msgsnd(msgid, &msg, sizeof(msg.mtext), 0);

        if (strncmp(msg.mtext, "end", 3) == 0) {
            msg.mtype = 255;
            strcpy(msg.mtext, "Chat terminated\n");
            msgsnd(msgid, &msg, sizeof(msg.mtext), 0);
            running = 0;
        }
    }
    msgctl(msgid, IPC_RMID, NULL);
    return 0;
}