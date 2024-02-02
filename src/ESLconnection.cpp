#include "ESLconnection.h"

ESLconnection::ESLconnection(const char* host, const char* port, const char* pass)
    : password(pass), ESLsocket(host, port) {}

ESLconnection::ESLconnection(int sockfd) : ESLsocket(sockfd) {}

ESLconnection::~ESLconnection() { 
    delete eventInfo;
    do_disconnect();
}

void ESLconnection::do_disconnect() {
    disconnect();
    connection= false;
}

bool ESLconnection::getStatus() const {
    return connection;
}

int ESLconnection::socketDescriptor() const {
    return getHandle();
}

void ESLconnection::connect_outbound() {
    if(!attach(socket)) {
        throw std::runtime_error("Error connecting");
    }
    //first we send `connect`
    std::string cmd = "connect";
    ESLevent *event = sendRecv(cmd);
    if(event) {
        eventInfo = event;
        //authorized
        connection = true;
    }
}

void ESLconnection::connect_inbound() {
    std::string auth = "auth " + std::string(password);
    std::string reply;

    if(!connect()) {
        throw std::runtime_error("Error connecting to " + host + ":" + port);
    }

    ESLevent *event = recvEvent();

    if (event && event->getHeader("Content-Type") !=  "auth/request") {
        //Unexpected header on auth
        delete event;
        throw std::runtime_error("Unexpected header on auth");
    }

    delete event;
    event= nullptr;

    //send auth
    event = sendRecv(auth);
    if(event) {
        reply = event->getHeader("Reply-Text");
    }

    if(reply.find("+OK") != std::string::npos) {
        //authorized
        connection= true;
    } else {
        delete event;
        throw std::runtime_error("Connection refused");
    }

    delete event;
    event= nullptr;
}

ESLevent* ESLconnection::recvEvent(int timeout) {
    if(timeout >0) {
        socket.SetTimeout(timeout);
    }

    std::iostream stream(&socket);
    std::string line;
    std::vector<std::string> response;
    int contentLength = -1;
    ESLevent* event = nullptr;

    // Read the header lines until we encounter an empty line
    while (std::getline(stream, line) && !line.empty()) {
        if (line.find("Content-Length: ") == 0) {
            // extract the value of Content-Length from the line
            contentLength = std::stoi(line.substr(16));
        }
        // Process header lines
        response.push_back(line);
    }

    if(!response.empty()) {
        event = new ESLevent(response);
        // read the specified number of bytes into the response buffer
        if (contentLength > 0) {
            line.resize(contentLength);
            stream.read(&line[0], contentLength);
            event->addBody(line);
        }
    }

    return event;
}

ESLevent* ESLconnection::sendRecv(std::string &command) {
    std::vector<std::string> waitFor = {"api/response", "command/reply"};

    if(!send(command)) {
        //connection = false;
        return nullptr;
    }

    sentCommand=true;

    // Collect and queue all the events
    ESLevent* event = nullptr;
    std::string contentType;
    do {
        event = recvEvent();
        if(event) {
            eventQueue.push_back(event);
            contentType = event->getHeader("Content-Type");
            //if(contentType=="text/disconnect-notice")
            //    break;
        } else {
            break;
        }
    } while (std::find(waitFor.begin(), waitFor.end(), contentType) == waitFor.end());

    sentCommand=false;

    ESLevent *ev = eventQueue.back();
    eventQueue.pop_back();
    return ev;
}

bool ESLconnection::send(std::string &command) {
    if(command.empty()) {
        return false;
    }
    return sendCmd(command);
}

void ESLconnection::setAsyncExecute(bool value) {
    asyncMode = value;
}

void ESLconnection::setEventLock(bool value) {
    eventLock = value;
}

ESLevent* ESLconnection::sendEvent(ESLevent *event) {
    std::string eventName = event->getHeader("Event-Name");
    std::string serializedEvent = event->serialize("plain");
    std::string cmd_buf = "sendevent";
    cmd_buf.append(" " + eventName + "\n" + serializedEvent);
    ESLevent *ev= sendRecv(cmd_buf);
    return ev;
}