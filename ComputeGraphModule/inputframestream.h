#ifndef INPUTFRAMESTREAM_H
#define INPUTFRAMESTREAM_H

#include "Shared/shared_decl.h"

class QFile;

namespace cv {
    class Mat;
}

class InputFrameStream
{
public:
    InputFrameStream(quint32 outputs_count);
    ~InputFrameStream();

    void moveToThread(QThread* thread);
    void setFileName(const QString& name);
    void setPause(bool flag) { paused = flag; }
    bool isPaused() const { return paused; }
    bool readFrame();
    void repeat();
    bool isFinished() const;
    bool isValid() const;

    const cv::Mat& getOutput() const;

    static QStringList getAvailableInputs();

private:
    ScopedPointer<cv::Mat> output;
    ScopedPointer<QFile> input_file;
    ScopedPointer<QDataStream> input_stream;

    quint32 outputs_count;
    quint32 reading_counter;
    bool paused;
};

#endif // INPUTFRAMESTREAM_H
