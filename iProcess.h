#pragma once

#include <memory>
#include <opencv2/opencv.hpp>

class IProcess
{
public:
    virtual ~IProcess() = default;

public:
    virtual std::string name() const = 0;
    virtual bool process(cv::Mat &src, cv::Mat &dst) = 0;
};

using IProcessPtr = std::shared_ptr<IProcess>;
