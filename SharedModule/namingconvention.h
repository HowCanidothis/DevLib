#ifndef NAMINGCONVENTION_H
#define NAMINGCONVENTION_H

#include <stdint.h>

typedef int32_t count_t;

struct DummyLogger
{
    template<typename T>
    DummyLogger& operator<<(const T&) { return *this; }
    DummyLogger& operator<<(DummyLogger&) { return *this; }
};

#ifdef NO_QT

#define qCInfo(void) DummyLogger()
#define qCWarning(void) DummyLogger()
#define qCCritical(void) DummyLogger()

#define Q_DECL_OVERRIDE override
#define Q_DECL_NOEXCEPT noexcept

#define Q_UNUSED(x) (void)x;

#include <assert.h>
#define Q_ASSERT(condition) assert(condition)
#define Q_ASSERT_X(condition, what, text) assert(condition)

#define ATTACH_MEMORY_SPY(x)
#define ATTACH_MEMORY_SPY_2(x)

#else

#include <Qt>

#endif

#endif // NAMINGCONVENTION_H
