#pragma once

#include <string>
#include <functional>

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

private:
    RequestCallback m_postCb;
    int m_socket {-1};
};
