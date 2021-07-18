#pragma once

#include <string>
#include <functional>
#include <chrono>
#include <memory>
#include <stdexcept>
#include "charIter.h"

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

class IStreamReader
{
public:
    virtual ~IStreamReader() = default;

public:
    virtual CharRange read() = 0;
};

class ReaderError : public std::logic_error
{
public:
    explicit ReaderError(int code = 0)
        : logic_error("ReaderError"),
          m_errorCode(code)
    {}

    explicit ReaderError(const std::string &msg, int code = 0)
        : logic_error(msg),
          m_errorCode(code)
    {}

public:
    int getErrorCode() const { return m_errorCode; }

private:
    int m_errorCode;
};

class ReaderTimeout : public ReaderError
{
public:
    explicit ReaderTimeout()
        : ReaderError("ReaderTimeout")
    {}
};

class ReaderBufferOverflow : public ReaderError
{
public:
    explicit ReaderBufferOverflow()
        : ReaderError("ReaderBufferOverflow")
    {}
};

class StreamReader : public IStreamReader
{
public:
    explicit StreamReader(int socket,
                          std::chrono::milliseconds timeout = std::chrono::seconds(5),
                          std::size_t bufferSize = 4096);

public:
    CharRange read() override;

private:
    int m_socket;
    std::chrono::milliseconds m_timeLeft;
    std::size_t m_bufSize;
    std::unique_ptr<char[]> m_buffer;
    std::size_t m_bufPos {0};
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
