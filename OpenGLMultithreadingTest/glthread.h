#ifndef GLTHREAD_H
#define GLTHREAD_H

#include <QThread>
#include <QVector>
#include <QMutex>
#include <functional>

class QOpenGLWidget;
class GLRender;

class GLThread : public QThread
{
    typedef QThread Super;
public:
    GLThread(GLRender* render, QOpenGLWidget* parent=nullptr);

    void Resize(qint32 w, qint32 h);
    void Asynch(const std::function<void ()>& event);

    void Start();
    void Stop();

    // QThread interface
protected:
    void run() Q_DECL_OVERRIDE;

private:
    QOpenGLWidget* getGL() const;

private:
    GLRender* _render;
    std::atomic_bool _stoped;

    QMutex _eventsLocker;
    QVector<std::function<void ()>> _events;
};


#endif // GLTHREAD_H
