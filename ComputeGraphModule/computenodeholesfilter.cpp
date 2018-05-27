#include "computenodeholesfilter.h"
#include <opencv2/opencv.hpp>

ComputeNodeHolesFilter::ComputeNodeHolesFilter(const QString& name)
    : GtComputeNodeBase(name)
{

}

void ComputeNodeHolesFilter::update(const cv::Mat* input)
{
    horizontalInterpolation(*input, *_output);
}

bool ComputeNodeHolesFilter::onInputChanged(const cv::Mat* input)
{
    *_output = input->clone();
    return true;
}

void ComputeNodeHolesFilter::horizontalInterpolation(const cv::Mat &input, cv::Mat &out)
{
    // Sew the holes
    cv::Size s = input.size();
    quint16 lborder, rborder;
    qint32 i,j,k;
    qint32 from, to;

    for (j = 0; j < s.height; j++)
    {
        lborder = rborder = 0.f;
        for (i = 0; i < s.width; i++)
        {
            quint16 in = input.at<quint16>(j,i);
            if (in == 0)
            {
                from = i++;
                qint32 widthMinusOne = s.width - 1;

                while (i < widthMinusOne && input.at<quint16>(j,i) == 0) i++;

                to = std::min(widthMinusOne, i);
                rborder = input.at<quint16>(j,to);

                quint16 value = (rborder == 0) ? lborder : rborder;

                for (k = from; k < to; k++)
                {
                    quint16& pixel = out.at<quint16>(j, k);
                    pixel = value;
                }
            }
            else
            {
                from = i;
                out.at<quint16>(j, i) = in;
            }

            lborder = input.at<quint16>(j,from);
        }
    }
}

