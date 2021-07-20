#pragma once

#include <functional>
#include "tcpServer.h"
#include "parserUtils.h"

struct ClientContext
{
    std::string method;
    std::string path;

    bool contentLengthFound {false};
    bool contentTypeFound {false};

    std::size_t contentLength {0};
    bool isContentTypeUrlencoded {false};

    bool filenameFound {false};
    bool gainFound {false};

    std::string filename;
    int gain {0};
};

class HttpServer
{
public:
    explicit HttpServer();
    ~HttpServer();

public:
    bool startListing(const std::string &ip, int port);
    bool stop();

    using RequestCallback = std::function<bool (const std::string&, int gain)>;
    void setRequestCallback(const RequestCallback &cb);

private:
    TcpServer m_tcpServer;
    RequestCallback m_requestCb;

private:
    std::string clientConnected(IStreamReaderPtr reader);
    std::string makeHttpResponse(int code, const std::string &content = std::string());
    std::string httpCodeToStatusText(int code);

    bool parseFirstLine(ParserUtils &pu, ClientContext &ctx);
    bool parseHeaders(ParserUtils &pu, ClientContext &ctx);
    bool parseParams(ParserUtils &pu, ClientContext &ctx);
};
