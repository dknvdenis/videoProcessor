#pragma once

#include <string>
#include <functional>

#include "streamReader.h"

enum class HttpMethod
{
    unknown,
    post
};

struct HttpRequest
{
    HttpMethod method;
    std::string path;
};

struct HttpResponse
{
    int code {500};
    std::string content;
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

    using ClientConnectedCallback = std::function<HttpResponse (IStreamReaderPtr)>;
    void setClientConnectedCallback(const ClientConnectedCallback &cb);

private:
    int m_socket {-1};
    ClientConnectedCallback m_clientConnectedCb;

private:
    void acceptConnections();
    void printPeer(int socket);
};
