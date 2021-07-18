#include "httpServer.h"

#include <cassert>
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
    StreamReader reader(peer/*, std::chrono::seconds(5), 10*/);

    auto range = reader.read();

    HttpLexer lexer;

    CharIter lexerIter;
    std::vector<Token> tokens;

    std::tie(lexerIter, tokens) = lexer.getTokens(range.begin, range.end);

    PRINT_LOG("tokens size: " << tokens.size());
    for (const Token &token : tokens)
    {
        PRINT_LOG_SIMPLE("token type: \"" << TokenTypeToString(token.type) << "\"\t");

        if (token.type == TokenType::string)
        {
            PRINT_LOG_SIMPLE("token value: \"" << token.value.toString()
                             << "\"");
        }

        PRINT_LOG_SIMPLE(std::endl);
    }
}

StreamReader::StreamReader(int socket, std::chrono::milliseconds timeout,
                           std::size_t bufferSize)
    : m_socket(socket),
      m_timeLeft(timeout),
      m_bufSize(bufferSize),
      m_buffer(new char[bufferSize])
{
    assert(socket > -1);
    assert(bufferSize > 0);
}

CharRange StreamReader::read()
{
    if (m_timeLeft.count() == 0)
        throw ReaderTimeout();

    if (m_bufPos == m_bufSize)
        throw ReaderBufferOverflow();

    pollfd pollInfo = {};
    pollInfo.fd = m_socket;
    pollInfo.events = POLLIN;

    int pollStatus = poll(&pollInfo, 1, m_timeLeft.count());

    if (pollStatus == 0)
        throw ReaderTimeout();
    else if (pollStatus == -1)
        throw ReaderError(errno);

    int count = ::read(m_socket, m_buffer.get() + m_bufPos, m_bufSize - m_bufPos);

    if (count == 0)
        throw ReaderError("EOF");
    else if (count == -1)
        throw ReaderError("Read error", errno);

    CharRange result(m_buffer.get() + m_bufPos, m_buffer.get() + m_bufPos + count);

    m_bufPos += count;

    return result;
}
