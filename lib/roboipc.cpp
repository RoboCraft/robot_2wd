//
//
// Inter-Process Communication
//
// robocraft.ru
//

#include "roboipc.h"

#include <string.h>

robolibrary::Communicator::Communicator():
    serv_sockfd(SOCKET_ERROR), cli_sockfd(SOCKET_ERROR)
{

}

robolibrary::Communicator::~Communicator()
{
    close();
}

int robolibrary::Communicator::init(const char* _name, int _type)
{
    if(!_name)
        return -1;

    int name_len = strlen(_name);
    name = new char[name_len+1];
    if(!name) {
        return -1;
    }
    strncpy(name, _name, name_len);

    type = _type;

    if(type == SERVER) {
        serv_sockfd = network_create_UNIX_server(name, &serv_addr);
        if(serv_sockfd == SOCKET_ERROR) {
            fprintf(stderr, "[!] Error: can't connect to %s!\n", name);
            return -1;
        }
        clilen = sizeof(struct sockaddr_un);

    }
    else if(type == CLIENT) {
        cli_sockfd = network_connect_UNIX_socket(name);
        if(cli_sockfd == SOCKET_ERROR) {
            fprintf(stderr, "[!] Error: can't connect to %s!\n", name);
            return -1;
        }
    }
    else {
        return -1;
    }

    return 0;
}

int robolibrary::Communicator::close()
{
    if(type == SERVER) {
        network_close_connection(serv_sockfd);
        network_close_connection(cli_sockfd);
        unlink(name);    // remove socket file
    }
    else if(type == CLIENT) {
        network_close_connection(cli_sockfd);
    }

    if(name) {
        delete []name;
        name = 0;
    }

    return 0;
}

int robolibrary::Communicator::connected(int msec)
{
    if(type == CLIENT)
        return -1;

    if( available(msec) == 0 ) {
        cli_sockfd = accept(serv_sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (cli_sockfd < 0)
            printf("[!] Error: accepting!\n");
        else {
            printf("[i] Client connected...\n");
        }
    }

    return cli_sockfd;
}

int robolibrary::Communicator::available(int msec)
{
    fd_set rfds;
    struct timeval tv;
    tv.tv_sec = msec / 1000;
    tv.tv_usec = (msec % 1000) * 1000;

    if( tv.tv_usec > 1000000 ) {
        tv.tv_sec++;
        tv.tv_usec -= 1000000;
    }

    int sockfd = get_socket_by_type();

    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);
    if( select(sockfd+1, &rfds, NULL, NULL, &tv) > 0 ) {
        return 0;
    }

    return -1;
}

int robolibrary::Communicator::read(void *ptr, int count)
{
    if(!ptr || count <= 0)
        return -1;

    int sockfd = cli_sockfd;

    if(sockfd == SOCKET_ERROR)
        return -1;

    return ::read(sockfd, ptr, count);
}

int robolibrary::Communicator::write(const void *ptr, int len)
{
    if(!ptr || len <= 0)
        return -1;

    int sockfd = get_socket_by_type();

    if(sockfd == SOCKET_ERROR)
        return -1;

    return ::write(sockfd, ptr, len);
}

int robolibrary::Communicator::get_socket_by_type()
{
    int sockfd = SOCKET_ERROR;

    if(type == SERVER) {
        sockfd = serv_sockfd;
    }
    else if(type == CLIENT) {
        sockfd = cli_sockfd;
    }

    return sockfd;
}
