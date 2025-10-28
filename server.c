#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#define PORT 8000
#define BUFFER_SIZE 1024
#define MAGIC_NUMBER 0xDEADBEEF

void error_handling(char *message);

int main(int argc, char *argv[]) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    char buffer[BUFFER_SIZE];
    int opt = 1;

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1) {
        error_handling("socket() error");
    }

    if (setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        error_handling("setsockopt() error");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        error_handling("bind() error");
    }

    if (listen(serv_sock, 5) == -1) {
        error_handling("listen() error");
    }

    printf("Server started on port %d\n", PORT);

    clnt_addr_size = sizeof(clnt_addr);
    while (1) {
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        if (clnt_sock == -1) {
            error_handling("accept() error");
            continue;
        }

        printf("Client connected: %s\n", inet_ntoa(clnt_addr.sin_addr));

        while (1) {
            int recv_len = 0;
            uint32_t magic;

            while (recv_len < sizeof(magic)) {
                int len = read(clnt_sock, buffer + recv_len, BUFFER_SIZE - recv_len);
                if (len <= 0) {
                    break;
                }
                recv_len += len;
            }

            if (recv_len <= 0) {
                printf("Client disconnected\n");
                break;
            }

            magic = *((uint32_t*)buffer);
            if (magic != MAGIC_NUMBER) {
                printf("Invalid magic number: 0x%X\n", magic);
                continue;
            }

            const char *ack = "ACK";
            write(clnt_sock, ack, strlen(ack));
        }

        close(clnt_sock);
    }

    close(serv_sock);
    return 0;
}

void error_handling(char *message) {
    perror(message);
    exit(1);
}
