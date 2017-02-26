#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void err(const char *msg){
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    if(argc != 2){

        err("ERROR, no port provided\n");
    }

    printf("server running...\n");

    size_t SizeBuff = 1024;
    char buff[SizeBuff];

    int server_fd = 0;
    int accept_fd = 0;
    int port = 0;
    int numb_read = 0;
    int numb_write = 0;

    struct sockaddr_in server, client;
    socklen_t client_len = sizeof(client);

    server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(server_fd < 0){

        err("ERROR opening socket\n");
    }

    port = atoi(argv[1]);
//    port = 3047;
    bzero((char *) &server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    bind(server_fd, (const struct sockaddr *) &server, sizeof(server));
    listen(server_fd, 5);

    accept_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);
    if(accept_fd < 0){

        err("ERROR on accept\n");
    }

    while(1){

        bzero(buff, SizeBuff);
        numb_read = read(accept_fd, buff, SizeBuff);

        if(numb_read < 0){

            err("ERROR reading from socket\n");
        }

        if(!strcmp(buff, "close")){

            break;
        }

        numb_write = write(accept_fd, buff, strlen(buff));
        if(numb_write < 0){

            err("ERROR writing to socket");
        }
    }

    close(accept_fd);
    close(server_fd);

    return 0;
}
