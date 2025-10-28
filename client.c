#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdint.h>

#define DEFAULT_SERVER_IP "127.0.0.1"
#define PORT 8000
#define BUFFER_SIZE 1024
#define MAGIC_NUMBER 0xDEADBEEF
#define PACKET_SIZE sizeof(uint32_t)
#define MAX_PINGS 10

void error_handling(char *message);
double get_current_time();

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
    const char *server_ip = DEFAULT_SERVER_IP;
    int ping_count = MAX_PINGS;

    if (argc > 1) {
        server_ip = argv[1];
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        error_handling("socket() error");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(server_ip);
    serv_addr.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        error_handling("connect() error");
    }

    printf("Connected to server %s\n", server_ip);

    uint32_t magic = MAGIC_NUMBER;
    int ack_len = 0;
    char ack_buffer[4];  // "ACK" + null terminator

    for (int i = 0; i < ping_count; i++) {
        double start_time, end_time, rtt;

        start_time = get_current_time();

        if (write(sock, &magic, sizeof(magic)) != sizeof(magic)) {
            error_handling("write() error");
        }

        ack_len = 0;
        while (ack_len < 3) {
            int len = read(sock, ack_buffer + ack_len, sizeof(ack_buffer) - ack_len - 1);
            if (len <= 0) {
                error_handling("read() error or server disconnected");
            }
            ack_len += len;
        }
        end_time = get_current_time();
        rtt = (end_time - start_time) * 1000;

        ack_buffer[ack_len] = '\0';
        printf("Ping %d: RTT = %.3f ms\n", i, rtt);

        usleep(1000000);
    }

    close(sock);
    return 0;
}

void error_handling(char *message) {
    perror(message);
    exit(1);
}

double get_current_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}
