#include "ESLevent.h"
extern "C" {
    #include "php.h"
}
std::string trim(const std::string &s);
std::vector<std::string> splitLine (const std::string &s, char delim);
std::vector<Header> parseXmlString(const std::string& xmlString, std::string& body);

ESLevent::~ESLevent() = default;

ESLevent::ESLevent(const std::string& event_type, const std::string& subclass) {
    headers.emplace_back(Header{"Event-Name", event_type});

    if (!subclass.empty()) {
        headers.emplace_back(Header{"Event-Subclass", subclass});
    }
}

ESLevent::ESLevent(std::vector<std::string>& event) {
    char delim = ':';
    for(auto &line: event) {
        if(line.empty() || line == "\n")
            continue;
        else if (line.find(delim) != std::string::npos ) {
            int pos = line.find_first_of(':');
            std::string key,val;
            key = line.substr(0, pos);
            val = line.substr(pos+1);
            addHeader(trim(key),trim(val));
        } else {
            addBody(line);
        }
    }
    convertPlainEvent();
}

bool ESLevent::addHeader(const std::string& headerName, const std::string& value) {
    try {
        headers.emplace_back(Header{headerName, value});
    } catch (const std::exception &) {
        return false;
    }
    return true;
}

std::string ESLevent::getHeader(const std::string &headerName) {
    auto it = std::find_if(headers.begin(), headers.end(),
                           [&](const auto& header) { return header.key == headerName; });
    if (it != headers.end()) {
        return it->value;
    } else {
        return std::string{};
    }
}

std::string ESLevent::getType() {
    std::string evName = getHeader("Event-Name");
    if(!evName.empty()) {
        return evName;
    } else {
        return "invalid";
    }
}

void ESLevent::addBody(const std::string& bodyValue) {
    if(body.empty()) {
        body = bodyValue;
    } else {
        body += bodyValue;
    }
    convertPlainEvent();
}

std::string ESLevent::getBody() {
    return body;
}

std::string ESLevent::serialize(const std::string& type) {
    std::string contentType = this->getHeader("Content-Type");
    std::string reply;
    if(type == "plain") {
        for (auto& header : headers) {
            if (header.key == "Content-Type" || header.key == "Content-Length")
                continue;
            reply += header.key + ": " + header.value + "\n";
        }
    } else if (type == "json") {
        cJSON* root = cJSON_CreateObject();
        for (auto& header : headers) {
            if (header.key == "Content-Type" || header.key == "Content-Length")
                continue;
            if(root)
                cJSON_AddStringToObject(root, header.key.c_str(), header.value.c_str());
        }
        if(!body.empty()) {
            cJSON_AddStringToObject(root, "Content-Length", std::to_string(body.size()).c_str() );
            cJSON_AddStringToObject(root, "_body", body.c_str() );
        }
        char* jsonString = cJSON_Print(root);
        reply = jsonString;

        free(jsonString);
        cJSON_Delete(root);
    } else if (type == "xml") {
        std::stringstream xmlStream;
        xmlStream << "<event>\n"
            << "  <headers>\n";
        for (const auto& header : headers) {
            if (header.key == "Content-Type" || header.key == "Content-Length")
                continue;
            xmlStream << "    <" << header.key << ">" << header.value << "</" << header.key << ">\n";
        }

        xmlStream << "  </headers>\n";

        if (!body.empty()) {
            xmlStream << "  <Content-Length>" << std::to_string(body.size()) << "</Content-Length>\n";
            xmlStream << "  <body>" << body << "</body>\n";
        }

        xmlStream << "</event>\n";
        
        reply= xmlStream.str();
    }
    
    if(!body.empty()) {
        if(type == "plain") {
            reply += "Content-Length: " + std::to_string(body.size()) + "\n\n";
            reply += body;
        }
    }
    return reply;
}

/* trim and split methods */
std::string ltrim(const std::string &s)
{
    size_t start = s.find_first_not_of(" \r\n");
    return (start == std::string::npos) ? "" : s.substr(start);
}

