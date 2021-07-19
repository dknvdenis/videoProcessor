#include "httpServer.h"

#include <cassert>
#include <queue>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "log.h"
#include "httpLexer.h"
#include "parserUtils.h"

HttpServer::HttpServer()
{

}

HttpServer::~HttpServer()
{
    stop();
}

bool HttpServer::startListing(const std::string &ip, int port)
{
    if (isWork())
    {
        PRINT_ERROR("Server is already listening");
        return false;
    }

    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket < 0)
    {
        PRINT_ERROR("Failed to create socket " << errno);
        return false;
    }

    int optValue = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &optValue, sizeof(optValue)) < 0)
    {
        PRINT_ERROR("Failed to set sockopt " << errno);
        stop();
        return false;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);

    if (bind(m_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        PRINT_ERROR("Failed to bind socket " << errno);
        stop();
        return false;
    }

    if (listen(m_socket, SOMAXCONN) < 0)
    {
        PRINT_ERROR("Failed to listen socket " << errno);
        stop();
        return false;
    }

    PRINT_LOG("Listening on " << ip << ":" << port);
    acceptConnections();

    return true;
}

bool HttpServer::stop()
{
    if (!isWork())
        return true;

    if (shutdown(m_socket, SHUT_RD) < 0)
    {
        PRINT_ERROR("Failed to shutdown socket " << errno);
        return false;
    }

    if (close(m_socket) < 0)
    {
        PRINT_ERROR("Failed to close socket " << errno);
        return false;
    }

    m_socket = -1;
    return true;
}

bool HttpServer::isWork() const
{
    return m_socket > -1;
}

void HttpServer::setRequestCallback(const HttpServer::RequestCallback &cb)
{
    m_postCb = cb;
}

void HttpServer::setClientConnectedCallback(const ClientConnectedCallback &cb)
{
    m_clientConnectedCb = cb;
}

void HttpServer::acceptConnections()
{
    while (isWork())
    {
        int peer = accept(m_socket, nullptr, nullptr);
        if (peer < 0)
        {
            if (errno == EINVAL)
                return;

            PRINT_ERROR("Failed to accept connection " << errno);
            continue;
        }

        printPeer(peer);
        readHttpRequest(peer);

        shutdown(peer, SHUT_RDWR);
        close(peer);
    }
}

void HttpServer::printPeer(int socket)
{
    sockaddr_storage addr;
    socklen_t len = sizeof(addr);

    getpeername(socket, reinterpret_cast<sockaddr*>(&addr), &len);

    if (addr.ss_family == AF_INET)
    {
        sockaddr_in *s = reinterpret_cast<sockaddr_in*>(&addr);
        char ipstr[INET_ADDRSTRLEN];
        int port = ntohs(s->sin_port);

        if (inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof(ipstr)) != nullptr)
        {
            PRINT_LOG("New peer: " << ipstr << ":" << port);
            return;
        }
    }

    PRINT_ERROR("Unknown peer");
}

void HttpServer::readHttpRequest(int peer)
{
    auto reader = std::make_shared<StreamReader>(peer);
    auto lexer = std::make_shared<HttpLexer>(reader);

    ParserUtils pu(lexer);

    pu.setMergeStringSequence(true);
    pu.setStopSequence({TokenType::cr, TokenType::lf,
                        TokenType::cr, TokenType::lf});

    std::string httpMethod;
    std::string path;

    // example: POST /process HTTP/1.1
    auto parseFirstLine = [&] {
        if (!pu.expected(TokenType::string))
            return false;

        httpMethod = pu.token().toString();

        // -----------------

        if (!pu.expected(TokenType::space))
            return false;

        // -----------------

        if (!pu.expected(TokenType::string))
            return false;

        path = pu.token().toString();

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
    };

    bool parseOk = parseFirstLine();

    PRINT_LOG("Parse first line: " << parseOk);
    if (parseOk)
        PRINT_LOG("Method: \"" << httpMethod << "\"\tpath: \"" << path << "\"");
}
