#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/socket.domain"

void err_quit(char *estr) {
    perror(estr);
    exit(-1);
}

int main(int argc, char *argv[]) {
    char buf[1024] = {0};
    int sockfd = -1;
    int rv = -1;
    struct sockaddr_un servaddr;
    socklen_t addrlen = sizeof(servaddr);

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        err_quit("socket create failure");
    }

    printf("Create sockfd[%d] ok\n", sockfd);

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sun_family = AF_UNIX;
    strncpy(servaddr.sun_path, SOCKET_PATH, sizeof(servaddr.sun_path) - 1);

    if (connect(sockfd, (struct sockaddr *) &servaddr, addrlen) < 0)
        printf("Connect to unix domain socket server on \"%s\" failure:%s\n", SOCKET_PATH, strerror(errno));

    printf("connect unix domain socket \"%s\" ok!\n", SOCKET_PATH);

    fgets(buf, sizeof(buf), stdin);

    if ((rv = write(sockfd, buf, strlen(buf))) < 0) {
        printf("Write to server failure:%s\n", strerror(errno));
        close(sockfd);
        return -1;
    }

    printf("Actually write %d bytes data to server:%s\n", rv - 1, buf);

    bzero(&buf, sizeof(buf));
    printf("start read\n");

    if ((rv = read(sockfd, buf, sizeof(buf))) < 0) {
        printf("Read to server failure:%s\n", strerror(errno));
        close(sockfd);
        return -1;
    } else if (0 == rv) {
        printf("socket connet disconnected\n");
        close(sockfd);
        return -3;
    }

    printf("Read %d bytes data from server:%s\n", rv - 1, buf);

    close(sockfd);

    return 0;

}
