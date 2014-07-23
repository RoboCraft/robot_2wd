//
//
// Inter-Process Communication
//
// robocraft.ru
//

#ifndef _ROBOIPC_H_
#define _ROBOIPC_H_

#include "network.h"

namespace roboipc {

class Communicator {
public:

    enum {SERVER=0, CLIENT};

    Communicator();
    ~Communicator();

    int init(const char* name, int type=SERVER);
    int close();
    int srv_close();
    int cli_close();

    int connected(int msec);
    int available(int msec, int sock=SOCKET_ERROR);

    int read(void *ptr, int count);
    int write(const void *ptr, int len);

    int get_socket_by_type();

    int type;

    int serv_sockfd, cli_sockfd;
    socklen_t servlen, clilen;
    struct sockaddr_un  serv_addr, cli_addr;

    char *name;
    bool is_auto_close;
};

}; //namespace roboipc {

#endif //#ifndef _ROBOIPC_H_
