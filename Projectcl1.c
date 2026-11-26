#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8078
#define BUFFER_SIZE 1024
#define IP INADDR_LOOPBACK

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("%s <IP> <PORT>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[2]);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Error");
        exit(-1);
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(IP);

    int c = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    if (c == -1) {
     perror("Error");
     close(sock);
     return 1;
    }

    printf("Connected\n");
    char buf[1024] = {0};
    int i = 0;
    int n;
    while (strchr(buf, '\n') == 0)
    {
       n = recv(sock, buf + i, sizeof(buf) - i - 1, 0);
       if (n <= 0) {
         perror("Error");
         exit(-1);
       }
       i += n;
    }
    buf[i] = '\0';
    printf("Buf = %s\n", buf);
    char s = buf[8];
    printf("You are %c\n", s);

    int j = 0;
    int m;
    char buf1[1024] = {0};
    while (strchr(buf1, '\n') == 0)
    {
       m = recv(sock, buf1 + j, sizeof(buf1) - j - 1, 0);
       if (m <= 0) {
         perror("Error");
         exit(-1);
       }
       j += m;
    }
    buf1[j] = '\0';
    printf("%s\n", buf1);

    fd_set readfds;
    int max_fd;
    int fd_act;
    int num;
    char buf2[1024] = {0}; 
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        FD_SET(STDIN_FILENO, &readfds);
        if (sock > STDIN_FILENO) {
            max_fd = sock;
        }
        else {
            max_fd = STDIN_FILENO;
        }

        fd_act = select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(sock, &readfds)) {
           int n = recv(sock, buf2, sizeof(buf2), 0);
           if (n == 0) {
              close(sock);
           }  
           else if (n < 0) {
              perror("Error");
              exit(-1);
           }
           else {
              buf2[n] = '\0';
              printf("%s", buf2);
           }
        }

        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            fgets(buf2, sizeof(buf2), stdin);
            num = atoi(buf2);
            if (num >= 0 && num <= 8) {
                sprintf(buf2, "MOVE %d\n", num);
                send(sock, buf2, strlen(buf2), 0);
            }
        }
    }
}
