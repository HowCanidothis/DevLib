#ifndef THREADS_DECLARATIONS_H
#define THREADS_DECLARATIONS_H

#include "threadsbase.h"

using ThreadHandlerNoThreadCheck = std::function<AsyncResult (const FAction& action)>;
using ThreadHandler = std::function<AsyncResult (const FAction& action)>;

const ThreadHandlerNoThreadCheck ThreadHandlerNoCheckMainLowPriority = [](const FAction& action) -> AsyncResult {
    if(QCoreApplication::instance() == nullptr) {
        return AsyncError();
    }
    return ThreadsBase::DoMainWithResult(action, Qt::LowEventPriority);
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
        return ThreadsBase::DoMainWithResult(action);
    }
};

#define THREAD_ASSERT_IS_THREAD(thread) Q_ASSERT(thread == QThread::currentThread());
#define THREAD_ASSERT_IS_NOT_THREAD(thread) Q_ASSERT(thread != QThread::currentThread());
#define THREAD_ASSERT_IS_MAIN() if(qApp != nullptr) { THREAD_ASSERT_IS_THREAD(qApp->thread()); }
#define THREAD_ASSERT_IS_NOT_MAIN() if(qApp != nullptr) { THREAD_ASSERT_IS_NOT_THREAD(qApp->thread()); }

#endif // THREADS_DECLARATIONS_H
