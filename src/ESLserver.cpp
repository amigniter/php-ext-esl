#include "ESLserver.h"
#include <stdexcept>
#include <cstring> // Include for strerror

ESLserver::ESLserver(const char* host, const char* port) {
    std::string error;
    struct addrinfo hints, *result;
    int reuse = 1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;

    try {
        if (getaddrinfo(host, port, &hints, &result) != 0) {
            throw std::runtime_error("Error resolving hostname: " + std::string(strerror(errno)));
        }

        servSock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

        if (servSock < 0) {
            throw std::runtime_error("Error creating socket: " + std::string(strerror(errno)));
        }

        if (setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) != 0) {
            throw std::runtime_error("Socket setsockopt(SO_REUSEADDR) failed: " + std::string(strerror(errno)));
        }

        if ((bind(servSock, result->ai_addr, result->ai_addrlen)) != 0) {
            throw std::runtime_error("Socket bind failed: " + std::string(strerror(errno)));
        }

        freeaddrinfo(result);

        if ((listen(servSock, SOMAXCONN)) != 0) {
            throw std::runtime_error("Socket listen failed: " + std::string(strerror(errno)));
        }
    } catch (const std::exception& e) {
        throw;
    }
}

ESLserver::~ESLserver() {
    Disconnect();
}

int ESLserver::accept() const {
    socklen_t addr_size;
    struct sockaddr_storage servStorage;

    while(true) {
        addr_size = sizeof servStorage;
        int clntSock = ::accept(servSock, (struct sockaddr *) &servStorage, &addr_size);
        return clntSock;
    }

}


void ESLserver::Disconnect() {
    if (IsConnected()) {
        ::close(servSock);
        servSock= -1;
    }
}

void ESLserver::close(int clientSock) {
    ::close(clientSock);
}