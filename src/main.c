#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>


int main() {
    char* command = NULL;
    size_t size = 0;

    while (1) {
        printf("shell> ");
        ssize_t len = getline(&command, &size, stdin);

        if (len == -1) {
            break;
        }

        command[strcspn(command, "\n")] = 0;
        if (strcmp(command, "exit") == 0) break;

        pid_t pid = fork();

        if (pid > 0) {
            wait(NULL);
        }
        else if (pid == 0) {
            char* args[64];
            int i = 0;
            char* token = strtok(command, " ");
            while (token != NULL && i < 63) {
                args[i++] = token;
                token = strtok(NULL, " ");
            }
            args[i] = NULL;

            execvp(args[0], args);

            perror("exec failed");
            _exit(1);
        }
        else {
            fprintf(stderr, "Fork failed.\n");
        }
    }
    free(command);

    return 0;
}

