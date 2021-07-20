#pragma once

#include "iProcess.h"

class DrawHistogramProcess : public IProcess
{
public:
    std::string name() const override;
    bool process(cv::Mat &src, cv::Mat &dst) override;

private:
    cv::Mat getHist(cv::Mat src, int channel);
    void drawHistElement(int index, cv::Mat hist, cv::Mat dst, float scale, cv::Scalar color);
};
