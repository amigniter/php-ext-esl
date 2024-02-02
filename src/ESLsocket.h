#ifndef ESL_ESLSOCKET_H
#define ESL_ESLSOCKET_H

#include "tcp-stream-buffer/TCPStreamBuffer.h"

#include <sstream>
#include <exception>
#include <mutex>

class ESLsocket {

public:
    ESLsocket(const char* host, const char* port);
    ESLsocket(int sockfd);
    //virtual ~ESLsocket();

protected:
    TCPStreamBuffer socket;
    std::string host;
    std::string port;
    
    int sockfd_ = -1; //outbound conn

    bool connect();
    virtual void disconnect();
    bool attach(TCPStreamBuffer& stream) const;
    bool sendCmd(std::string &command);
    int getHandle() const;
};


#endif //ESL_ESLSOCKET_H