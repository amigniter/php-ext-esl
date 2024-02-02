#ifndef ESL_ESLCONNECTION_H
#define ESL_ESLCONNECTION_H

#include "ESLevent.h"
#include "ESLsocket.h"

class ESLconnection : public ESLsocket {
public:
    ESLconnection(const char* host, const char* port, const char* pass);
    ESLconnection(int sockfd);
    ~ESLconnection();
    
    bool getStatus() const;
    bool getEventLock() const {return eventLock; }
    bool getAsyncMode() const {return asyncMode; }
    void setAsyncExecute(bool value);
    void setEventLock(bool value);
    void connect_inbound();
    void connect_outbound();
    void do_disconnect();
    int socketDescriptor() const;
    bool send(std::string& command);
    ESLevent* recvEvent(int timeout=0);
    ESLevent* sendRecv(std::string& command);
    ESLevent* getInfo() { return eventInfo; }
    ESLevent* sendEvent(ESLevent* event);
private:
    bool sentCommand = false;
    bool connection = false;

    bool eventLock = false;
    bool asyncMode = false;

    std::string password;

    ESLevent* eventInfo = nullptr;
    std::vector<ESLevent *> eventQueue;
};

#endif //ESL_ESLCONNECTION_H