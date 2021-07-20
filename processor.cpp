#include "processor.h"

#include <opencv2/opencv.hpp>
#include "log.h"
#include "copyProcess.h"
#include "drawHistogramProcess.h"
#include "brightnessGainProcess.h"

using namespace cv;

Processor::Processor()
{
    m_thread = std::thread([this] {
        run();
    });
}

Processor::~Processor()
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_exitFlag = true;
    }

    m_signal.notify_one();
    m_thread.join();
}

std::future<bool> Processor::addTask(const std::string &filename, int gain)
{
    Task task;

    task.filename = filename;
    task.gain = gain;
    task.promise = std::make_shared<std::promise<bool>>();

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(task);
    }

    m_signal.notify_one();

    return task.promise->get_future();
}

void Processor::run()
{
    std::queue<Task> tmpQueue;

    while (!m_exitFlag)
    {
        {
            std::unique_lock<std::mutex> lock(m_mutex);

            while (m_queue.empty())
            {
                m_signal.wait(lock);

                if (m_exitFlag)
                    return;
            }

            tmpQueue.swap(m_queue);
        }

        while (!tmpQueue.empty())
        {
            processTask(tmpQueue.front());
            tmpQueue.pop();
        }
    }
}

void Processor::processTask(const Task &task)
{
    bool futureNotSet = true;
    auto setFutureValue = [&] (bool value) {
        if (futureNotSet)
        {
            task.promise->set_value(value);
            futureNotSet = false;
        }
    };

    std::vector<IProcessPtr> processes;

    processes.push_back(std::make_shared<CopyProcess>());
    processes.push_back(std::make_shared<DrawHistogramProcess>());
    processes.push_back(std::make_shared<BrightnessGainProcess>(task.gain));

    VideoCapture capture(task.filename);

    if (!capture.isOpened())
    {
        setFutureValue(false);
        return;
    }

    Mat srcFrame;
    Mat dstFrame;
    namedWindow("w", 1);

    while (true)
    {
        capture >> srcFrame;

        if (srcFrame.empty())
        {
            setFutureValue(false);
            return;
        }

        if (dstFrame.empty())
        {
            dstFrame = Mat(srcFrame.rows, srcFrame.cols * processes.size(),
                           srcFrame.type());
            setFutureValue(true);
        }

        for (int i = 0; i < processes.size(); i++)
        {
            auto process = processes[i];

            Rect dstRect(srcFrame.cols * i, 0, srcFrame.cols, srcFrame.rows);
            Mat dst = dstFrame(dstRect);

            if (!process->process(srcFrame, dst))
                PRINT_ERROR(process->name() << ": Failed to process image");
        }

        imshow("w", dstFrame);
        waitKey(20);
    }

    waitKey(0);
    setFutureValue(false);
}
