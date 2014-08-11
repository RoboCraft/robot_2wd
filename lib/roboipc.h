//
//
// Inter-Process Communication
//
// robocraft.ru
//

#ifndef _ROBOIPC_H_
#define _ROBOIPC_H_

#include "network.h"
#include <string>

namespace roboipc {

class CommunicatorServer {
public:
    CommunicatorServer();
    ~CommunicatorServer();

    int init(const char* name);
    int close();

    SOCKET connected(int msec);

    SOCKET sockfd;
    socklen_t clilen;
#if defined(LINUX)
    struct sockaddr_un serv_addr, cli_addr;
#endif //#if defined(LINUX)

    std::string name;
    bool is_auto_close;
};

class CommunicatorClient {
public:
    CommunicatorClient();
    ~CommunicatorClient();

    int connect(const char* name);
    int close();

    int available(int msec);
    int read(void *ptr, int count);
    int write(const void *ptr, int len);

    SOCKET sockfd;
#if defined(LINUX)
    struct sockaddr_un soc_addr;
#endif //#if defined(LINUX)

    std::string name;
    bool is_auto_close;
};

}; //namespace roboipc {

#endif //#ifndef _ROBOIPC_H_
