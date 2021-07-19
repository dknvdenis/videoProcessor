#pragma once

#include "tcpServer.h"
#include "parserUtils.h"

struct ClientContext
{
    explicit ClientContext(ParserUtils &parser_)
        : parser(parser_)
    {}

    ParserUtils &parser;

    std::string method;
    std::string path;

    bool contentLengthFound {false};
    bool contentTypeFound {false};

    std::size_t contentLength {0};
    bool isContentTypeUrlencoded {false};
};

class HttpServer
{
public:
    explicit HttpServer();
    ~HttpServer();

public:
    bool startListing(const std::string &ip, int port);
    bool stop();

private:
    TcpServer m_tcpServer;

private:
    HttpResponse clientConnected(IStreamReaderPtr reader);
    bool parseFirstLine(ClientContext &ctx);
    bool parseHeaders(ClientContext &ctx);
};