std::string rtrim(const std::string &s)
{
    size_t end = s.find_last_not_of(" \r\n");
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string trim(const std::string &s) {
    return rtrim(ltrim(s));
}

std::vector<std::string> splitLine(const std::string& s, char delim)
{
    std::vector<std::string> result;
    size_t start = 0, end = 0;

    while ((end = s.find(delim, start)) != std::string::npos) {
        result.push_back(s.substr(start, end - start));
        start = end + 1;
    }
    result.push_back(s.substr(start));

    return result;
}

/* convert plain event */
void ESLevent::convertPlainEvent() {
    std::string contentType = getHeader("Content-Type");
    if(contentType == "text/event-plain") {
        if(body.empty())
            return;
        std::string nbody = body;
        body.clear();

        std::vector<std::string> lines= splitLine(nbody, '\n');
        std::vector<Header> headers_;
        bool gotContent = false;

        for(auto &line: lines) {
            if (line == "\n" /*|| line.empty()*/)
                continue;

            if(!gotContent) {
                int pos = line.find_first_of(':');
                if(pos>0) {
                    std::string key, val;
                    key = line.substr(0, pos);
                    val = line.substr(pos + 1);
                    if(key == "Content-Length") {
                        gotContent = true;
                    }
                    headers_.emplace_back(Header{trim(key), trim(val)});
                }
            } else if(!line.empty()) {
                body.append(line);
            }
        }
        // add headers in the correct order
        for (auto& header : headers_) {
            addHeader(header.key, header.value);
        }
    } else if(contentType == "text/event-json") {
        if(body.empty())
            return;
        std::string nbody = body;
        body.clear();

        cJSON* jsonData = cJSON_Parse(nbody.c_str());
        cJSON* item = NULL;
        std::vector<Header> headers_;
        cJSON_ArrayForEach(item, jsonData) {
            if (item->type == cJSON_String) {
                std::string key(item->string);
                std::string value(item->valuestring);
                if(key == "_body") {
                    body.append(value); 
                } else {
                    headers_.emplace_back(Header{trim(key), trim(value)});
                }
            }
        }

        cJSON_Delete(jsonData);

        for (auto& header : headers_) {
            addHeader(header.key, header.value);
        }
    } else if(contentType == "text/event-xml") {
        if(body.empty())
            return;
        std::string nbody = body;
        body.clear();

        std::vector<Header> headers_;

        try {
            headers_= parseXmlString(nbody, body);
        } catch (const std::exception &e) {
            //zend_throw_exception(NULL, e.what(), 0);
        }
        for (auto& header : headers_) {
            addHeader(header.key, header.value);
        }
    }
}

std::vector<Header> parseXmlString(const std::string& xmlString, std::string& body) {
    std::vector<Header> headers;

    std::size_t eventStart = xmlString.find("<event>");
    std::size_t eventEnd = xmlString.find("</event>");

    if (eventStart == std::string::npos || eventEnd == std::string::npos) {
        throw std::runtime_error("Invalid XML format");
    }

    std::string eventString = xmlString.substr(eventStart + 7, eventEnd - eventStart - 7);

    std::size_t headerStart = eventString.find("<headers>");
    std::size_t headerEnd = eventString.find("</headers>");

    if (headerStart == std::string::npos || headerEnd == std::string::npos) {
        throw std::runtime_error("Invalid headers format");
    }

    std::string headersString = eventString.substr(headerStart + 9, headerEnd - headerStart - 9);

    std::size_t startPos = 0;
    std::size_t endPos = headersString.find("</", startPos);

    while (endPos != std::string::npos) {
        std::string headerString = headersString.substr(startPos, endPos - startPos);

        std::size_t keyStart = headerString.find('<') + 1;
        std::size_t keyEnd = headerString.find('>', keyStart);
        std::size_t valueStart = keyEnd + 1;

        if (keyStart == std::string::npos || keyEnd == std::string::npos || valueStart == std::string::npos) {
            throw std::runtime_error("Invalid header format");
        }

        std::string key = headerString.substr(keyStart, keyEnd - keyStart);
        std::string value = headerString.substr(valueStart, endPos - valueStart);

        headers.emplace_back(Header{key, value});

        startPos = endPos + 2;
        endPos = headersString.find("</", startPos);
    }

    // Find optional Content-Length and Body
    std::size_t contentLengthStart = eventString.find("<Content-Length>");
    std::size_t contentLengthEnd = eventString.find("</Content-Length>");
    std::size_t contentLength;
    if (contentLengthStart != std::string::npos && contentLengthEnd != std::string::npos) {
        std::string contentLengthString = eventString.substr(contentLengthStart + 16, contentLengthEnd - contentLengthStart - 16);
        contentLength = std::stoi(contentLengthString);
        headers.emplace_back(Header{"Content-Length", contentLengthString});
    }

    std::size_t bodyStart = eventString.find("<body>");
    std::size_t bodyEnd = eventString.find("</body>");

    if (bodyStart != std::string::npos && bodyEnd != std::string::npos) {
        std::size_t bodyLength = bodyEnd - bodyStart - 6;
        std::string bodyString = eventString.substr(bodyStart + 6, bodyLength);
        if(bodyLength == contentLength) {
            body.append(bodyString);
        }
    }

    return headers;
}

std::string ESLevent::firstHeader() {
    currentHeader = headers.begin();
    return (currentHeader != headers.end()) ? currentHeader->key : "";
}

std::string ESLevent::nextHeader() {
    if (currentHeader != headers.end()) {
        ++currentHeader;
        return (currentHeader != headers.end()) ? currentHeader->key : "";
    }

    return "";
}

bool ESLevent::delHeader(const std::string& headerName) {
    auto it = std::remove_if(headers.begin(), headers.end(),
                             [&headerName](const Header& header) {
                                 return header.key == headerName;
                             });
    bool headerRemoved = (it != headers.end());
    headers.erase(it, headers.end());
    return headerRemoved;
}

bool ESLevent::setPriority(const std::string &priority) {
    return addHeader("priority", priority);
}