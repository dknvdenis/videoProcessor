#include "httpServer.h"

#include <stdexcept>
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

    if (!parseOk)
        return HttpResponse();

    PRINT_LOG("Method: \"" << ctx.method << "\"\tpath: \"" << ctx.path << "\"");

    parseOk = parseHeaders(ctx);
    PRINT_LOG("Parse headers: " << parseOk);

    if (!parseOk)
        return HttpResponse();

    PRINT_LOG("ContentLength: \"" << ctx. contentLength
              << "\"\tisContentTypeUrlencoded: \""
              << ctx.isContentTypeUrlencoded << "\"");

    if (ctx.contentLength == 0)
        return HttpResponse();

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

bool HttpServer::parseHeaders(ClientContext &ctx)
{
    auto &pu = ctx.parser;

    while (!pu.isStopSequenceReached())
    {
        if (ctx.contentLengthFound && ctx.contentTypeFound)
            break;

        if (!pu.expected(TokenType::string))
            return false;

        if (pu.token().compare("Content-Length:"))
        {
            if (!pu.expected(TokenType::space))
                return false;

            if (!pu.expected(TokenType::string))
                return false;

            try
            {
                ctx.contentLength = std::stoi(pu.token().toString()); // throw exc
                ctx.contentLengthFound = true;
            }
            catch (const std::exception &exc)
            {
                PRINT_ERROR("Failed to parse contentLength header. " << exc.what());
                return false;
            }

            if (!pu.expected({TokenType::cr, TokenType::lf}))
                return false;
        }
        else if (pu.token().compare("Content-Type:"))
        {
            if (!pu.expected(TokenType::space))
                return false;

            if (!pu.expected(TokenType::string))
                return false;

            ctx.isContentTypeUrlencoded = pu.token()
                    .compare("application/x-www-form-urlencoded");
            ctx.contentTypeFound = true;

            if (!pu.expected({TokenType::cr, TokenType::lf}))
                return false;
        }
        else
            pu.skipAndStopAfterSequence({TokenType::cr, TokenType::lf});
    }

    return ctx.contentLengthFound && ctx.contentTypeFound;
}
