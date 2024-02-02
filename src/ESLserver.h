#ifndef ESL_ESLSERVER_H
#define ESL_ESLSERVER_H

#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <errno.h>
#include <string.h>

class ESLserver {

public:
    ESLserver(const char* host, const char* port);
    ~ESLserver();
    bool IsConnected() const {return servSock != -1;}
    void Disconnect();
    int accept() const;
    void close(int clientSock);

private:
    std::string host;
    std::string port;
    int servSock = -1;
};


#endif //ESL_ESLSERVER_H