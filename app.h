#pragma once

#include "httpServer.h"

class App
{
public:
    explicit App();

public:
    bool start(const std::string &ip, int port);
    bool stop();

private:
    HttpServer m_httpServer;
};
