#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_LINE 80
#define HISTORY_SIZE 10
#define HISTORY_MAX_SIZE 10000

int main() {
    char input[MAX_LINE];   // 输入原始命令
    char *args[MAX_LINE*3];   // 解析后命令数组
    char *history[HISTORY_MAX_SIZE]; //历史数组
    char *token;
    int should_run = 1;    // 代码是否循环变量
    int history_real = 0; //  输入命令的实际数量
    int status;

    while (should_run) {
        printf("osh> ");
        fflush(stdout);
        fgets(input, MAX_LINE, stdin);
        input[strlen(input) - 1] = '\0';

        // 检查用户输入是否为空或只包含空白字符
        int input_is_empty = 1;
        for (int i = 0; i < strlen(input); i++) {
            if (!isspace(input[i])) {
                input_is_empty = 0;
                break;
            }
        }

        if (input_is_empty) continue;

        // 历史功能（输出）
        if (strcmp(input, "!!") == 0) {
            if (history_real == 0) {
                printf("No commands in history.\n");
                continue;
            }
            strcpy(input, history[history_real - 1]);
            printf("the most recent command: %s\n", input);
        } else if (input[0] == '!') {
            int n = atoi(input + 1);
            if (n > 0 && n <= history_real) {
                strcpy(input, history[n - 1]);
                printf("the %dth command: %s\n", n, input);
            } else {
                printf("No such command in history.\n");
                continue;
            }
        }

        // 退出功能
        if (strcmp(input, "exit") == 0) {
            should_run = 0;
            continue;
        }

        // 历史功能（记录）
        if (history_real < HISTORY_MAX_SIZE) {
            history[history_real] = strdup(input);
            history_real++;
        }

        // 历史功能（输出）
        if (strcmp(input, "history") == 0){
            int start = (history_real > HISTORY_SIZE) ? (history_real - HISTORY_SIZE) : 0;
            for (int i = start; i < history_real; ++i) {
                printf("%d  %s\n", i + 1, history[i]);
            }
            continue;
        }

        // 后台并发运行功能
        int run_concurrently = 0;
        if (input[strlen(input) - 1] == '&') {
            run_concurrently = 1;
            input[strlen(input) - 1] = '\0';
        }

        // 解析指令
        token = strtok(input, " ");
        int i = 0;
        while (token != NULL) {
            args[i] = token;
            token = strtok(NULL, " ");
            i++;
        }
        args[i] = NULL;

        // 创建子进程执行
        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        } else if (pid == 0) { // 子进程执行命令
            if (execvp(args[0], args) == -1) {
                perror("Exec failed");
                exit(1);
            }
        } else { // 父进程根据是否并发选择是否等待
            if (!run_concurrently) {
                wait(&status);
            }
        }
    }
    // 释放内存
    for (int i = 0; i < history_real; i++) free(history[i]);

    return 0;
}
