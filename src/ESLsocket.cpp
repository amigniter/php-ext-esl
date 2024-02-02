#include "ESLsocket.h"

ESLsocket::ESLsocket(const char* host, const char* port) : host(host), port(port) {}

ESLsocket::ESLsocket(int sockfd) : sockfd_(sockfd) {}

int ESLsocket::getHandle() const {
    return socket.SocketHandle();
}

bool ESLsocket::connect() {
    return socket.Connect(host, port);
}

void ESLsocket::disconnect() {
    if(socket.IsConnected()) {
        socket.Disconnect();
    }
}

bool ESLsocket::attach(TCPStreamBuffer& stream) const {
    if(sockfd_ != -1) {
        return stream.Connect(sockfd_);
    } else {
        return false;
    }
}

bool ESLsocket::sendCmd(std::string &command) {
    if(!socket.IsConnected())
        return false;

    socket.resetSend();

    command += "\n\n";
    std::iostream stream(&socket);

    stream << command;
    stream.flush();

    if(socket.IsSent())
        return true;
    
    return false;
}