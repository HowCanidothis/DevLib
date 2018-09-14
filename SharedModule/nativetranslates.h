#ifndef NATIVETRANSLATES_H
#define NATIVETRANSLATES_H

#ifndef NO_QT

#include <QObject>
#define TR_W(text) \
    QObject::tr(text).toStdWString().c_str()
#define TR(text) \
    QObject::tr(text)

#else

#define TR_W(text) L##text
#define TR(text) text

#endif

#endif // TRANSLATES_H
