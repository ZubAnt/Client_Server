#include <cstdio>
#include <set>
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

static bool is_closing(const char *msg){

    char msg_close[] = "close";
    size_t len = strlen(msg_close);
    for(size_t i = 0; i < len; ++i){
        if(msg[i] != msg_close[i]){
            return false;
        }
    }
    return true;
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
    std::set<int> rm_SlaveSockets;

    struct sockaddr_in SockAddr;
    init_server_sockaddr(&SockAddr, port);
    bind(MasterSocket, (const struct sockaddr*) &SockAddr, sizeof(SockAddr));
    set_nonblock(MasterSocket);
    listen(MasterSocket, SOMAXCONN);

    fd_set Set;

    while (true) {

        FD_ZERO(&Set);
        FD_SET(MasterSocket, &Set);
        for(auto &Iter:SlaveSockets){

            FD_SET(Iter, &Set);
        }

//        int Max = std::max(MasterSocket, *std::max_element(SlaveSockets.begin(), SlaveSockets.end()));
        int Max = MasterSocket;
        if (!SlaveSockets.empty()){ Max = *SlaveSockets.rbegin(); }
        select(Max + 1, &Set, nullptr, nullptr, nullptr);

        for(auto Iter = SlaveSockets.begin(); Iter != SlaveSockets.end(); ++Iter){

            if(FD_ISSET(*Iter, &Set)){

                bzero(buff, SizeBuff);
                if(*Iter == 4){
                    close(*Iter);
                }
                int n_recv = recv(*Iter, buff, SizeBuff, MSG_NOSIGNAL);
                printf("buff = %s, len = %zu, n_recv = %d\n", buff, strlen(buff), n_recv);

                if(n_recv == -1){

                    printf("ERROR: NO READ; n_recv == -1; Line = %d\n", __LINE__);
                    shutdown(*Iter, SHUT_RDWR);
                    close(*Iter);
                    rm_SlaveSockets.insert(*Iter);
                }
                if(n_recv == 0){ // Клиент разорвал соединение

                    printf("ERROR: NO READ; Client disconnected; n_recv == 0 && errno != EAGAIN "
                           "Line = %d\n", __LINE__);
                    shutdown(*Iter, SHUT_RDWR);
                    close(*Iter);
                    rm_SlaveSockets.insert(*Iter);
                }
                else if(n_recv > 0){

                    printf("n_recv > 0\n");
                    if(is_closing(buff)){

                        printf("closing\n");
                        shutdown(*Iter, SHUT_RDWR);
                        close(*Iter);
                        rm_SlaveSockets.insert(*Iter);
                    }
                    else{send(*Iter, buff, n_recv, MSG_NOSIGNAL); }
                }
            }
        }
        for(auto Iter = rm_SlaveSockets.begin(); Iter != rm_SlaveSockets.end(); ++Iter){
            SlaveSockets.erase(*Iter);
        }
        rm_SlaveSockets.clear();


        if(FD_ISSET(MasterSocket, &Set)){

            int newSocket = accept(MasterSocket, 0, 0);
            set_nonblock(newSocket);
            SlaveSockets.insert(newSocket);
        }
    }

    return 0;
}
