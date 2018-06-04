#ifndef INPUTFRAMESTREAM_H
#define INPUTFRAMESTREAM_H

#include "SharedModule/decl.h"

class QFile;

namespace cv {
    class Mat;
}

class InputFrameStream
{
public:
    InputFrameStream(quint32 outputsCount);
    ~InputFrameStream();

    void MoveToThread(QThread* thread);
    void SetFileName(const QString& name);
    void SetPause(bool flag) { _paused = flag; }
    bool IsPaused() const { return _paused; }
    bool ReadFrame();
    void Repeat();
    bool IsFinished() const;
    bool IsValid() const;

    const cv::Mat& GetOutput() const;

    static QStringList GetAvailableInputs();

private:
    ScopedPointer<cv::Mat> _output;
    ScopedPointer<QFile> _inputFile;
    ScopedPointer<QDataStream> _inputStream;

    quint32 _outputsCount;
    quint32 _readingCounter;
    bool _paused;
};

#endif // INPUTFRAMESTREAM_H
