#include "drawHistogramProcess.h"

using namespace cv;

namespace
{
    const int HistSize = 256;
}

std::string DrawHistogramProcess::name() const
{
    return "MakeHistogramProcess";
}

bool DrawHistogramProcess::process(Mat &src, cv::Mat &dst)
{
    dst.setTo(0);

    Mat histB = getHist(src, 0);
    Mat histG = getHist(src, 1);
    Mat histR = getHist(src, 2);

    float scale = float(dst.cols) / HistSize;
    for (int i = 1; i < HistSize; i++)
    {
        drawHistElement(i, histB, dst, scale, Scalar(255, 0, 0));
        drawHistElement(i, histG, dst, scale, Scalar(0, 255, 0));
        drawHistElement(i, histR, dst, scale, Scalar(0, 0, 255));
    }

    return true;
}

Mat DrawHistogramProcess::getHist(Mat src, int channel)
{
    int histSize[] = {HistSize};

    float hranges[] = {0, HistSize};
    float sranges[] = {0, HistSize};

    const float* ranges[] = {hranges, sranges};
    int channels[] = {channel};

    Mat hist;

    calcHist(&src, 1, channels, Mat(),
             hist, 1, histSize, ranges,
             true, false);

    normalize(hist, hist, 0, hist.rows, NORM_MINMAX, -1, Mat());

    return hist;
}

void DrawHistogramProcess::drawHistElement(int index, Mat hist, Mat dst,
                                           float scale, Scalar color)
{
    if (index < 1 || index >= hist.rows)
        return;

    Point p1((index - 1) * scale, dst.rows - hist.at<float>(index - 1));
    Point p2(index * scale, dst.rows - hist.at<float>(index));

    line(dst, p1, p2, color, 2);
}
