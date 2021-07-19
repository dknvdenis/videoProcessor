#include "httpServer.h"

#include "log.h"
#include "httpLexer.h"

HttpServer::HttpServer()
{

}

HttpServer::~HttpServer()
{
    stop();
}

bool HttpServer::startListing(const std::string &ip, int port)
{
    m_tcpServer.setClientConnectedCallback(std::bind(&HttpServer::clientConnected,
                                                     this, std::placeholders::_1));

    return m_tcpServer.startListing(ip, port);
}

bool HttpServer::stop()
{
    return m_tcpServer.stop();
}

HttpResponse HttpServer::clientConnected(IStreamReaderPtr reader)
{
    auto lexer = std::make_shared<HttpLexer>(reader);
    ParserUtils pu(lexer);

    ClientContext ctx(pu);

    pu.setMergeStringSequence(true);
    pu.setStopSequence({TokenType::cr, TokenType::lf,
                        TokenType::cr, TokenType::lf});

    bool parseOk = parseFirstLine(ctx);

    PRINT_LOG("Parse first line: " << parseOk);
    if (parseOk)
        PRINT_LOG("Method: \"" << ctx.method << "\"\tpath: \"" << ctx.path << "\"");

    return HttpResponse();
}

bool HttpServer::parseFirstLine(ClientContext &ctx)
{
    auto &pu = ctx.parser;

    // example: POST /process HTTP/1.1
    if (!pu.expected(TokenType::string))
        return false;

    ctx.method = pu.token().toString();

    // -----------------

    if (!pu.expected(TokenType::space))
        return false;

    // -----------------

    if (!pu.expected(TokenType::string))
        return false;

    ctx.path = pu.token().toString();

    // -----------------

    if (!pu.expected(TokenType::space))
        return false;

    // -----------------

    if (!pu.expected(TokenType::string))
        return false;

    if (!pu.token().compare("HTTP/1.1"))
        return false;

    if (!pu.expected({TokenType::cr, TokenType::lf}))
        return false;

    return true;
}
