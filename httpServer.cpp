#include "httpServer.h"

#include <stdexcept>
#include "log.h"
#include "httpLexer.h"
#include "vpException.h"

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

void HttpServer::setRequestCallback(const HttpServer::RequestCallback &cb)
{
    m_requestCb = cb;
}

std::string HttpServer::clientConnected(IStreamReaderPtr reader)
{
    try
    {
        auto lexer = std::make_shared<HttpLexer>(reader);
        ParserUtils pu(lexer);

        ClientContext ctx;

        pu.setMergeStringSequence(true);
        pu.setStopSequence({TokenType::cr, TokenType::lf,
                            TokenType::cr, TokenType::lf});

        bool parseOk = parseFirstLine(pu, ctx);
        PRINT_LOG("Parse first line: " << parseOk);

        if (!parseOk)
            return makeHttpResponse(400, "Failed to parse http header");

        PRINT_LOG("Method: \"" << ctx.method << "\"\tpath: \"" << ctx.path << "\"");

        parseOk = parseHeaders(pu, ctx);
        PRINT_LOG("Parse headers: " << parseOk);

        if (!parseOk)
            return makeHttpResponse(400, "Failed to parse http header");

        PRINT_LOG("ContentLength: \"" << ctx. contentLength
                  << "\"\tisContentTypeUrlencoded: \""
                  << ctx.isContentTypeUrlencoded << "\"");

        if (ctx.contentLength == 0)
            return makeHttpResponse(400, "Content not found");

        while (pu.next())
        {
            ; // move to \r\n\r\n
        }

        pu.setStopSequence({});
        pu.setMaxLength(ctx.contentLength);

        parseOk = parseParams(pu, ctx);
        PRINT_LOG("Parse params: " << parseOk);

        if (!parseOk)
            return makeHttpResponse(400, "Failed to parse params");

        PRINT_LOG("Filename: \"" << ctx.filename
                  << "\"\tgain: \"" << ctx.gain << "\"");

        if (m_requestCb)
        {
            if (m_requestCb(ctx))
                return makeHttpResponse(200);
            else
                return makeHttpResponse(404, "File not found");
        }
    }
    catch (const ReaderTimeout &exc)
    {
        PRINT_ERROR("!Exception: reader timeout. " << exc.what());
        return makeHttpResponse(408, "timeout");
    }
    catch (const ReaderBufferOverflow &exc)
    {
        PRINT_ERROR("!Exception: reader buffer overflow. " << exc.what());
        return makeHttpResponse(400, "Content too large");
    }
    catch (...)
    {
        PRINT_ERROR("!Exception: unknown");
    }

    return makeHttpResponse(500);
}

std::string HttpServer::makeHttpResponse(int code, const std::string &content)
{
    std::string response("HTTP/1.1 ");
    response.append(httpCodeToStatusText(code));
    response.append("\r\n"
                    "Content-Type: text/html\r\n"
                    "Connection: close\r\n"
                    "Content-Length: ");
    response.append(std::to_string(content.size()));
    response.append("\r\n\r\n");
    response.append(content);

    return response;
}

std::string HttpServer::httpCodeToStatusText(int code)
{
    switch (code)
    {
    case 200:
        return "200 OK";
    case 400:
        return "400 Bad Request";
    case 404:
        return "404 Not Found";
    case 408:
        return "408 Request Timeout";
    case 500:
        return "500 Internal Server Error";
    default:
        return std::to_string(code) + " code";
    }
}

bool HttpServer::parseFirstLine(ParserUtils &pu, ClientContext &ctx)
{
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

bool HttpServer::parseHeaders(ParserUtils &pu, ClientContext &ctx)
{
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

bool HttpServer::parseParams(ParserUtils &pu, ClientContext &ctx)
{
    while (!pu.isMaxLengthReached())
    {
        if (ctx.filenameFound && ctx.gainFound)
            break;

        if (!pu.expected(TokenType::string))
            return false;

        if (pu.token().compare("filename"))
        {
            if (!pu.expected(TokenType::equalSign))
                return false;

            if (!pu.next())
                return false;

            if (pu.token().type == TokenType::doubleQuotes)
            {
                while (pu.next())
                {
                    if (pu.token().type == TokenType::doubleQuotes)
                        ctx.filenameFound = true;
                    else
                        ctx.filename += pu.token().toString();
                }

                if (!ctx.filenameFound)
                    return false;
            }
            else if (pu.token().type == TokenType::string)
            {
                ctx.filename = pu.token().toString();
                ctx.filenameFound = true;
            }
        }
        else if (pu.token().compare("gain"))
        {
            if (!pu.expected(TokenType::equalSign))
                return false;

            if (!pu.expected(TokenType::string))
                return false;

            try
            {
                ctx.gain = std::stoi(pu.token().toString()); // throw exc
                ctx.gainFound = true;
            }
            catch (const std::exception &exc)
            {
                PRINT_ERROR("Failed to parse gain param. " << exc.what());
                return false;
            }
        }

        pu.skipAndStopAfterSequence({TokenType::ampersand});
    }

    return ctx.filenameFound && ctx.gainFound;
}
