#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define MAX 4096

int main() {
    char command[MAX];

    while (1) {
        printf("shell> ");
        if (!fgets(command, MAX, stdin)) break;
        command[strcspn(command, "\n")] = 0;
        if (strcmp(command, "exit") == 0) break;

        pid_t pid = fork();

        if (pid > 0) {
            wait(NULL);
        }
        else if (pid == 0) {
            char* args[32];
            int i = 0;
            char* token = strtok(command, " ");
            while (token != NULL && i < 32) {
                args[i++] = token;
                token = strtok(NULL, " ");
            }
            args[i] = NULL;

            execvp(args[0], args);

            perror("exec failed");
        }
        else {
            fprintf(stderr, "Fork failed.\n");
        }
    }

    return 0;
}

