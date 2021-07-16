#include "httpServer.h"

#include <iostream>
//#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "log.h"

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
