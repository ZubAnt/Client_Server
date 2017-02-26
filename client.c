#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

#include "admin.h"

#define SizeBuff 1024

static void print_err_and_exit(const char *msg, int line)
{
    printf("ERROR: %s, LINE = %d\n", msg, line);
    exit(1);
}

static void init_server_sockaddr(struct sockaddr_in *server, const char *ip_addr, int port_nomber)
{
    struct hostent *server_data;
    server_data = gethostbyname(ip_addr);
    if (server == NULL) { print_err_and_exit("No such host", __LINE__); }

    bzero((char *) server, sizeof(server));
    server->sin_family = AF_INET;
    bcopy((char *)server_data->h_addr,
          (char *) &(server->sin_addr.s_addr),
          server_data->h_length);
    server->sin_port = htons(port_nomber);
}

int main(int argc, char *argv[])
{
    if (argc != 2) { print_err_and_exit("Usage \"name_program\" \"port\"", __LINE__); }

    struct sockaddr_in server;
    char buffer[SizeBuff];

    int port = atoi(argv[1]);
    if(port < 1024){ print_err_and_exit("BAD PORT NOMBER", __LINE__); }

    int MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(MasterSocket < 0){ print_err_and_exit("Opening socket", __LINE__);}

    init_server_sockaddr(&server, SERVER_IP, port);
    int con_flag = connect(MasterSocket, (struct sockaddr *) &server, sizeof(server));
    if(con_flag == -1){ print_err_and_exit("No connected", __LINE__); }

    printf("Please enter the message: ");
    bzero(buffer, SizeBuff);
    fgets(buffer, SizeBuff, stdin);

    int n_send = send(MasterSocket, buffer, strlen(buffer), MSG_NOSIGNAL);
    if(n_send < 0){ print_err_and_exit("No send to server", __LINE__); }

    bzero(buffer, SizeBuff);
    int n_recv = recv(MasterSocket, buffer, SizeBuff, MSG_NOSIGNAL);
    if (n_recv < 0){ print_err_and_exit("No recv to server", __LINE__); }

    printf("Message from srevr: %s\n",buffer);

    shutdown(MasterSocket, SHUT_RDWR);
    close(MasterSocket);
    return 0;
}
