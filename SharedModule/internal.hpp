#ifndef SHARED_INTERNAL_HPP
#define SHARED_INTERNAL_HPP

#if defined __cplusplus

#ifdef NO_QT

#include "smartpointersadapters.h"
#include "Process/processbase.h"
#include "nativetranslates.h"
#include "Serialization/streambuffer.h"
#include "Serialization/stdserializer.h"
#include "array.h"
#include "stack.h"
#include "flags.h"

#else

#include "MemoryManager/memorymanager.h"
#include "defaultfactorybase.h"
#include "smartpointersadapters.h"
#include "Serialization/streambuffer.h"
#include "Serialization/stdserializer.h"
#include "Serialization/qserializer.h"
#include "Serialization/libserializer.h"
#include "Serialization/string/textconverter.h"
#include "Serialization/string/qtextconverters.h"
#include "array.h"
#include "stack.h"
#include "FileSystem/filesguard.h"
#include "Process/qprocessbase.h"
#include "Process/processbase.h"
#include "Process/processfactory.h"
#include "Threads/threadcomputingbase.h"
#include "Threads/threadeventshelper.h"
#include "Threads/ThreadFunction/threadfunction.h"
#include "Threads/Promises/promise.h"
#include "Threads/threadsbase.h"
#include "flags.h"
#include "profile_utils.h"
#include "timer.h"
#include "shared_decl.h"
#include "nativetranslates.h"
#include "name.h"
#include "Logger/logger.h"
#include "dispatcher.h"

#endif

#endif

#endif // INTERNAL_H
