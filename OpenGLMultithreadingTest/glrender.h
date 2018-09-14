#ifndef GLRENDER_H
#define GLRENDER_H

#include <QOpenGLFunctions_2_0>
#include <SharedModule/internal.hpp>

class GtCamera;

class GLRender : public QObject, protected QOpenGLFunctions_2_0
{
    Q_OBJECT
public:
    GLRender(QObject* parent = nullptr);

    void SetCamera(GtCamera* camera);

    void Resize(qint32 w, qint32 h);
    void Initialize();
    void Draw();

Q_SIGNALS:
    void imageUpdated();

private:
    GtCamera* _camera;
    qint32 _w;
    qint32 _h;
};

#endif // GLRENDER_H
