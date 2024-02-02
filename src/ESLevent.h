#ifndef ESL_ESLEVENT_H
#define ESL_ESLEVENT_H

#include <string>
#include <vector>
#include <algorithm>

#include "cJSON/cJSON.h"
#include <stdexcept>
#include <iostream>
#include <sstream>

struct Header {
    std::string key;
    std::string value;
};

using Headers = std::vector<Header>;

class ESLevent {

public:
    ESLevent() = default;
    explicit ESLevent(const std::string& event_type, const std::string& subclass);
    explicit ESLevent(std::vector<std::string>& event);
    ~ESLevent();

    std::string getHeader(const std::string& headerName);
    std::string getType();
    std::string getBody();
    bool addHeader(const std::string& headerName, const std::string& value);
    void addBody(const std::string& bodyValue);
    std::string serialize(const std::string& type);
    std::string firstHeader();
    std::string nextHeader();
    bool delHeader(const std::string& headerName);
    bool setPriority(const std::string& priority);
private:
    Headers headers;
    Headers::iterator currentHeader;
    std::string body;
    void convertPlainEvent();
};

#endif //ESL_ESLEVENT_H