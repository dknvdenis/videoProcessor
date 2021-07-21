#pragma once

#include "httpServer.h"
#include "processor.h"

class App
{
public:
    explicit App();

public:
    bool start(const std::string &ip, int port);
    bool stop();

private:
    HttpServer m_httpServer;
    Processor m_processor;

private:
    bool newRequest(const std::string &filename, unsigned long gain);
};
