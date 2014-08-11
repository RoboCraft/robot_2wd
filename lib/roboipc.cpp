//
//
// Inter-Process Communication
//
// robocraft.ru
//

#include "roboipc.h"

roboipc::CommunicatorServer::CommunicatorServer():
    sockfd(SOCKET_ERROR), is_auto_close(true)
{
}

roboipc::CommunicatorServer::~CommunicatorServer()
{
    if(is_auto_close) {
        close();
    }
}

int roboipc::CommunicatorServer::init(const char *_name)
{

    if(!_name)
        return -1;

    name = _name;

#if defined(LINUX)
    sockfd = network_create_UNIX_server(name.c_str(), &serv_addr);
#endif //#if defined(LINUX)

    if(sockfd == SOCKET_ERROR) {
        fprintf(stderr, "[!] Error: can't create %s!\n", name.c_str());
        return -1;
    }

    return 0;
}

int roboipc::CommunicatorServer::close()
{
    network_close_connection(sockfd);
    sockfd = SOCKET_ERROR;
#if defined(LINUX)
    unlink(name.c_str());    // remove socket file
#endif //#if defined(LINUX)

    return 0;
}

SOCKET roboipc::CommunicatorServer::connected(int msec)
{
    if(sockfd == SOCKET_ERROR) {
        return -1;
    }

    SOCKET cli_sockfd = SOCKET_ERROR;

#if defined(LINUX)
    fd_set rfds;
    struct timeval tv;
    tv.tv_sec = msec / 1000;
    tv.tv_usec = (msec % 1000) * 1000;

    if( tv.tv_usec > 1000000 ) {
        tv.tv_sec++;
        tv.tv_usec -= 1000000;
    }

    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);

    clilen = sizeof(struct sockaddr_un);

    if( select(sockfd+1, &rfds, NULL, NULL, &tv) > 0 ) {
        cli_sockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (cli_sockfd < 0 ) {
            printf("[!] Error: accepting!\n");
        }
        else {
            printf("[i] Client connected...\n");
        }
    }
#endif //#if defined(LINUX)

    return cli_sockfd;
}

// =======================================================

roboipc::CommunicatorClient::CommunicatorClient():
    sockfd(SOCKET_ERROR), is_auto_close(true)
{
}

roboipc::CommunicatorClient::~CommunicatorClient()
{
    if(is_auto_close) {
        close();
    }
}

int roboipc::CommunicatorClient::connect(const char *_name)
{
    if(!_name) {
        return -1;
    }
    name = _name;

#if defined(LINUX)
    sockfd = network_connect_UNIX_socket(name.c_str());
#endif //#if defined(LINUX)
    if(sockfd == SOCKET_ERROR) {
        fprintf(stderr, "[!] Error: can't connect to %s!\n", name.c_str());
        return -1;
    }
    return 0;
}

int roboipc::CommunicatorClient::close()
{
    network_close_connection(sockfd);
    sockfd = SOCKET_ERROR;

    return 0;
}

int roboipc::CommunicatorClient::available(int msec)
{
    if(sockfd == SOCKET_ERROR) {
        return -1;
    }

    fd_set rfds;
    struct timeval tv;
    tv.tv_sec = msec / 1000;
    tv.tv_usec = (msec % 1000) * 1000;

    if( tv.tv_usec > 1000000 ) {
        tv.tv_sec++;
        tv.tv_usec -= 1000000;
    }

    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);
    return select(sockfd+1, &rfds, NULL, NULL, &tv);
}

int roboipc::CommunicatorClient::read(void *ptr, int count)
{
    if(!ptr || count <= 0) {
        return -1;
    }

    if(sockfd == SOCKET_ERROR) {
        return -1;
    }

#if defined(LINUX)
    return ::read(sockfd, ptr, count);
#else
    return 0;
#endif //#if defined(LINUX)
}

int roboipc::CommunicatorClient::write(const void *ptr, int len)
{
    if(!ptr || len <= 0) {
        return -1;
    }

    if(sockfd == SOCKET_ERROR) {
        return -1;
    }

#if defined(LINUX)
    return ::write(sockfd, ptr, len);
#else
    return 0;
#endif //#if defined(LINUX)
}
