#pragma once

#include "iProcess.h"

class BrightnessGainProcess : public IProcess
{
public:
    explicit BrightnessGainProcess(float gain);

public:
    std::string name() const override;
    bool process(cv::Mat &src, cv::Mat &dst) override;

private:
    float m_gain;
};
