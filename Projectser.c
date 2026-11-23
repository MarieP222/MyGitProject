#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <time.h>

#define IP INADDR_ANY
#define PORT 8078
#define BUFFER_SIZE 1024

void printboard(char board[][3])
{
   int k = 0;
   for (int i = 0; i < 3; ++i)
   {
      for (int j = 0; j < 3; ++j) 
      {
        if (board[i][j] == ' ') {
            printf("%d", k);
            if (j < 2) {
                printf(" | ");
            }
        }
        else {
          printf("%c", board[i][j]);
           if (j < 2) {
                printf(" | ");
            }
        }
        k++;
      }
      printf("\n");
      if (i < 2) {
        printf("---------------\n");
      }
   }
}
int main(int argc, char *argv[])
{
   if (argc != 2) {
     printf("Usage: %s <port>\n", argv[0]);
     return 1;
    }

    int port = atoi(argv[1]);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Error");
        exit(-1);
    }

    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
      perror("setsockopt");
      exit(1);
    }
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(IP);

    int b = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    if (b == -1) {
        perror("Error");
        exit(-1);
    }

    int l = listen(sock, 3);
    if (l == -1) {
        perror("Error");
        exit(-1);
    }

    int clients[2] = {-1, -1};
    fd_set readfds;
    int max_fd;
    int active;
    int k;
    char buf[1024] = {0};
    int started = 0;
    char board[3][3];
    int qayl;
    int hert = 0;
    int win = 0;
    int full = 1;
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        board[i][j] = ' ';
      }
    }
    printboard(board);
    while (1) {
     max_fd = sock;
     FD_ZERO(&readfds);
     FD_SET(sock, &readfds);

     for (int i = 0; i < 2; ++i)
     {
        if (clients[i] != -1) {
            FD_SET(clients[i], &readfds);

            if (clients[i] > max_fd) {
                max_fd = clients[i];
            }
        }
     }

     active = select(max_fd + 1, &readfds, NULL, NULL, NULL);

     if (active == -1) {
        perror("select");
        continue;
     }

    if (FD_ISSET(sock, &readfds)) {
        socklen_t len = sizeof(addr);
        k = accept(sock, (struct sockaddr*)&addr, &len);

        if (k == -1) {
            perror("accept");
            continue;
        }

        int i;
        for (i = 0; i < 2; ++i)
        {
            if (clients[i] == -1) {
                clients[i] = k;
                if (i == 0) {
                   send(clients[0], "WELCOME X\n", 10, 0);
                   send(clients[0], "WAIT\n", 5, 0);
                }
                else if (i == 1) {
                   send(clients[1], "WELCOME O\n", 10, 0);
                }
                break;
            }
        }

        if (i == 2) {
            close(k);
        }
     }
    if (clients[0] != -1 && clients[1] != -1 && started == 0) {
     send(clients[0], "START 3 3\n", 10, 0);
     send(clients[1], "START 3 3\n", 10, 0);
     send(clients[0], "TURN X\n", 7, 0);
     send(clients[1], "TURN X\n", 7, 0);

     printf("TURN X\n");
     printboard(board);
     started = 1;
    }
     if (clients[0] != -1 && clients[1] != -1) {
      for (int i = 0; i < 2; ++i)
      {
        int r;
        if (clients[i] != -1 && FD_ISSET(clients[i], &readfds))
        {
            r = recv(clients[i], buf, 1023, 0);

            if (r < 1) {
                close(clients[i]);
                clients[i] = -1;
                continue;
            }

            buf[r] = '\0';

        qayl = atoi(&buf[5]);
        int r1 = qayl / 3;
        int c = qayl % 3;

    if (qayl >= 0 && qayl <= 8) {

      if (i != hert) {
        send(clients[i], "ERROR NOT_YOUR_TURN\n", 20, 0);
      }
      else if (board[r1][c] != ' ') {
        send(clients[i], "ERROR OCCUPIED\n", 15, 0);
      }
      else {
        if (i == 0)
            board[r1][c] = 'X';
        else
            board[r1][c] = 'O';

        send(clients[i], "OK MOVE\n", 8, 0);

        printboard(board);

        if (i == 0 && clients[1] != -1) {
            send(clients[1], buf, r, 0);
        }
        else if (i == 1 && clients[0] != -1) {
            send(clients[0], buf, r, 0);
        }

        for (int i = 0; i < 3; ++i) {
            if ((board[i][0] == board[i][1] &&
                 board[i][1] == board[i][2] &&
                 board[i][0] != ' ') ||

                (board[0][i] == board[1][i] &&
                 board[1][i] == board[2][i] &&
                 board[0][i] != ' ')) {

                win = 1;
                printf("Winner is %c\n",
                    board[i][0] == board[i][1] &&
                    board[i][1] == board[i][2] ?
                    board[i][0] : board[0][i]);
            }
        }

        if ((board[0][0] == board[1][1] &&
             board[1][1] == board[2][2] &&
             board[1][1] != ' ') ||

            (board[0][2] == board[1][1] &&
             board[1][1] == board[2][0] &&
             board[1][1] != ' ')) {

            win = 1;
            printf("Winner is %c\n", board[1][1]);
        }

        if (win) {
            send(clients[0], "Game over\n", 10, 0);
            send(clients[1], "Game over\n", 10, 0);

            close(clients[0]);
            close(clients[1]);
            return 0;
        }

        full = 1;

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                if (board[i][j] == ' ') {
                    full = 0;
                }
            }
        }

        if (full && !win) {
            send(clients[0], "Havasar\n", 8, 0);
            send(clients[1], "Havasar\n", 8, 0);

            close(clients[0]);
            close(clients[1]);
            return 0;
        }
        hert = 1 - hert;

        if (hert == 0) {
            send(clients[0], "TURN X\n", 7, 0);
            send(clients[1], "TURN X\n", 7, 0);
        }
        else {
            send(clients[0], "TURN O\n", 7, 0);
            send(clients[1], "TURN O\n", 7, 0);
        }
    }
}
         else {
          send(clients[i], "ERROR INVALID_MOVE\n", 19, 0);
         }
       }
      }
    }
   }
}
