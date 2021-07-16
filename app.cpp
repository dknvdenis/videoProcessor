#include "app.h"

App::App()
{

}

bool App::start(const std::string &ip, int port)
{
    if (!m_httpServer.startListing(ip, port))
        return false;

    return true;
}

bool App::stop()
{
    return m_httpServer.stop();
}
