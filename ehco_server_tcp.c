#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SizeBuff 1024

static void print_err_and_exit(const char *msg, int line)
{
    printf("ERROR: %s, LINE = %d\n", msg, line);
    exit(1);
}

static void init_server_sockaddr(struct sockaddr_in *server, int port_nomber)
{
    bzero((char *) server, sizeof(server));
    server->sin_family = AF_INET;
    server->sin_addr.s_addr = INADDR_ANY;
    server->sin_port = htons(port_nomber);
}

int main(int argc, char *argv[])
{
    if(argc != 2){ print_err_and_exit("No port provided", __LINE__); } // second arg must be port number
    printf("Server running...\n");

    char buff[SizeBuff];
    struct sockaddr_in server, client;
    socklen_t SockAddr_len = sizeof(client);

    int MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(MasterSocket < 0){ print_err_and_exit("Opening socket", __LINE__); }

    int port = atoi(argv[1]);
    init_server_sockaddr(&server, port);
    bind(MasterSocket, (const struct sockaddr *) &server, sizeof(server));
    listen(MasterSocket, 5);

    while(1){

        int SlaveSocket = accept(MasterSocket, (struct sockaddr *) &client, &SockAddr_len);
        if(SlaveSocket < 0){ print_err_and_exit("No accept", __LINE__); }

        bzero(buff, SizeBuff);
        int n_read = recv(SlaveSocket, buff, SizeBuff, MSG_NOSIGNAL);
        if(n_read < 0){ print_err_and_exit("NO reading from socket", __LINE__); }

        if(!strcmp(buff, "close")){

            shutdown(SlaveSocket, SHUT_RDWR);
            close(SlaveSocket);
            break;
        }
        else{ printf("Receive %d bytes; Message: %s\n", n_read, buff); }

        int n_write = send(SlaveSocket, buff, strlen(buff), MSG_NOSIGNAL);
        if(n_write < 0){ print_err_and_exit("NO writing to socket", __LINE__); }
        else{ printf("Send %d bytes; Message: %s\n", n_read, buff); }

        shutdown(SlaveSocket, SHUT_RDWR);
        close(SlaveSocket);
    }

    close(MasterSocket);

    return 0;
}
