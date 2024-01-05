#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <locale>

#define SOCKET_PATH "/tmp/socket.domain"

void err_quit(char *estr) {
    perror(estr);
    exit(-1);
}

int main(int argc, char *argv[]) {
    char buf[1024];
    int i;
    int listen_fd = -1;
    int client_fd;
    int rv = -1;
    struct sockaddr_un servaddr;   //unix域socket套接字
    struct sockaddr_un cliaddr;
    socklen_t addrlen = sizeof(servaddr);

    listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        err_quit("socket create fail");
    }

    printf("create sockfd[%d] ok!\n", listen_fd);

    if (!access(SOCKET_PATH, F_OK)) {
        remove(SOCKET_PATH);
    }

    bzero(&servaddr, addrlen);

    servaddr.sun_family = AF_UNIX;
    strncpy(servaddr.sun_path, SOCKET_PATH, sizeof(servaddr.sun_path));

    if (bind(listen_fd, (struct sockaddr *) &servaddr, addrlen) < 0) {
        printf("Create socket failure:%s\n", strerror(errno));
        unlink(SOCKET_PATH);
        return -1;
    }

    listen(listen_fd, 13);

    while (1) {
        printf("Start waiting and accept new client connect......\n");
        client_fd = accept(listen_fd, (struct sockaddr *) &cliaddr, &addrlen);
        if (client_fd < 0) {
            printf("Accept new client failure:%s\n", strerror(errno));
            return -2;
        }

        memset(buf, 0, sizeof(buf));

        if ((rv = read(client_fd, buf, sizeof(buf))) < 0) {
            printf("Read from client[%d] failure:%s\n", client_fd, strerror(errno));
            close(client_fd);
            continue;
        } else if (rv == 0) {
            printf("socket connet disconneted\n");
            close(client_fd);
            continue;
        }

        printf("Read massage from client[%d]:%s\n", listen_fd, buf);

        for (i = 0; i < rv; i++) {
            buf[i] = toupper(buf[i]);
        }

        if (write(client_fd, buf, rv) < 0) {
            printf("Write to client[%d] failure:%s\n", client_fd, strerror(errno));
            close(client_fd);
            continue;
        }

        printf("Write %d bytes data to client[%d]\n", rv - 1, client_fd);

        close(client_fd);

        sleep(1);
    }

    close(listen_fd);


}


