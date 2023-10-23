#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_LINE 80
#define MAX_ARGS 10
#define HISTORY_SIZE 10

int main() {
    char input[MAX_LINE];
    char *args[MAX_ARGS];
    char *history[HISTORY_SIZE];
    char *token;
    int should_run = 1;
    int history_count = 0;

    while (should_run) {
        printf("osh> ");
        fgets(input, MAX_LINE, stdin);
        input[strlen(input) - 1] = '\0';

        if (strcmp(input, "\n") == 0) continue;
        if (strcmp(input, "!!") == 0) {
            if (history_count == 0) {
                printf("No commands in history.\n");
                continue;
            }
            strcpy(input, history[history_count - 1]);
            printf("the most recent command: %s\n", input);
        } else if (input[0] == '!') {
            int n = atoi(input + 1);
            if (n > 0 && n <= history_count) {
                strcpy(input, history[history_count - n]);
                printf("the %dth command: %s\n", n, input);
            } else {
                printf("No such command in history.\n");
                continue;
            }
        }

        if (strcmp(input, "exit") == 0) {
            should_run = 0;
            continue;
        }

        if (history_count < HISTORY_SIZE) {
            history[history_count] = strdup(input);
            history_count++;
        } else {
            free(history[0]);
            for (int i = 0; i < HISTORY_SIZE - 1; i++) {
                history[i] = history[i + 1];
            }
            history[HISTORY_SIZE - 1] = strdup(input);
        }

        int run_concurrently = 0;
        if (input[strlen(input) - 1] == '&') {
            run_concurrently = 1;
            input[strlen(input) - 1] = '\0';
        }

        token = strtok(input, " ");
        int i = 0;
        while (token != NULL) {
            args[i] = token;
            token = strtok(NULL, " ");
            i++;
        }
        args[i] = NULL;

        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        } else if (pid == 0) {
            if (execvp(args[0], args) == -1) {
                perror("Exec failed");
                exit(1);
            }
        } else {
            if (!run_concurrently) {
                int status;
                wait(&status);
            }
        }
    }

    for (int i = 0; i < history_count; i++) free(history[i]);

    return 0;
}
