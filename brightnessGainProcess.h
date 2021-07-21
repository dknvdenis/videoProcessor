#pragma once

#include "iProcess.h"

class BrightnessGainProcess : public IProcess
{
public:
    explicit BrightnessGainProcess(unsigned long gain);

public:
    std::string name() const override;
    bool process(cv::Mat &src, cv::Mat &dst) override;

private:
    unsigned long m_gain;
};
