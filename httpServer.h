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
    int code;
    std::string content;
};

class HttpServer
{
public:
    explicit HttpServer();
    ~HttpServer();

public:
    bool startListing(const std::string &ip, int port);
    bool stop();
    bool isWork() const;

    using RequestCallback = std::function<HttpResponse (const HttpRequest&)>;
    void setRequestCallback(const RequestCallback &cb);

    using ClientConnectedCallback = std::function<HttpResponse (IStreamReader*)>;
    void setClientConnectedCallback(const ClientConnectedCallback &cb);

private:
    RequestCallback m_postCb;
    int m_socket {-1};
    ClientConnectedCallback m_clientConnectedCb;

private:
    void acceptConnections();
    void printPeer(int socket);
    void readHttpRequest(int peer);
};
