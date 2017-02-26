#include <cstdio>
#include <set>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#define SizeBuff 1024

int set_nonblock(int fd)
{
    int flags;

#if defined(O_NONBLOCK)

    if(-1 == (flags = fcntl(fd, F_GETFL, 0))){ flags = 0; }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    ﻿flags = 1;
    return ioctl(fd, FIOBIO, &flags);
#endif
}

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

int main(int argc, char **argv){

    if(argc != 2){ print_err_and_exit("No port provided", __LINE__); } // second arg must be port number
    printf("Server running...\n");

    int port = atoi(argv[1]);
    if(port < 1024){ print_err_and_exit("BAD PORT NOMBER", __LINE__); }

    char buff[SizeBuff];

    int MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(MasterSocket < 0){ print_err_and_exit("Opening socket", __LINE__); }
    std::set<int> SlaveSockets;

    struct sockaddr_in SockAddr;
    init_server_sockaddr(&SockAddr, port);
    bind(MasterSocket, (const struct sockaddr*) &SockAddr, sizeof(SockAddr));
    set_nonblock(MasterSocket);
    listen(MasterSocket, SOMAXCONN);

    while (true) {

        fd_set Set;
        FD_ZERO(&Set);
        FD_SET(MasterSocket, &Set);
        for(auto &Iter:SlaveSockets){

            FD_SET(Iter, &Set);
        }

        int Max = std::max(MasterSocket, *std::max_element(SlaveSockets.begin(), SlaveSockets.end()));
        select(Max + 1, &Set, nullptr, nullptr, nullptr);

        for(auto &Iter:SlaveSockets){

            if(FD_ISSET(Iter, &Set)){

                bzero(buff, SizeBuff);
                int n_recv = recv(Iter, buff, SizeBuff, MSG_NOSIGNAL);
                if(n_recv == 0 && errno != EAGAIN){
                    shutdown(Iter, SHUT_RDWR);
                    close(Iter);
                    SlaveSockets.erase(Iter);
                }
                else{

                    printf("buff = %s len = %d\n", buff, strlen(buff));
                    if(!strcmp(buff, "close\n")){

                        printf("here\n");
                        shutdown(Iter, SHUT_RDWR);
                        close(Iter);
                    }
                    else{ int n_send = send(Iter, buff, SizeBuff, MSG_NOSIGNAL); }
                }
            }
        }
        if(FD_ISSET(MasterSocket, &Set)){

            int newSocket = accept(MasterSocket, 0, 0);
            set_nonblock(newSocket);
            SlaveSockets.insert(newSocket);
        }
    }

    return 0;
}
