#include "computenodeblackhole.h"
#include <opencv2/opencv.hpp>


ComputeNodeBlackHole::ComputeNodeBlackHole(const QString& name)
    : GtComputeNodeBase(name)
    , min_height(name+"/min_height", 2000, 0, 3000)
{
}

bool ComputeNodeBlackHole::onInputChanged(const cv::Mat* input)
{
    output->create(input->size(), CV_8UC1);
    return true;
}

void ComputeNodeBlackHole::update(const cv::Mat* inputptr)
{
    int end = inputptr->rows * inputptr->cols;
    const cv::Mat& input = *inputptr;
    cv::Mat& output = *this->output;

    qint32 offset_col = (0.1f * input.cols);
    qint32 offset_row = (0.1f * input.rows);

    for (int r = 0; r < end; r++)
    {
        quint16 in = input.at<quint16>(r);
        quint8& out = output.at<quint8>(r);
        out = in;
        qint32 row = r / input.cols;
        qint32 col = r % input.cols;
        qint32 ic = offset_col;
        qint32 ir = offset_row;
        while(out != 0 && ic--) {
            qint32 right = col + ic;
            qint32 left = col - ic;

            if(right < input.cols && input.at<quint16>(row, right) <= min_height) {
                out = 0;
            }
            else if(left >= 0 && input.at<quint16>(row, left) <= min_height) {
                out = 0;
            }
            else{
                out = in;
            }
        }
        while(out != 0 && ir--) {
            qint32 top = row - ir;
            qint32 bottom = row + ir;
            if(top >= 0 && input.at<quint16>(top, col) <= min_height) {
                out = 0;
            }
            else if(bottom < input.rows && input.at<quint16>(bottom, col) <= min_height) {
                out = 0;
            }
            else {
                out = in;
            }
        }
        if(out != 0) {
            out = 1;
        }
    }
}
