#ifndef QTQSSREADER_H
#define QTQSSREADER_H

#ifdef QT_GUI_LIB

#include <QString>
#include "SharedModule/internal.hpp"

class _Export QtQSSReader
{
    QString _fileName;
    ScopedPointer<class QtObserver> _observer;
public:
    QtQSSReader();
    ~QtQSSReader();

    void SetEnableObserver(bool enable);
    void Install(const QString& mainQSSFile);

    QString ReadAll();
};

#endif

#endif // QTQSSREADER_H
