#include "brightnessGainProcess.h"
#include <chrono>
#include "log.h"

using namespace cv;

//#define BENCH

BrightnessGainProcess::BrightnessGainProcess(int gain)
    : m_gain(gain)
{

}

std::string BrightnessGainProcess::name() const
{
    return "BrightnessGainProcess";
}

bool BrightnessGainProcess::process(cv::Mat &src, cv::Mat &dst)
{
#ifdef BENCH
    using namespace std::chrono;
    auto start = high_resolution_clock::now();
#endif

    #pragma omp parallel for
    for (int r = 0; r < src.rows; r++)
    {
        for (int c = 0; c < src.cols; c++)
        {
            auto value = src.at<Vec3b>(r, c) * m_gain;

            dst.at<Vec3b>(r, c) = value;
        }
    }

#ifdef BENCH
    auto now = high_resolution_clock::now();
    PRINT_LOG(duration_cast<milliseconds>(now - start).count());
#endif

    return true;
}
