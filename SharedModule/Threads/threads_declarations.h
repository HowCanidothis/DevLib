#ifndef THREADS_DECLARATIONS_H
#define THREADS_DECLARATIONS_H

#include "threadsbase.h"

using ThreadHandler = std::function<AsyncResult (const FAction& action)>;
using ThreadHandlerNoThreadCheck = std::function<AsyncResult (const FAction& action)>;

const ThreadHandlerNoThreadCheck ThreadHandlerNoCheckMainLowPriority = [](const FAction& action) -> AsyncResult {
    if(QCoreApplication::instance() == nullptr) {
        return AsyncError();
    }
    return ThreadsBase::DoMainWithResult(CONNECTION_DEBUG_LOCATION, action, Qt::LowEventPriority);
};

const ThreadHandler ThreadHandlerMain = [](const FAction& action) -> AsyncResult {
    if(QCoreApplication::instance() == nullptr) {
        return AsyncError();
    }
    if(QThread::currentThread() == QCoreApplication::instance()->thread()) {
        action();
        AsyncResult result;
        result.Resolve(true);
        return result;
    } else {
        return ThreadsBase::DoMainWithResult(CONNECTION_DEBUG_LOCATION, action);
    }
};

#endif // THREADS_DECLARATIONS_H
