#include "httpServer.h"

#include <iostream>
//#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
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
    const int bufSize = 1024;
    char buf[bufSize + 1] = {};

    pollfd pollInfo = {};
    pollInfo.fd = peer;
    pollInfo.events = POLLIN;

    poll(&pollInfo, 1, 1000);

    int count = read(peer, buf, bufSize);
    buf[count] = '\0';

    PRINT_LOG("Receive msg (" << count << " bytes):\n" << buf << "\n---End msg---");

    HttpLexer lexer;

    CharIter lexerIter;
    std::vector<Token> tokens;

    std::tie(lexerIter, tokens) = lexer.getTokens(buf, buf + count);

    PRINT_LOG("tokens size: " << tokens.size());
    for (const Token &token : tokens)
    {
        PRINT_LOG_SIMPLE("token type: \"" << TokenTypeToString(token.type) << "\"\t");

        if (token.type == TokenType::string)
        {
            PRINT_LOG_SIMPLE("token value: \"" << std::string(token.begin, token.end)
                             << "\"");
        }

        PRINT_LOG_SIMPLE(std::endl);
    }
}
