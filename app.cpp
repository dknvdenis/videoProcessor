#include "app.h"

#include "log.h"
#include "vpException.h"

App::App()
{

}

bool App::start(const std::string &ip, int port)
{
//    try
//    {
//        m_httpServer.setRequestCallback(std::bind(&App::newRequest, this,
//                                                  std::placeholders::_1));

//        if (!m_httpServer.startListing(ip, port))
//            return false;
//    }
//    catch (const ReaderError &exc)
//    {
//        PRINT_ERROR("! Exception. " << exc.what());
//    }
//    catch (...)
//    {
//        PRINT_ERROR("! Unknown exception. ");
//    }

    try
    {
//        newRequest("/home/denis/test.mp4", 2);
        newRequest("/home/denis/no-god-please.mp4", 3);
//        newRequest("/home/denis/hist_test.jpg", 2);
    }
    catch (const std::exception &exc)
    {
        PRINT_ERROR("!! Exception. " << exc.what());
    }

    return true;
}

bool App::stop()
{
//    return m_httpServer.stop();
    return true;
}

bool App::newRequest(const std::string &filename, int gain)
{
    auto resultFuture = m_processor.addTask(filename, gain);

    if (resultFuture.wait_for(std::chrono::seconds(5)) == std::future_status::timeout)
        return false;

    return resultFuture.get();
}
