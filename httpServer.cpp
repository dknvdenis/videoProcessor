#include "httpServer.h"

#include <stdexcept>
#include <algorithm>
#include <sstream>

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

        if (ctx.method != "POST")
        {
            PRINT_ERROR("Supported only POST method");
            return makeHttpResponse(400, "Supported only POST method");
        }

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
            if (m_requestCb(ctx.filename, ctx.gain))
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
    catch (const ReaderError &exc)
    {
        PRINT_ERROR("!Exception: Read error. " << exc.what());
        return makeHttpResponse(400, "Read error");
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

            if (!pu.expected(TokenType::string))
                return false;

            ctx.filename = pu.token().toString();
            std::replace(ctx.filename.begin(), ctx.filename.end(), '+', ' ');
            ctx.filenameFound = true;
        }
        else if (pu.token().compare("gain"))
        {
            if (!pu.expected(TokenType::equalSign))
                return false;

            if (!pu.expected(TokenType::string))
                return false;

            std::stringstream ss;
            ss.imbue(std::locale("C"));

            std::string value = pu.token().toString();
            std::replace(value.begin(), value.end(), ',', '.');

            ss.write(value.c_str(), value.size());
            if (ss >> ctx.gain)
                ctx.gainFound = true;
        }

        pu.skipAndStopAfterSequence({TokenType::ampersand});
    }

    return ctx.filenameFound && ctx.gainFound;
}
