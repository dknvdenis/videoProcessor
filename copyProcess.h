#pragma once

#include "iProcess.h"

class CopyProcess : public IProcess
{
public:
    std::string name() const override;
    bool process(cv::Mat &src, cv::Mat &dst) override;
};
