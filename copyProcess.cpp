#include "copyProcess.h"

std::string CopyProcess::name() const
{
    return "CopyProcess";
}

bool CopyProcess::process(cv::Mat &src, cv::Mat &dst)
{
    src.copyTo(dst);
    return true;
}
