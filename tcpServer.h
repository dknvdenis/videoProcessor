#pragma once

#include <string>
#include <functional>

#include "streamReader.h"

enum class HttpMethod
{
    unknown,
    post
};

class TcpServer
{
public:
    explicit TcpServer();
    ~TcpServer();

public:
    bool startListing(const std::string &ip, int port);
    bool stop();
    bool isWork() const;

    using ClientConnectedCallback = std::function<std::string (IStreamReaderPtr)>;
    void setClientConnectedCallback(const ClientConnectedCallback &cb);

private:
    int m_socket {-1};
    ClientConnectedCallback m_clientConnectedCb;

private:
    void acceptConnections();
    void printPeer(int socket);
};
