#include "dbarray.h"

DB_BEGIN_NAMESPACE

//===================================================FOR COMPARE STRINGS FROM QT==============================================
#if defined(Q_COMPILER_LAMBDA) && !defined(__OPTIMIZE_SIZE__)
namespace {
template <uint MaxCount> struct UnrollTailLoop
{
    template <typename RetType, typename Functor1, typename Functor2>
    static inline RetType exec(int count, RetType returnIfExited, Functor1 loopCheck, Functor2 returnIfFailed, int i = 0)
    {
        /* equivalent to:
         *   while (count--) {
         *       if (loopCheck(i))
         *           return returnIfFailed(i);
         *   }
         *   return returnIfExited;
         */

        if (!count)
            return returnIfExited;

        bool check = loopCheck(i);
        if (check) {
            const RetType &retval = returnIfFailed(i);
            return retval;
        }

        return UnrollTailLoop<MaxCount - 1>::exec(count - 1, returnIfExited, loopCheck, returnIfFailed, i + 1);
    }

    template <typename Functor>
    static inline void exec(int count, Functor code)
    {
        /* equivalent to:
         *   for (int i = 0; i < count; ++i)
         *       code(i);
         */
        exec(count, 0, [=](int i) -> bool { code(i); return false; }, [](int) { return 0; });
    }
};
template <> template <typename RetType, typename Functor1, typename Functor2>
inline RetType UnrollTailLoop<0>::exec(int, RetType returnIfExited, Functor1, Functor2, int)
{
    return returnIfExited;
}
}
#endif



int DbArrayDelegate<QString>::ucstrncmp_my(const QChar* a, const QChar* b, int l){
#ifdef __OPTIMIZE_SIZE__
    const QChar *end = a + l;
    while (a < end) {
        if (int diff = (int)a->unicode() - (int)b->unicode())
            return diff;
        ++a;
        ++b;
    }
    return 0;
#else
#if defined(__mips_dsp)
    if (l >= 8) {
        return qt_ucstrncmp_mips_dsp_asm(reinterpret_cast<const ushort*>(a),
                                         reinterpret_cast<const ushort*>(b),
                                         l);
    }
#endif // __mips_dsp
#ifdef __SSE2__
    const char *ptr = reinterpret_cast<const char*>(a);
    qptrdiff distance = reinterpret_cast<const char*>(b) - ptr;
    a += l & ~7;
    b += l & ~7;
    l &= 7;

    // we're going to read ptr[0..15] (16 bytes)
    for ( ; ptr + 15 < reinterpret_cast<const char *>(a); ptr += 16) {
        __m128i a_data = _mm_loadu_si128((const __m128i*)ptr);
        __m128i b_data = _mm_loadu_si128((const __m128i*)(ptr + distance));
        __m128i result = _mm_cmpeq_epi16(a_data, b_data);
        uint mask = ~_mm_movemask_epi8(result);
        if (ushort(mask)) {
            // found a different byte
            uint idx = qCountTrailingZeroBits(mask);
            return reinterpret_cast<const QChar *>(ptr + idx)->unicode()
                    - reinterpret_cast<const QChar *>(ptr + distance + idx)->unicode();
        }
    }
#  if defined(Q_COMPILER_LAMBDA)
    const auto &lambda = [=](int i) -> int {
        return reinterpret_cast<const QChar *>(ptr)[i].unicode()
                - reinterpret_cast<const QChar *>(ptr + distance)[i].unicode();
    };
    return UnrollTailLoop<7>::exec(l, 0, lambda, lambda);
#  endif
#endif
#if defined(__ARM_NEON__) && defined(Q_PROCESSOR_ARM_64) // vaddv is only available on Aarch64
    if (l >= 8) {
        const QChar *end = a + l;
        const uint16x8_t mask = { 1, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, 1 << 6, 1 << 7 };
        while (a + 7 < end) {
            uint16x8_t da = vld1q_u16(reinterpret_cast<const uint16_t *>(a));
            uint16x8_t db = vld1q_u16(reinterpret_cast<const uint16_t *>(b));

            uint8_t r = ~(uint8_t)vaddvq_u16(vandq_u16(vceqq_u16(da, db), mask));
            if (r) {
                // found a different QChar
                uint idx = qCountTrailingZeroBits(r);
                return (int)a[idx].unicode() - (int)b[idx].unicode();
            }
            a += 8;
            b += 8;
        }
        l &= 7;
    }
    const auto &lambda = [=](int i) -> int {
        return a[i].unicode() - b[i].unicode();
    };
    return UnrollTailLoop<7>::exec(l, 0, lambda, lambda);
#endif // __ARM_NEON__
    if (!l)
        return 0;

    // check alignment
    if ((reinterpret_cast<quintptr>(a) & 2) == (reinterpret_cast<quintptr>(b) & 2)) {
        // both addresses have the same alignment
        if (reinterpret_cast<quintptr>(a) & 2) {
            // both addresses are not aligned to 4-bytes boundaries
            // compare the first character
            if (*a != *b)
                return a->unicode() - b->unicode();
            --l;
            ++a;
            ++b;

            // now both addresses are 4-bytes aligned
        }

        // both addresses are 4-bytes aligned
        // do a fast 32-bit comparison
        const quint32 *da = reinterpret_cast<const quint32 *>(a);
        const quint32 *db = reinterpret_cast<const quint32 *>(b);
        const quint32 *e = da + (l >> 1);
        for ( ; da != e; ++da, ++db) {
            if (*da != *db) {
                a = reinterpret_cast<const QChar *>(da);
                b = reinterpret_cast<const QChar *>(db);
                if (*a != *b)
                    return a->unicode() - b->unicode();
                return a[1].unicode() - b[1].unicode();
            }
        }

        // do we have a tail?
        a = reinterpret_cast<const QChar *>(da);
        b = reinterpret_cast<const QChar *>(db);
        return (l & 1) ? a->unicode() - b->unicode() : 0;
    } else {
        // one of the addresses isn't 4-byte aligned but the other is
        const QChar *e = a + l;
        for ( ; a != e; ++a, ++b) {
            if (*a != *b)
                return a->unicode() - b->unicode();
        }
    }
    return 0;
#endif
}

DB_END_NAMESPACE
