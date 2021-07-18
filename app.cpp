#include "app.h"
#include "log.h"

App::App()
{

}

bool App::start(const std::string &ip, int port)
{
    try
    {
        if (!m_httpServer.startListing(ip, port))
            return false;
    }
    catch (const ReaderError &exc)
    {
        PRINT_ERROR("! Exception. " << exc.what());
    }
    catch (...)
    {
        PRINT_ERROR("! Unknown exception. ");
    }

    return true;
}

bool App::stop()
{
    return m_httpServer.stop();
}
