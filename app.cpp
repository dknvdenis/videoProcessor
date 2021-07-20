#include "app.h"

#include <opencv2/opencv.hpp>
#include "log.h"
#include "vpException.h"
#include "copyProcess.h"
#include "drawHistogramProcess.h"

using namespace cv;

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
        newRequest("/home/denis/no-god-please.mp4", 2);
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
    std::vector<IProcessPtr> processes;

    processes.push_back(std::make_shared<CopyProcess>());
    processes.push_back(std::make_shared<DrawHistogramProcess>());

    VideoCapture capture(filename);

    Mat frame;

    if (!capture.isOpened())
        return false;

    Mat outFrame;

    namedWindow("w", 1);
    while (true)
    {
        capture >> frame;

        if(frame.empty())
            break;

        if (outFrame.empty())
            outFrame = Mat(frame.rows, frame.cols * processes.size(), frame.type());

        for (int i = 0; i < processes.size(); i++)
        {
            auto process = processes[i];

            Rect dstRect(frame.cols * i, 0, frame.cols, frame.rows);
            Mat dst = outFrame(dstRect);

            if (!process->process(frame, dst))
                PRINT_ERROR(process->name() << ": Failed to process image");
        }

        imshow("w", outFrame);
        waitKey(20);
    }

    waitKey(0);

    return true;
}
