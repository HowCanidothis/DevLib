#ifndef SHARED_INTERNAL_HPP
#define SHARED_INTERNAL_HPP

#if defined __cplusplus

#ifdef NO_QT

#include "smartpointersadapters.h"
#include "Process/processbase.h"
#include "nativetranslates.h"
#include "Serialization/StreamBuffer.h"
#include "Serialization/stdserializer.h"
#include "array.h"
#include "stack.h"
#include "flags.h"

#else

#include "MemoryManager/MemoryManager.h"
#include "defaultfactorybase.h"
#include "smartpointersadapters.h"
#include "Serialization/StreamBuffer.h"
#include "Serialization/stdserializer.h"
#include "Serialization/qserializer.h"
#include "array.h"
#include "stack.h"
#include "Process/qprocessbase.h"
#include "Process/processbase.h"
#include "Threads/threadcomputingbase.h"
#include "Threads/threadeventshelper.h"
#include "Threads/ThreadFunction/threadfunction.h"
#include "flags.h"
#include "profile_utils.h"
#include "timer.h"
#include "shared_decl.h"
#include "nativetranslates.h"
#include "name.h"

#endif

#endif

#endif // INTERNAL_H
