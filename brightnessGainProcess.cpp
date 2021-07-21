#include "brightnessGainProcess.h"
#include <chrono>
#include "log.h"

using namespace cv;

//#define BENCH

BrightnessGainProcess::BrightnessGainProcess(float gain)
    : m_gain(std::abs(gain))
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
            auto srcValue = src.at<Vec3b>(r, c);

            dst.at<Vec3b>(r, c)[0] = std::min(std::roundf(srcValue[0] * m_gain), 255.f);
            dst.at<Vec3b>(r, c)[1] = std::min(std::roundf(srcValue[1] * m_gain), 255.f);
            dst.at<Vec3b>(r, c)[2] = std::min(std::roundf(srcValue[2] * m_gain), 255.f);
        }
    }

#ifdef BENCH
    auto now = high_resolution_clock::now();
    PRINT_LOG(duration_cast<milliseconds>(now - start).count());
#endif

    return true;
}
