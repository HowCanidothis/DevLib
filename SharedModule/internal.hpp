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
#include "Serialization/string/xmlserializer.h"
#include "array.h"
#include "stack.h"
#include "FileSystem/filesguard.h"
#include "Process/processbase.h"
#include "Process/processfactory.h"
#include "Threads/threadcomputingbase.h"
#include "Threads/threadeventshelper.h"
#include "Threads/ThreadFunction/threadfunction.h"
#include "Threads/Promises/promise.h"
#include "Threads/threadsbase.h"
#include "Threads/ThreadFunction/threadpool.h"
#include "Threads/threadtimer.h"
#include "Threads/threadcalculator.h"
#include "delayedcall.h"
#include "FullTextSearch/ftsdictionary.h"
#include "flags.h"
#include "profile_utils.h"
#include "timer.h"
#include "nativetranslates.h"
#include "idgenerator.h"
#include "Logger/logger.h"
#include "dispatcher.h"
#include "events.h"
#include "Trees/cubetree.h"
#include "Trees/quadtreef.h"
#include "interruptor.h"
#include "externalservice.h"
#include "ImportExport/importexport.h"
#include "registrymanager.h"
#include "tokenizer.h"
#include "wrappers.h"
#include "exception.h"
#include "debugobjectinfo.h"
#include "builders.h"
//#include "errors.h"
#endif

#endif

#endif // INTERNAL_H
