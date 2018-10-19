#ifndef NATIVETRANSLATES_H
#define NATIVETRANSLATES_H

#ifndef NO_QT

#include <QCoreApplication>

#define Q_DECLARE_TR_FUNCTIONS_W(context) \
protected: \
    std::wstring tr(const char* text) const { return QCoreApplication::translate(#context, text, nullptr, -1).toStdWString(); }

#else

#define Q_DECLARE_TR_FUNCTIONS_W(context)
#define tr(text) L##text

#endif

#endif // TRANSLATES_H
