# Карта кода DevLib

> Карта собирается по папкам скиллом codemap-devlib: у каждой папки свой раздел «## Папка», источники указаны в начале раздела.

> Метка «(?)» означает предположение, которого нет ни в индексе, ни в дампе.

## Оглавление

- [SharedModule](#sharedmodule)
- [PropertiesModule](#propertiesmodule)
- [GraphicsToolsModule](#graphicstoolsmodule)
- [WidgetsModule](#widgetsmodule)
- [CloudModule](#cloudmodule)
- [CommandsModule](#commandsmodule)
- [ComputeGraphModule](#computegraphmodule)
- [NetworkModule](#networkmodule)
- [ProgressModule](#progressmodule)
- [SQLiteModule](#sqlitemodule)
- [StylesModule](#stylesmodule)

## SharedModule

> Источник: индекс `.codemap/SharedModule/` (`codemap: v4`, совпадает с `python3 codemap.py --version`; `00_overview.md`, `index_01.md`: 108 файлов, 15 823 строки; 63 `.h`, 40 `.cpp`, 2 `.hpp`, 2 `.pri`, 1 `.txt`). Связи с другими папками сверены grep-ом по индексам `.codemap/<Папка>/` (у всех папок `codemap: v4`). Типы, которых индекс не показал (`Stack`, `ArrayCommon`, `ScopedPointer`, `ThreadHandlerMain`), найдены grep-ом по исходникам SharedModule. Метки источника: [индекс], [grep], [дамп] (текущий `.codemap/_dump/dump_01.md`: `ImportExport/importexport.h`, `CMakeLists.txt`, `SharedModule.pri`, `StdSharedModule.pri`), [дамп v1] (дамп первой версии карты, сейчас недоступен). Метка «(?)» означает предположение.

### Назначение

Базовая C++/Qt5-библиотека DevLib без собственного UI: умные указатели и контейнеры (`SharedPointer`, `ScopedPointer`, `Array`, `Stack`, `Flags`), интернированные строки `Name`, диспетчер событий `CommonDispatcher`, промисы `Promise`/`AsyncResult`, потоки и пул потоков, отложенные вызовы, прогресс и отмена процессов.
Кроме того: бинарная, XML- и JSON-сериализация, `TextConverter`, импорт и экспорт файлов, логгер, полнотекстовый поиск, quad- и cube-деревья, профилирование.
Единый заголовок `internal.hpp` (PCH) в Qt-ветке подключает 46 заголовков модуля, последним — `builders.h`; в ветке `NO_QT` — 8 [индекс].

### Подмодули

| Путь | Назначение | Ключевые классы / файлы |
|---|---|---|
| `./` (фундамент) | Базовые типы, математика, guards, синглтоны, флаги, свои контейнеры и умные указатели | `declarations.h` (`FAction`, `FTranslationHandler`, `count_t`, `Comparator`, `Nanosecs`, `SerializationMode`, `EPriority`, `AdditionalStandardRoles`, `guards::LambdaGuard/BooleanGuard`, `Singletone<T>`, `SingletoneGlobal<T>`, `adapters::Range/interpolated`, `TCachedExpression`, `EnumHelper`, `CastablePtr`; макрос `SHARED_LIB_ADD_UI` = `QT_GUI_LIB && !FORCE_NO_UI` [grep]), `flags.h` (`Flags<Enum>`), `smartpointersadapters.h` (`SharedPointer`/`SP`, `SPNullable`, `SharedPointerInitialized`, `ScopedPointer` [grep], `SmartPointer`, `SmartPointerValue`), `array.h` (`Array`, `ArrayPointers`, `MiddleAlgoData`; база `ArrayCommon` [grep]), `stack.h` (`StackData`, `StackPointers`; `Stack` [grep]), `namingconvention.h`, `math.h` (`SLMath::Geometry`, `ClosestPoint`) |
| `./` (имена, ID, события) | Интернированные строки, генератор ID, подписки без QObject | `name.h` (`Name`, `Latin1Name`, `UniName`, `StringsMapper`, `StringsToEnumMapper`, `DebugNameManager` [if QT_DEBUG]), `idgenerator.h` (`Id::Id : Name`, `Id::Generator`), `dispatcher.h` (`CommonDispatcher<Args...>`, `Dispatcher = CommonDispatcher<>`, `CommonDispatcherThreadSafe`, `DispatcherConnection(s)`, `DispatcherConnectionSafe`), `events.h` (`EventBase/EventValue/EventVoid`), `debugobjectinfo.h` (`DebugObjectManager`) |
| `./` (отложенные вызовы, отмена) | Debounce, отложенные действия, прерывание операций | `delayedcall.h` (`DelayedCallObject`, `DelayedCall`, `DelayedCallDelayOnCall`, `DelayedCallManager`, `DelayedCallDispatchersCommutator`, `DispatchersCommutator`), `interruptor.h` (`Interruptor`, `AsyncRequest`, `AsyncRequestPending`) |
| `./` (билдеры, фабрики, прочее) | Fluent-строители, LINQ-выборки, парсинг XML/CSV по ключам, фабрики по расширению; исключения, таймеры, внешний процесс, реестр | `builders.h` (`lq::Wrapper/Iterators`, `StringBuilder`, `ContainerBuilder`, `ParseFactoryBuilder`, `FactoryBuilder`, `Factory`), `tokenizer.h` (`Tokenizer`), `defaultfactorybase.h` (`DefaultFactoryBase<Delegate>`), `wrappers.h` (`ContainerWrapper` [дамп]), `exception.h` (`Exception : QException`, `ExceptionHandleParams`), `errors.h` (`TErrorParamsContainer`; не входит в `internal.hpp`), `timer.h` (`Timer`), `profile_utils.h` (`TimerClocks`, `PerformanceClocks`), `externalservice.h` (`ExternalService : QProcess`), `registrymanager.h` (`RegistryManager`; `.cpp` подключает `<windows.h>`, `<shellapi.h>` [if WIN64]), `hacks.h` [if !__MINGW32__], `nativetranslates.h` |
| `External/` | Qt-адаптеры: событие в очередь Qt, лямбда-соединения, наблюдатель, тики шкалы | `QtInlineEvent`, `QtInlineEventWithResult`, `QtLambdaConnections`, `QtObserver` (Q_OBJECT), `TickSplitter`; агрегатор `external.hpp` |
| `FileSystem/` | Ротация файлов по шаблону имени | `FilesGuard`, `FileNamesGeneratorWithGuard` |
| `FullTextSearch/` | Полнотекстовый словарь и поиск | `FTSDictionary`, `FTSObject`, `TFTSObject<T>`, `FTSMatchResult` |
| `ImportExport/` | Импорт и экспорт через источники (файл, буфер, буфер обмена) и фабрики по расширению, версии, контрольная сумма, импорт/экспорт таблиц | `ImportExportSource`/`ImportExportFileSource`/`ImportExportBufferSource`, `ImportExportDelegate<T>`, `ImportExportFactory<T>`, `ImportExportFormatFactory`, `ImportExport`, `ImportExportFilterExtensionsBuilder`; `.cpp` подключает `builders.h`, а `QClipboard`/`QApplication` — только [if SHARED_LIB_ADD_UI] |
| `Logger/` | Логгер-синглтон: файл и консоль, уровни, срок хранения | `Logger : Singletone<Logger>` (`EnableLogging`, `SetSeverity`, `SetMaxDays`, `SetConsoleEnabled`, `Print`, `Terminate`) |
| `MemoryManager/` | Учёт живых объектов, отчёт о памяти | `MemoryManager` (`MakeMemoryReport`), `MemorySpy` |
| `Process/` | Прогресс долгих операций и отмена | `ProcessBase`, `ProcessIncrementGuard`, `ProcessFactory`, `ProcessValue`, `ProcessDeterminateValue`, `DescProcess*ValueState`, `ProcessManager` |
| `Serialization/` | Бинарная сериализация через специализации `Serializer<T>`, версии | `streambuffer.h` (`SerializerBufferBase`, `TSerializerWriteBuffer`/`TSerializerReadBuffer`, `SerializerVersion`, `StandardVariantPropertiesContainer`), `stdserializer.h` (`PlainData`, std-типы), `qserializer.h` (Qt-типы, `SerializeToArray`/`DeSerializeFromArray`), `libserializer.h` (`Name`, `Id::Id`, `Latin1Name`) |
| `Serialization/string/` | XML, JSON, значение ↔ строка | `xmlserializer.h` (`SerializerXml<T>`, `SerializerXmlObject`, `SerializerXmlWrite/ReadBuffer`, `SerializerXmlVersion`), `jsonserializer.h` (`SerializerJsonWrite/ReadBuffer`), `textconverter.h` (`TextConverter<T>`, `TextHelper`), `qtextconverters.h` (QUrl, QDateTime, QImage; QColor и QMatrix4x4 — [if QT_GUI_LIB]), `textconvertercontext.h` |
| `Threads/` | Главный и рабочие потоки, очереди событий потока, таймеры, фоновые вычислители | `ThreadsBase`, `threads_declarations.h` (`ThreadHandler`, `ThreadHandlerNoThreadCheck`; константы `ThreadHandlerMain`, `ThreadHandlerNoCheckMainLowPriority` [grep]), `ThreadEventsContainer`/`ThreadEvent`/`TagThreadEvent`, `MainThreadTaskContainer`, `ThreadComputingBase : QThread`, `ThreadCalculator<T>`, `ThreadTimer`/`ThreadTimerManager` |
| `Threads/Promises/` | Промисы и агрегаторы ожидания | `Promise` (`AsyncResult = Promise`), `PromisedValue<T>`, `FutureResult`, `AsyncSafeResult`, `AsyncError`, `AsyncSuccess`, `QtFutureWatcher<T>` |
| `Threads/ThreadFunction/` | Свой пул потоков с приоритетами | `ThreadPool`, `Thread : QThread`, `ThreadFunction`, `ThreadTaskDesc` |
| `Trees/` | 2D quadtree и 3D cubetree (шаблоны) | `quadtreef.h`/`quadtreef_decl.h` (`BoundingRect`, `Point`; вывод в картинку [if !QUADTREE_NO_IMAGE_OUTPUT]), `cubetree.h`/`cubetree_decl.h` (`BoundingBox`, `Point`), `p_*_branchnode.h` |

### Зависимости

**От каких папок DevLib зависит SharedModule**
- `PropertiesModule` — **условно, при `PROPERTIES_LIB`**. Единственный include из другой папки: `builders.h` → `<PropertiesModule/internal.hpp>` [if PROPERTIES_LIB] [индекс]. Через `builders.h` он попадает в `internal.hpp` (Qt-ветка), `tokenizer.h/.cpp` и `ImportExport/importexport.cpp` [индекс]. Используемые классы (по `uses:` SharedModule и индексу PropertiesModule):
  - `PropertiesModule/localproperty.h`: `LocalPropertySequentialEnum : LocalPropertyInt`, `LocalPropertyBool : LocalProperty<bool>`, `LocalPropertyInt`/`LocalPropertyDouble` (= `LocalPropertyLimitedDecimal<…>`), `LocalPropertyString`, `LocalPropertyName`, `LocalProperty*Optional` — в сигнатурах `ParseFactoryBuilder` [индекс];
  - `PropertiesModule/sharedsettings.h`: `SharedSettings : Singletone<SharedSettings>` — в `builders.cpp` (`StringBuilder::XMLCreateHyperlink`, вариант [if SHARED_LIB_ADD_UI]) [индекс]. `builders.cpp` подключает только `builders.h`, поэтому `SharedSettings` виден лишь при `PROPERTIES_LIB` (`PropertiesModule/internal.hpp` → `sharedsettings.h`) [индекс].
  
  Обратная зависимость безусловна (см. ниже), поэтому получается **цикл SharedModule ↔ PropertiesModule, условный по `PROPERTIES_LIB`**; разбор — в «Подробном разборе».
- `UnitsModule` — косвенно, без include: `ParseFactoryBuilder::RegisterDouble/RegisterMeasurement*` принимают `MeasurementUnit` (часть методов [if PROPERTIES_LIB]), `declarations.h` объявляет `FMeasurementGetter = TCachedExpression<const class Measurement*>` [индекс]. Оба класса определены в `UnitsModule/measurementdeclarations.h` (`MeasurementUnit`) и `UnitsModule/measurementunitmanager.h` (`Measurement`) [индекс UnitsModule]. В SharedModule они известны по forward-declaration (`class MeasurementUnit`, `class Measurement`) [индекс]; для ссылок и указателей этого достаточно (?).
- `WidgetsModule` — **без include, в телах шаблонов `importexport.h`** [дамп]. В `importexport.h` нет ни одного `#include` и ни одного `#if`, кроме include guard [дамп]. Типы WidgetsModule используются без условий:
  - `ImportExport::ImportTable<T>` — внутри лямбды `ThreadHandlerMain(...)`: `WidgetsImportView` (`WidgetsModule/Widgets/widgetsimportview.h`), `DescCustomDialogParams`, `WidgetsDialogsManagerDefaultButtons::Cancel/Insert/ReplaceButton()` (`WidgetsModule/widgetsdeclarations.h`), `WidgetAbstractButtonWrapper`, `WidgetsDialogsManager::GetInstance().ShowTempDialog(..., DescShowDialogParams())` (`WidgetsModule/Managers/widgetsdialogsmanager.h`), `ViewModelsTableBase::DisableMeasurementValidationByEpsilon` (`WidgetsModule/Models/modelstablebase.h`); там же `WidgetProperties::ExtraFieldsCount` [дамп; файлы — индекс WidgetsModule]. `DescShowDialogParams` и `WidgetsDialogsManagerDefaultButtons` в индексе WidgetsModule помечены [if WIDGETS_MODULE_LIB];
  - конструктор `DescImportExportTableExport<T>` (экстрактор по умолчанию) — `WidgetProperties::ExtraFieldsCount` (namespace в `WidgetsModule/widgetsdeclarations.h` и `PropertiesModule/Ui/styleutils.h`) [дамп + индексы];
  - `ExportTable<T>` сам виджетов не использует [дамп].
  
  Эти имена не зависят от `T`. При строгом двухфазном поиске их нужно объявить до определения шаблона, а SharedModule их не подключает. Значит, `importexport.h` собирается только в двух случаях: WidgetsModule (и `PropertiesModule/Ui/styleutils.h`) подключён раньше, либо компилятор не ищет имена строго, например MSVC без `/permissive-` (?). Поскольку WidgetsModule подключает SharedModule, получается неявный цикл SharedModule ↔ WidgetsModule на уровне шаблонов, без условного макроса.
- Остальное — Qt (Core; Gui-типы `QColor`/`QImage`/`QMatrix4x4` [if QT_GUI_LIB], `QPainter`/`QImage` в `Trees/quadtreef.h` [if !QUADTREE_NO_IMAGE_OUTPUT]; `QApplication`/`QClipboard` [if SHARED_LIB_ADD_UI]; `QProcess`, `QSettings`), STL и Windows API (`registrymanager.cpp` [if WIN64]) [индекс].

**Кто зависит от SharedModule** (по строкам `inc:` индексов этих папок)
- `PropertiesModule` [индекс]:
  - `<SharedModule/internal.hpp>` подключают `property.h`, `propertiessystem.h`, `propertiesscope.h`, `propertieslogger.h`, `propertiesfilenamesgenerator.h`, `Models/modelstree.h`, `Models/modelstreeitembase.h`, `Ui/propertieswindow.h`, `Ui/localpropertieswidgetconnector.h` (ещё и `External/external.hpp`);
  - отдельные заголовки: `localpropertydeclarations.h` и `Ui/styleutils.h` → `declarations.h`, `stateproperty.h` → `exception.h`, `propertiessystem.cpp` → `stack.h`, `Threads/threadeventshelper.h`;
  - наследование: `PropertiesLogger : Logger`, `PropertiesFileNamesGenerator : FileNamesGeneratorWithGuard`, `SharedSettings : Singletone<SharedSettings>`, `StateCalculator<T> : ThreadCalculator<T>`.
- `WidgetsModule` [индекс]: `<SharedModule/internal.hpp>` — в 19 файлах (`widgetsdeclarations.h`, `qtqssreader.h`, `Actions/*`, `Attachments/*`, `Delegates/*`, `Models/viewmodelsdefaultfieldmodel.h`, `ProgressBars/*`, `TableViews/widgetstableviewbase.h` и др.); `External/external.hpp` — `qtqssreader.cpp`, `ProgressBars/shadowprogressbar.h`, `TableViews/widgetsadjustabletableview.h`; `hacks.h` — `TableViews/Header/widgetsresizableheaderattachment.cpp`. Наследование: `ActionsManager`, `WidgetsDialogsManager`, `WidgetsStandardTableHeaderManager` : `SingletoneGlobal<…>`, `GlobalTimer : Singletone<GlobalTimer>`. Прогресс-бары (`ProgressBars/mainprogressbar.cpp`, `progressbar.cpp`, `shadowprogressbar.cpp`, `shadowprogressbaroneforall.cpp`) читают `ProcessFactory`/`DescProcess*ValueState` (`uses:`).
- `GraphicsToolsModule` [индекс]: `internal.hpp` (`decl.h`, `gtscene.h`, `gtrendererbase.h`, `gtrenderercontroller.h`, `gtmeshloader.h`, `Objects/gtobjectbase.h`, `Objects/gtmaterialparameterbase.h`), `External/external.hpp` (`Objects/gtobjectbase.h`, `Objects/gtshaderprogram.h`; в `gtcamera.h` — в нижнем регистре `SharedModule/external/external.hpp`), `array.h`, `stack.h`, `flags.h`. `GtRendererBase : ThreadComputingBase`.
- `ComputeGraphModule` [индекс]: `internal.hpp` (`computegraphbase.h`, `inputframestream.h`, `opennisensor.cpp`), `array.h`, `flags.h` (`computenodebase.h`), `profile_utils.h`, `timer.h` (`computegraphbase.cpp`). `GtComputeGraph : ThreadComputingBase`.
- `NetworkModule` [индекс]: `Private/networkthread.h` → `internal.hpp`.
- `ProgressModule` [индекс]: `iprogress.h`, `progressfactory.h` → `internal.hpp`.
- `SQLiteModule` [индекс]: `sqlitedatabase.cpp` → `Threads/ThreadFunction/thread.h`.
- `CloudModule` [индекс]: прямого include нет; `uses:` содержит `ThreadsBase`, `SharedPointer`, `AsyncRequest`. SharedModule приходит транзитивно через `cloudservice.h` → `<PropertiesModule/internal.hpp>` → `property.h` → `<SharedModule/internal.hpp>`.
- `CommandsModule`, `StylesModule` — связей по индексу нет (у CommandsModule единственный include `"operations.h"`; у StylesModule индексирован только `.pri`, `index_*.md` нет).
- Не описаны в CODEMAP, но тоже подключают `<SharedModule/internal.hpp>` [индекс]: `SharedGuiModule/decl.h`, `ControllersModule/controllerbase.h`, `controllerscontainer.h`, `ResourcesModule/resource.h`, `resourcessystem.h`. `UnitsModule` получает SharedModule транзитивно через `<PropertiesModule/internal.hpp>` и `<WidgetsModule/internal.hpp>`.

### Точки входа и сборочные цели

- Сборка: `SharedModule/CMakeLists.txt`, `SharedModule/SharedModule.pri`, `SharedModule/StdSharedModule.pri` (вариант `NO_QT`). В индексе их содержимое пустое, по дампу: `CMakeLists.txt` — `library(TARGET SharedModule PRE_BUILD_LINKS Qt5::Core Qt5::Widgets)`; `SharedModule.pri` — `includeAll(true)`; `StdSharedModule.pri` — `DEFINES += NO_QT`, PCH `internal.hpp`, 3 заголовка и 2 `.cpp` из `process/` [дамп]. Подробности — в «Подробном разборе».
- Единый заголовок: `#include <SharedModule/internal.hpp>` (PCH); `External/external.hpp` — агрегатор Qt-адаптеров.
- `main` нет, это библиотека. Рантайм-входы: `ThreadsBase::Initialize()/Terminate()` (`threadsbase.cpp` подключает `ImportExport/importexport.h` и `ThreadFunction/threadpool.h`); синглтоны `Logger`, `ProcessFactory::Instance()`, `MemoryManager::GetInstance()`, `RegistryManager::GetInstance()`, `QtObserver::Instance()`, `DebugNameManager` [if QT_DEBUG]; статические `DelayedCallManager::Terminate()` и `ThreadTimerManager::Terminate()`; `Exception::Handle(...)`; `PerformanceClocks::PrintReport()`, `MemoryManager::MakeMemoryReport()`.

### Где искать

| Задача | Файлы |
|---|---|
| Свой «сигнал» и подписка | `dispatcher.h/.cpp` |
| Код в фоне или в главном потоке | `Threads/threadsbase.h/.cpp`, `Threads/threads_declarations.h`, `Threads/ThreadFunction/threadpool.h`, `External/qtinlineevent.h` |
| Промисы, `Then`, ожидание нескольких задач | `Threads/Promises/promise.h/.cpp` (`Promise`, `FutureResult`) |
| Debounce, отложенный вызов, таймер | `delayedcall.h/.cpp`, `Threads/threadtimer.h/.cpp` |
| Фоновый пересчёт с отменой | `Threads/threadcalculator.h`, `interruptor.h` |
| Прогресс и отмена долгой операции | `Process/processbase.h`, `Process/processfactory.h`, `Process/processmanager.h`; UI — `WidgetsModule/ProgressBars/` |
| Бинарная сериализация | `Serialization/streambuffer.h`, `stdserializer.h`, `qserializer.h`, `libserializer.h` |
| XML/JSON, версии формата, значение ↔ строка | `Serialization/string/xmlserializer.h`, `jsonserializer.h`, `textconverter.h`, `qtextconverters.h` |
| Импорт/экспорт, фильтры диалога, буфер обмена, таблицы | `ImportExport/importexport.h/.cpp`, `defaultfactorybase.h`; диалог импорта таблицы — `ImportExport::ImportTable` (виджеты из WidgetsModule) |
| Парсинг XML-атрибутов/CSV в поля и `LocalProperty` | `builders.h` (`ParseFactoryBuilder`; часть методов [if PROPERTIES_LIB]) |
| Строки, LINQ-выборки, отсортированный контейнер | `builders.h` (`StringBuilder`, `lq::`), `wrappers.h` |
| `Name`, enum ↔ строка, ID | `name.h/.cpp`, `idgenerator.h/.cpp` |
| Умные указатели, `Array`, `Stack`, `Flags` | `smartpointersadapters.h`, `array.h`, `stack.h`, `flags.h` |
| Логи и ротация файлов | `Logger/logger.h/.cpp`, `FileSystem/filesguard.h/.cpp` |
| Исключения | `exception.h/.cpp`, `errors.h` |
| Замер времени, профилирование, утечки | `timer.h`, `profile_utils.h`, `MemoryManager/memorymanager.h`, `debugobjectinfo.h` |
| Полнотекстовый поиск | `FullTextSearch/ftsdictionary.h/.cpp` |
| Пространственный индекс | `Trees/quadtreef.h`, `Trees/cubetree.h` |
| Деления шкалы, наблюдение за файлом, внешний процесс, реестр | `External/ticksplitter.h`, `External/qtobserver.h`, `externalservice.h`, `registrymanager.h` |
| Условные макросы модуля (`SHARED_LIB_ADD_UI`, `NO_QT`, `PROPERTIES_LIB`) | `declarations.h`, `namingconvention.h`, `internal.hpp`, `builders.h` |
| Сборка | `CMakeLists.txt`, `SharedModule.pri`, `StdSharedModule.pri`, `internal.hpp` |

### Подробный разбор

> Перенесено из прошлой версии раздела и сверено с индексом v4. Сборочные файлы и `importexport.h` сверены с текущим дампом. Остальные сведения «по дампу» (`Global.pri`, `threadsbase.cpp`, номера строк `builders.h`, макрос `PROPERTIES_LIB` в `PropertiesModule.pri`) взяты из дампа первой версии карты и сейчас не перепроверены.

#### Назначение

SharedModule — базовая C++/Qt5 библиотека утилит DevLib, от которой зависят остальные модули: по индексам её подключают PropertiesModule, WidgetsModule, GraphicsToolsModule, ComputeGraphModule, NetworkModule, ProgressModule, SQLiteModule, SharedGuiModule, ControllersModule, ResourcesModule (см. «Зависимости» выше).
Что в ней есть: собственные умные указатели и контейнеры (`SharedPointer`, `Array`, `Stack`, `Flags`), интернированные строки-идентификаторы (`Name`), события и подписки (`CommonDispatcher`), промисы и асинхронность (`Promise`/`AsyncResult`, `ThreadsBase`, `ThreadPool`), отложенные вызовы, сериализация (бинарная через `QDataStream`, XML, JSON, текстовые конвертеры), импорт и экспорт файлов, логгер, прогресс и отмена долгих процессов, полнотекстовый поиск, quad- и oct-деревья (cubetree), профилирование.
Общий заголовок `internal.hpp` подключает модуль целиком и служит precompiled header. У него две ветки: полная Qt-сборка и урезанная `NO_QT` (см. «Точки входа и сборочные цели» ниже).

#### Модули

| Папка / файлы | Назначение | Ключевые классы / файлы | Зависит от |
|---|---|---|---|
| `./` (ядро: декларации) | Базовые типы, математические хелперы, guards, синглтоны, адаптеры диапазонов | `declarations.h` (`Comparator`, `Nanosecs`, `SerializationMode`, `EPriority`, `guards::LambdaGuard/BooleanGuard`, `Singletone<T>`, `adapters::Range`, `TCachedExpression`, `EnumHelper`), `flags.h` (`Flags<Enum>`), `namingconvention.h` | Qt (`QMetaEnum`, `QDateTime`, `QLoggingCategory`), `flags.h`, `debugobjectinfo.h` |
| `./` (умные указатели и контейнеры) | Обёртки над `std::shared_ptr`, свои массив и стек | `smartpointersadapters.h` (`SharedPointer`, `SPNullable`, `ScopedPointer`/`make_scoped`, `SmartPointer`, `SmartPointerValue`), `array.h` (`Array`, `ArrayPointers`, `MiddleAlgoData`), `stack.h` (`Stack`, `StackPointers`) | `<memory>`, `namingconvention.h`, `declarations.h` |
| `./` (имена и ID) | Интернированные строковые ключи и генератор ID | `name.h` (`Name`, `Latin1Name`, `UniName`, `StringsMapper`, `StringsToEnumMapper`, `DebugNameManager`), `idgenerator.h` (`Id::Id`, `Id::Generator`) | `declarations.h`, `smartpointersadapters.h` |
| `./` (диспетчер событий) | Подписки «сигнал → обработчик» без QObject, управление соединениями | `dispatcher.h` (`CommonDispatcher<Args...>`, `CommonDispatcherThreadSafe`, `DispatcherConnection(s)`, `DispatcherConnectionSafe`), `events.h` (`EventBase/EventValue/EventVoid`), `debugobjectinfo.h` (`DebugObjectManager`) | `MemoryManager/memorymanager.h`, `smartpointersadapters.h`, `stack.h`, `QMutex` |
| `./` (отложенные вызовы и прерывания) | Debounce и отложенные действия, отмена операций | `delayedcall.h` (`DelayedCallObject`, `DelayedCall`, `DelayedCallDelayOnCall`, `DelayedCallManager`, `DelayedCallDispatchersCommutator`), `interruptor.h` (`Interruptor`, `AsyncRequestPending`) | `Threads/Promises/promise.h`, `dispatcher.h`, `Threads/threadsbase.h`, `Threads/threadtimer.h` |
| `./` (билдеры и фабрики) | Fluent-построители строк и контейнеров, LINQ-подобная выборка, фабрики парсинга XML/CSV по ключам | `builders.h` (`lq::Wrapper/Iterators`, `StringBuilder`, `ContainerBuilder`, `ParseFactoryBuilder`, `FactoryBuilder`, `Factory`), `defaultfactorybase.h` (`DefaultFactoryBase<Delegate>` — выбор по расширению файла), `tokenizer.h` (`Tokenizer`) | **`<PropertiesModule/internal.hpp>`**, но только под `#ifdef PROPERTIES_LIB`. `ParseFactoryBuilder` использует `LocalProperty*` и `MeasurementUnit`; под тот же `#ifdef` убрана лишь часть этих методов (см. «Потоки данных и связи»). Сам `builders.h` не подключает заголовки SharedModule, а `Name`, `QString` и прочее берёт из окружения, то есть из `internal.hpp`, где он подключается последним |
| `./` (прочее) | Исключения, таймеры и профилирование, внешний процесс, реестр Windows, сортированные контейнеры | `exception.h` (`Exception`, `ExceptionHandleParams`), `timer.h` (`Timer`), `profile_utils.h` (`TimerClocks`, `PerformanceClocks`), `externalservice.h` (`ExternalService : QProcess`), `registrymanager.h` (`RegistryManager`; `.cpp` подключает `<windows.h>`, только Windows), `errors.h` (`TErrorParamsContainer`; в `internal.hpp` закомментирован), `math.h` (`SLMath::Geometry`, `ClosestPoint`), `hacks.h` (макросы доступа к private-членам), `wrappers.h` (`ContainerWrapper<T, ComparisonTarget, Container=QVector<T>>`: отсортированный контейнер с бинарным поиском `Find/FindExact/FindOrCreate/Insert/Remove/Sort`), `nativetranslates.h` | `declarations.h`, `dispatcher.h`, `Threads/*`, `QProcess`, `QSettings` |
| `External/` | Qt-адаптеры: inline-события в очередь Qt, лямбда-соединения, наблюдатель за условиями и файлами, разбивка шкалы на тики | `QtInlineEvent`, `QtInlineEventWithResult`, `QtLambdaConnections`, `QtObserver` (Q_OBJECT, `Instance()`), `TickSplitter`; агрегатор `external.hpp` | `QEvent`, `QObject`, `QTimer`, `promise.h`, `internal.hpp` |
| `FileSystem/` | Ротация файлов по шаблону и максимальному числу | `FilesGuard`, `FileNamesGeneratorWithGuard` | `dispatcher.h`, `QDir` |
| `FullTextSearch/` | Полнотекстовый словарь и сопоставление строк | `FTSDictionary`, `FTSObject`, `TFTSObject<T>`, `FTSMatchResult` | `internal.hpp` |
| `ImportExport/` | Импорт и экспорт объектов и таблиц через источники (файл, буфер, буфер обмена) и фабрики по расширению; версии форматов, контрольная сумма | `ImportExportSource` / `ImportExportFileSource` / `ImportExportBufferSource`, `ImportExportDelegate<T>`, `ImportExportFactory<T>`, `ImportExportFormatFactory`, `ImportExport` (статические `Import/Export/ImportTable/ExportTable/Async`, `Standard*Device`), `ImportExportFilterExtensionsBuilder` | `builders.h`, сериализаторы (`SerializerVersion`, `SerializerXmlVersion`, JSON), `QApplication`/`QClipboard` (только [if SHARED_LIB_ADD_UI]), `QCryptographicHash`, свой пул потоков; в шаблоне `.h` без include — типы WidgetsModule (`WidgetsImportView`, `WidgetsDialogsManager`, `ViewModelsTableBase`) |
| `Logger/` | Логгер-синглтон: файл и консоль, уровни, хранение логов N дней | `Logger : Singletone<Logger>` (`EnableLogging`, `SetSeverity`, `SetMaxDays`, `Print`, `Terminate`) | `External/external.hpp`, `Threads/threadsbase.h`, `FileSystem/filesguard.h` |
| `MemoryManager/` | Отслеживание живых объектов и отчёт о памяти | `MemoryManager` (`MakeMemoryReport`, `TestObjects<T>`), `MemorySpy` | `QHash`, `QMutex` |
| `Process/` | Модель прогресса долгих операций: (не)детерминированные процессы, отмена | `ProcessBase`, `ProcessIncrementGuard`, `ProcessFactory::Instance()`, `ProcessValue`, `ProcessDeterminateValue`, `DescProcess*ValueState`, `ProcessManager` (весь API private, `friend ProcessValue`; хранит `Array<qint32>` под `QMutex`, `registerNewProcessValue()` / `unregisterProcessValue(depth)` — учёт глубины вложенных процессов) | `interruptor.h`, `name.h`, `dispatcher.h` |
| `Serialization/` | Бинарная сериализация через специализации `Serializer<T>` (std, Qt и библиотечные типы), версии | `streambuffer.h` (`SerializerBufferBase`, `TSerializerWriteBuffer`, `TSerializerReadBuffer`, `SerializerVersion`), `stdserializer.h` (`PlainData`, std-контейнеры, указатели), `qserializer.h` (Qt-типы; `SerializeToArray`, `DeSerializeFromArray`, `SerializeCopyObject`, `*Versioned`), `libserializer.h` (`Id`) | `smartpointersadapters.h`, `array.h`, `stack.h`, `flags.h`, `name.h`, `idgenerator.h`, `QDataStream` |
| `Serialization/string/` | Текстовая сериализация (XML, JSON) и `TextConverter<T>` (значение ↔ строка) | `xmlserializer.h` (`SerializerXml<T>`, `SerializerXmlObject`, `SerializerXmlWriteBuffer/ReadBuffer`, `SerializerXmlVersion`, `SerializeToXML`, `DeSerializeFromXML`), `jsonserializer.h` (`SerializerJsonWriteBuffer/ReadBuffer`, `SerializeToJson`, `DeSerializeFromJson`), `textconverter.h` (`TextHelper`), `qtextconverters.h` (конвертеры для QUrl, QColor, QImage, QMatrix4x4 и т. д.), `textconvertercontext.h` (точность и формат изображений) | `qserializer.h`, `declarations.h`, `QXmlStreamReader`, `QJsonDocument` |
| `Threads/` | Главный поток и рабочие потоки, очереди событий потока, таймеры, фоновые вычислители | `ThreadsBase` (`Initialize`, `DoMain*`, `Async`, `DoQThreadWorker*`, `FreeAtMainThread`, `Terminate`), `threads_declarations.h` (`ThreadHandler` / `ThreadHandlerNoThreadCheck` = `std::function<AsyncResult(const FAction&)>`; готовые `ThreadHandlerMain` и `ThreadHandlerNoCheckMainLowPriority`), `ThreadEventsContainer`/`ThreadEvent`/`TagThreadEvent`, `ThreadComputingBase : QThread` (цикл с целевым временем кадра), `ThreadCalculator<T>`, `ThreadTimer`/`ThreadTimerManager` (`SingleShot*`) | `promise.h`, `dispatcher.h`, `External/qtinlineevent.h`, `ThreadFunction/`, `ImportExport/importexport.h` (`threadsbase.cpp` управляет `ImportExport::threadPool()`) |
| `Threads/Promises/` | Промисы с кодом результата `qint8`, ожидание, перенос продолжения в главный поток | `Promise` (`Then`, `Resolve`, `Wait`, `MoveToMain`, `ThenException`, `MakeSafe`, статический `OnFirstInvokeWithResult(dispatcher)`); **`using AsyncResult = Promise;`**; `PromiseData` (`CommonDispatcher<qint8> onFinished`, `std::exception_ptr`), `FutureResult` (агрегирует промисы через `+=`, `Wait`), `AsyncSafeResult`, `AsyncError`, `AsyncSuccess`, `PromisedValue<T>` (цепочка `Then` с данными типа `T`), `QtFutureWatcher` (`QFuture` → `Promise.Resolve(true)`) | `dispatcher.h`, `QFuture`; `ATTACH_MEMORY_SPY` → `MemoryManager` |
| `Threads/ThreadFunction/` | Собственный пул потоков с приоритетами | `ThreadPool` (`PushTask`, `Await`, `Terminate`), `Thread : QThread`, `ThreadFunction::Async`, `ThreadTaskDesc` | `stack.h`, `promise.h` |
| `Trees/` | Шаблонные пространственные деревья: 2D quadtree и 3D октодерево (cubetree), отладочная отрисовка и печать | `quadtreef.h` / `quadtreef_decl.h` (`Tree<ObjectNode, MAX>`, `BoundingRect`, `Point`), `cubetree.h` / `cubetree_decl.h` (`Tree`, `BoundingBox`, `Point`), `p_*_branchnode.h` | `dispatcher.h`, `QPainter`/`QImage` |

#### Точки входа и сборочные цели

**Сборочные цели** (три сборочных файла сверены с текущим дампом; `Global.pri` — по дампу первой версии)
- `SharedModule/CMakeLists.txt`: только `library(TARGET SharedModule PRE_BUILD_LINKS Qt5::Core Qt5::Widgets)`; вызова `includeAll` в нём нет (исправлено по текущему дампу). Это библиотека SharedModule на Qt5 Core и Widgets; как CMake собирает список исходников, неизвестно (?). CMake-макросы `library` и `includeAll` в дампе не найдены: `Global.pri` содержит только qmake-версию `includeAll`. Где лежит CMake-определение (?), неизвестно.
- `SharedModule/SharedModule.pri`: `includeAll(true)`. Это qmake-функция `defineTest(includeAll)` из корневого `Global.pri`, она:
  - рекурсивно собирает `*.cpp → SOURCES`, `*.h → HEADERS`, `*.ui → FORMS`, `*.qrc → RESOURCES`, **`*.hpp → PRECOMPILED_HEADER`**;
  - с аргументом `true` исключает все `*main.cpp`. По комментарию в `Global.pri`, в `.pri` аргумент всегда `true`, в `.pro` — `false`;
  - при `CONFIG += excludeUi` пропускает файлы из папки `Ui/` и ставит `DEFINES += FORCE_NO_UI`.
  
  Под маску `*.hpp` попадают оба `.hpp` модуля, `internal.hpp` и `External/external.hpp`, то есть оба уходят в `PRECOMPILED_HEADER`. Комментарий в `Global.pri` утверждает, что повторный вызов не дублирует файлы. По коду: `includeFiles` добавляет файлы через `*=` (уникальное добавление), отслеживания по `DEFINES` в коде нет.
- Прочее в `Global.pri`:
  - `setBuildDirectory(path)` раскладывает `DESTDIR` в `path_debug`, `path_release` или `path_profile` при `force_debug_info`, тогда же `DEFINES += QT_PROFILE`;
  - `makeDeploy` копирует `DEPLOY_FILES` и `DEPLOY_DIRS`, затем запускает `windeployqt`;
  - переменная окружения `BUILD_CONFIG` добавляется в `CONFIG`.
- `SharedModule/StdSharedModule.pri`: `DEFINES += NO_QT`, PCH `internal.hpp`, явно перечислены `HEADERS` `process/iprocess.h`, `process/processbase.h`, `process/processfactory.h` и `SOURCES` `process/processbase.cpp`, `process/processfactory.cpp` [дамп]. Путь `process/` написан в нижнем регистре, а папка в индексе — `Process/`; на регистрозависимой ФС пути не совпадут (?). С `NO_QT` файл `internal.hpp` подключает только `smartpointersadapters.h`, `Process/processbase.h`, `nativetranslates.h`, `Serialization/streambuffer.h`, `stdserializer.h`, `array.h`, `stack.h`, `flags.h`. Это урезанная сборка «без Qt». При этом `processbase.h` → `interruptor.h` → `dispatcher.h`/`threadtimer.h` тянут Qt; работает ли вариант `NO_QT` на деле (?), неизвестно. Файла `iprocess.h` в индексе нет, поэтому `.pri` выглядит устаревшим (?).

**Точки входа в рантайме** (функции `main` в модуле нет, это библиотека)
- `#include <SharedModule/internal.hpp>` — единый заголовок. В Qt-ветке (без `NO_QT`) подключает 46 заголовков модуля [индекс], включая `builders.h`, а через него при `PROPERTIES_LIB` — `PropertiesModule/internal.hpp`. `errors.h` в списке нет (по дампу — закомментирован).
- `ThreadsBase::Initialize()` создаёт два пула: `ThreadFunction::threadPool()` и `ImportExport::threadPool()`. `ThreadsBase::Terminate()` завершает оба пула. `ThreadsBase::IsTerminated()` проверяет пул `ThreadFunction`. Кто вызывает `Initialize`, в модуле не видно; вероятно, приложение (?).
- Синглтоны и глобальные менеджеры: `Logger` (создаётся с `QDir`), `ProcessFactory::Instance()`, `ProcessManager::getInstance()` (private, только для `ProcessValue`), `MemoryManager::GetInstance()`, `RegistryManager::GetInstance()`, `QtObserver::Instance()`, `DelayedCallManager` (статический, `Terminate()`), `ThreadTimerManager` (статический, `Terminate()`).
- Обработка ошибок верхнего уровня: `Exception::Handle(module, action)` / `Exception::Handle(ExceptionHandleParams)`.

#### Потоки данных и связи

```
declarations.h / flags.h / smartpointersadapters.h / array.h / stack.h   <- фундамент
        |
     name.h, idgenerator.h, MemoryManager
        |
   dispatcher.h (CommonDispatcher)  -----------------------------+
        |                                                         |
   Threads/Promises/promise.h (Promise, AsyncResult, FutureResult)|
        |                                                         |
   Threads/ (ThreadsBase, ThreadPool, ThreadTimer, ThreadEvents) <+-- External/qtinlineevent (очередь Qt)
        |                                                         |
   delayedcall.h, interruptor.h, Process/ -----------------------+
        |
   Serialization/ (binary) -> Serialization/string/ (XML, JSON, TextConverter)
        |
   ImportExport/ (источники + фабрики по расширению -> сериализаторы, Async в пуле)
        |
   Logger/ (FilesGuard, ThreadsBase, external.hpp)
```

- **Асинхронность.** `AsyncResult` — это псевдоним `Promise` (`using AsyncResult = Promise;`). Результат имеет тип `qint8`: `true` значит успех, `false` — ошибку или отмену. `ThreadsBase::Async(task)` напрямую вызывает `ThreadFunction::Async(task, priority)`, тот ставит задачу в `ThreadPool::PushTask`. Продолжения регистрируются через `Promise::Then(qint8)`; внутри это подписка на `CommonDispatcher<qint8> PromiseData::onFinished`. `MoveToMain` переносит продолжение в главный поток. Несколько промисов собираются в `FutureResult` (`result += promise; result.Wait()`). `AsyncSemaphore` добавляет `Async`-задачу в общий `FutureResult`.
- **Главный поток.** Макрос `ENABLE` в `threadsbase.cpp` закомментирован. Поэтому рабочий путь такой: `DoMain` → `QtInlineEvent::Post`, `DoMainWithResult` → `QtInlineEventWithResult::Post`. Ветка `MainThreadTaskContainer` (своя очередь, Block/Unblock для `ProcessUiOnly`) сейчас мёртвый код: `ProcessUiOnly` ничего не делает, `ProcessMainEvents` сводится к `qApp->processEvents()`. `DoMainAwait` в главном потоке выполняет задачу сразу, из другого потока ждёт через `FutureResult::Wait()`; без `QCoreApplication` просто возвращается. `DoQThreadWorker*` отправляет событие в QObject-воркер, а сам `QThread` туда передавать нельзя (`Q_ASSERT`).
- **ThreadHandler.** `ThreadHandler` — это `std::function<AsyncResult(const FAction&)>`, стратегия «где выполнить». `ThreadHandlerMain` выполняет задачу синхронно, если уже в главном потоке, иначе вызывает `DoMainWithResult`. `ThreadHandlerNoCheckMainLowPriority` всегда вызывает `DoMainWithResult` с `Qt::LowEventPriority`. Оба без `QCoreApplication` возвращают `AsyncError()`. `ThreadHandlerNoCheckMainLowPriority` — значение по умолчанию в `DelayedCallObjectParams`; его же принимают `QtObserver` и `ThreadCalculator`.
- **События.** `CommonDispatcher<Args...>` — основной механизм подписок: `Connect`, `ConnectAndCall`, `ConnectCombined`, `OnFirstInvoke`. Соединения хранятся в `DispatcherConnections(Safe)` и связаны с `DebugObjectManager`. `DelayedCallDispatchersCommutator` объединяет несколько `Invoke` в один отложенный вызов. `Interruptor::OnInterrupted()` возвращает `Dispatcher`.
- **Отложенные вызовы.** `DelayedCallManager::CallDelayed(…, DelayedCallObject | Name key, action)` → `ThreadTimer` или `ThreadHandler` → `AsyncResult`.
- **Прогресс и отмена.** Код операции вызывает `ProcessBase::BeginProcess/IncrementProcess`. `ProcessFactory` создаёт `ProcessValue`. `ProcessManager` (доступен только `ProcessValue`) выдаёт и освобождает глубину вложенности (`registerNewProcessValue` / `unregisterProcessValue(depth)`), а `Interruptor` отменяет работу. Состояние отдаётся наружу через `DescProcess*ValueState`; его читают прогресс-бары `WidgetsModule/ProgressBars/` (`mainprogressbar.cpp`, `progressbar.cpp`, `shadowprogressbar.cpp`, `shadowprogressbaroneforall.cpp` — `uses:` индекса WidgetsModule).
- **Сериализация.** Объект сериализуется через `Serializer<T>::Write/Read(Buffer&)` (бинарный формат) или `SerializerXml<T>` с `buffer.Sect/Attr` (XML и JSON). Буферы `TSerializer*Buffer`, `SerializerXml*Buffer` и `SerializerJson*Buffer` имеют общий интерфейс `OpenSection/CloseSection/BeginArray/KeyValue`. Версионирование: `SerializerVersion` (бинарный формат) и `SerializerXmlVersion` (XML и JSON).
- **Импорт и экспорт.** `ImportExportSource` (файл, буфер, буфер обмена) → `ImportExportFactory<T>` (ключ — расширение) → `ImportExportDelegate<T>` (в т. ч. `CreateSerializer{Xml,Json,}StandardDelegate`) → `ImportExport::Standard*Device` → `AsyncResult`. Защита целостности — `WriteSecuritySum` и `CheckSecuritySum` (`QCryptographicHash`).
- **Парсинг конфигов.** `ParseFactoryBuilder` (`builders.h`) регистрирует ключи и экстракторы в поля или `LocalProperty*` и разбирает `QXmlStreamReader`-атрибуты или CSV. Отсюда зависимость SharedModule → PropertiesModule.
- **Цикл SharedModule ↔ PropertiesModule: есть, на уровне заголовков, включается макросом `PROPERTIES_LIB`.**
  - *PropertiesModule → SharedModule* (подтверждено индексом DevLib). `PropertiesModule/property.h`, `propertiessystem.h`, `propertiesscope.h`, `propertieslogger.h`, `propertiesfilenamesgenerator.h`, `Models/modelstree.h`, `Models/modelstreeitembase.h` подключают `<SharedModule/internal.hpp>`. `localpropertydeclarations.h` подключает `<SharedModule/declarations.h>`, `stateproperty.h` — `SharedModule/exception.h`, `propertiessystem.cpp` — `SharedModule/stack.h` и `Threads/threadeventshelper.h`. Сам `PropertiesModule/internal.hpp` SharedModule напрямую не подключает: зависимость идёт через `property.h` и остальные заголовки.
  - *SharedModule → PropertiesModule* (индекс: `builders.h` `inc: <PropertiesModule/internal.hpp>[if PROPERTIES_LIB]`; кроме того, `builders.cpp` использует `SharedSettings` из `PropertiesModule/sharedsettings.h`). Макрос задаётся в `PropertiesModule/PropertiesModule.pri` (`DEFINES += PROPERTIES_LIB`). Значит, в любом qmake-проекте, который подключает `PropertiesModule.pri`, цепочка `SharedModule/internal.hpp` → `builders.h` → `PropertiesModule/internal.hpp` → `property.h` → `SharedModule/internal.hpp` замыкается. В CMake-сборке, где задаётся `PROPERTIES_LIB`, неизвестно (?).
  - *Как цикл разрывается.* Только include guard'ами (`SHARED_INTERNAL_HPP`, `BUILDERS_H`, `PROPERTIES_INTERNAL_HPP`), поэтому результат зависит от того, какой заголовок подключён первым:
    - если первым подключён `SharedModule/internal.hpp`, то `builders.h` идёт в нём последним. К моменту входа в PropertiesModule весь остальной SharedModule уже объявлен, а повторный `SharedModule/internal.hpp` из `property.h` ничего не делает. Не объявлено только содержимое самого `builders.h` (`StringBuilder`, `ParseFactoryBuilder`, `Factory`, `lq::`);
    - если первым подключён `PropertiesModule/…` (например, `property.h`), то `SharedModule/internal.hpp` → `builders.h` → `PropertiesModule/internal.hpp` пропускается guard'ом. Тело `builders.h` разбирается до того, как объявлены `LocalProperty*` и `MeasurementUnit`.
  - *Неполная защита `#ifdef`.* По индексу [if PROPERTIES_LIB] помечены: в `ParseFactoryBuilderObject` — `RegisterPropertyEnum`, `Insert`, `RegisterDouble(MeasurementUnit, double | LocalPropertyDouble)`; в `ParseFactoryBuilder` — `RegisterPropertyEnum`, `RegisterProperty`, `RegisterId(LocalPropertyName)`, `RegisterMeasurementOptional`, `RegisterMeasurementPropertyOptional`, `RegisterMeasurementProperty`, `RegisterMeasurementField` (по дампу — строки 379–417 и 672–761). Без пометки остаются методы с `LocalPropertyBool`, `LocalPropertyName`, `LocalPropertyInt(Optional)`, `LocalPropertyDouble(Optional)`, `LocalPropertyString(Optional)`, `LocalPropertySequentialEnum` (параметр по умолчанию) и `MeasurementUnit`: `RegisterBool`, `RegisterId`, `RegisterDouble` (в т. ч. с `MeasurementUnit` и `std::optional<double>`/`LocalPropertyDoubleOptional`), `RegisterString`, `RegisterEnum`. Это невынесенные (non-dependent) имена в членах шаблонного класса. Без `PROPERTIES_LIB` или при порядке «PropertiesModule первым» такой код должен собираться только на компиляторе без строгого двухфазного поиска имён, например MSVC без `/permissive-` (?).
  - *Следствие для `NO_QT`.* В ветке `NO_QT` файл `internal.hpp` не подключает `builders.h`, поэтому цикла там нет.
- **Threads ↔ ImportExport.** `ThreadsBase::Initialize/Terminate` управляет `ImportExport::threadPool()`, поэтому `threadsbase.cpp` подключает `importexport.h`. `ImportExport::Async` в свою очередь выполняет задачи в этом отдельном пуле. Получается двунаправленная связь на уровне `.cpp`.

#### Где искать

| Задача | Файлы |
|---|---|
| Подписаться на событие или создать свой «сигнал» | `SharedModule/dispatcher.h`, `dispatcher.cpp` |
| Выполнить код в фоне или в главном потоке | `Threads/threadsbase.h/.cpp`, `Threads/threads_declarations.h` (`ThreadHandlerMain`), `Threads/ThreadFunction/threadpool.h`, `External/qtinlineevent.h` |
| Промисы, ожидание результата, цепочки `Then` | `Threads/Promises/promise.h/.cpp` |
| Debounce или отложенный вызов | `delayedcall.h/.cpp`, `Threads/threadtimer.h/.cpp` |
| Таймер в отдельном потоке, single-shot | `Threads/threadtimer.h` |
| Фоновый пересчёт с отменой | `Threads/threadcalculator.h`, `interruptor.h` |
| Поток с фиксированным временем кадра | `Threads/threadcomputingbase.h/.cpp` |
| Прогресс-бар и отмена долгой операции | `Process/processbase.h`, `Process/processfactory.h`, `Process/processmanager.h`, `interruptor.h` |
| Бинарная сериализация своего типа | `Serialization/streambuffer.h`, `Serialization/stdserializer.h`, `Serialization/qserializer.h` |
| XML или JSON сериализация, версии формата | `Serialization/string/xmlserializer.h/.cpp`, `Serialization/string/jsonserializer.h/.cpp` |
| Конвертация значения в строку и обратно (цвет, URL, изображение, матрица) | `Serialization/string/qtextconverters.h`, `textconverter.h`, `textconvertercontext.h` |
| Импорт или экспорт файла, фильтры диалогов, буфер обмена | `ImportExport/importexport.h/.cpp`, `defaultfactorybase.h` |
| Парсинг XML-атрибутов или CSV в поля и свойства | `builders.h` (`ParseFactoryBuilder`; свойства и единицы измерения — только при `PROPERTIES_LIB`) |
| Разобраться с циклом SharedModule ↔ PropertiesModule | `SharedModule/builders.h` (по дампу строки 4–6, 379–417, 672–761), `SharedModule/builders.cpp` (`SharedSettings`), `SharedModule/internal.hpp`, `PropertiesModule/internal.hpp`, `PropertiesModule/property.h`, `PropertiesModule/PropertiesModule.pri` |
| Склейка строк, LINQ-выборки по контейнерам | `builders.h` (`StringBuilder`, `lq::`, `ContainerBuilder`) |
| Отсортированный вектор с поиском по ключу | `wrappers.h` (`ContainerWrapper`) |
| Строковые ключи, enum ↔ строка | `name.h/.cpp` (`Name`, `StringsToEnumMapper`) |
| Генерация уникальных ID | `idgenerator.h/.cpp` |
| Флаги на enum | `flags.h` |
| Умные указатели, `make_shared` | `smartpointersadapters.h/.cpp` |
| Логирование в файл, ротация логов | `Logger/logger.h/.cpp`, `FileSystem/filesguard.h/.cpp` |
| Исключения и глобальная обработка ошибок | `exception.h/.cpp`, `errors.h` |
| Профилирование и замер времени | `profile_utils.h/.cpp`, `timer.h/.cpp` |
| Утечки и учёт объектов | `MemoryManager/memorymanager.h/.cpp`, `debugobjectinfo.h/.cpp` |
| Полнотекстовый поиск | `FullTextSearch/ftsdictionary.h/.cpp` |
| Пространственный индекс 2D и 3D | `Trees/quadtreef.h`, `Trees/cubetree.h` (+ `*_decl.h`, `p_*_branchnode.h`) |
| Шаги делений на осях графиков | `External/ticksplitter.h/.cpp` |
| Наблюдение за файлом или условием | `External/qtobserver.h/.cpp` |
| Запуск и перезапуск внешнего процесса | `externalservice.h/.cpp` |
| Реестр Windows и настройки продукта | `registrymanager.h/.cpp` |
| Математика: clamp, lerp, fuzzyCompare, round, интерполяция | `declarations.h` (`adapters::interpolated`), `math.h` |
| Сборка и подключение модуля | `SharedModule/CMakeLists.txt`, `SharedModule/SharedModule.pri`, `SharedModule/StdSharedModule.pri`, `SharedModule/internal.hpp` (PCH), `Global.pri` (`includeAll`, `setBuildDirectory`, `makeDeploy`) |

## PropertiesModule

> Источники: индекс `.codemap/PropertiesModule/` (`00_overview.md`, `index_01.md`; `codemap: v4` совпадает с `python3 codemap.py --version`; 81 файл, 14 229 строк: 41 `.h`, 36 `.cpp`, 2 `.hpp`, 1 `.txt`, 1 `.pri`). Типы из строк `uses:` сверены grep-ом по индексам `.codemap/SharedModule/`, `.codemap/SharedGuiModule/`, `.codemap/WidgetsModule/`, `.codemap/UnitsModule/` (все v4); не найденные в индексах — grep по исходникам. Обратные связи — по строкам `inc:` индексов CloudModule, ComputeGraphModule, GraphicsToolsModule, SQLiteModule, SharedModule, UnitsModule, WidgetsModule. Метки: [индекс], [grep], [сборка] — файл сборки прочитан напрямую, так как в индексе его содержимое пустое. «(?)» — предположение.

### Назначение

Система свойств DevLib на C++/Qt5. Глобальные именованные свойства `Property`/`TProperty<T>` живут в скоупах `PropertiesSystem`/`PropertiesScope` и загружаются/сохраняются через `QSettings`. Локальные реактивные значения `LocalProperty<T>` имеют валидаторы и подписки. Цепочки фоновых вычислений строятся как `StateParameters` → `StateCalculator<T>` [индекс].
Кроме того: модель ошибок (`LocalPropertyErrorsModel`), обёртки данных для Qt-моделей (`Models/`), UI-слой (модели и виды свойств, делегаты, связывание свойств с виджетами, `Ui/`), синглтон настроек `SharedSettings`, переводы `TranslatorManager`/`TranslatedString`, транзакционное редактирование `EditingObject` [индекс].
Единый заголовок `internal.hpp` подключает ядро и `Models/*`, а `Ui/internal.hpp` — только `[if SHARED_LIB_ADD_UI]`. Макрос объявлен в `SharedModule/declarations.h` (строка `defines:` индекса SharedModule); при каком условии он задаётся, из индекса не видно (?) [индекс].

### Подмодули

| Путь | Назначение | Ключевые классы / файлы |
|---|---|---|
| `./` (глобальные свойства) | Именованные свойства с путём `Name`, опциями (`Options`), делегатами UI (`DelegateValue`), min/max, подпиской | `property.h/.cpp` (`Property`, `TPropertyBase<T>`, `TProperty<T>`, `TDecimalProperty<T>`, `PointerProperty<T>`, `SharedPointerProperty<T>`, `FileNameProperty`, `NamedUIntProperty`, `UrlListProperty`, `HashProperty`, `SetProperty`, `ListProperty`, `VariantProperty`, `PropertiesDialogGeometryProperty`; псевдонимы `BoolProperty`, `IntProperty`, `UIntProperty`, `FloatProperty`, `DoubleProperty`, `StringProperty`, `UrlProperty`, `ByteArrayProperty`; `[if QT_GUI_LIB]`: `ColorProperty`, `RectProperty : TProperty<RectI>`, `Vector3FProperty`), `externalproperty.h/.cpp` (`ExternalPropertyProperty` — ссылка на свойство в другом скоупе; `TExternalPropertyBase<T>` — getter/setter или ссылка `T&`; `TExternalProperty<T>`, `TExternalDecimalProperty<T>`, `ExternalNameProperty`, `ExternalFileNameProperty`, `ExternalNamedUIntProperty`, `ExternalStdWStringProperty`, `ExternalTextFileNameProperty`, псевдонимы `External{String,Bool,Double,Float,Int,UInt}Property`; `[if QT_GUI_LIB]`: `ExternalColorProperty`, `ExternalVector3FProperty`) |
| `./` (система и скоупы) | Реестр скоупов, текущий скоуп и префикс, загрузка/сохранение, отложенный доступ к свойству по имени | `propertiessystem.h/.cpp` (статический `PropertiesSystem`, `PropertiesSystemScopeGuard`, `PropertiesScopeName = Name`), `propertiesscope.h/.cpp` (`PropertiesScope`), `propertypromise.h/.cpp` (`PropertyPromiseBase`, `PropertyPromise<T>`, `PropertyPtr`, `IntPropertyPtr`, `BoolPropertyPtr`, `StringPropertyPtr` … ; `ColorPropertyPtr` `[if QT_GUI_LIB]`), `propertiesvalidators.h/.cpp` (`PropertiesValidators::OddValidator<T>`), `propertyanimation.h/.cpp` (`PropertyAnimation : QVariantAnimation`, Q_OBJECT) |
| `./` (локальные свойства) | Значение с валидатором и подпиской без глобального реестра: числа с min/max и флагами, enum, указатели, контейнеры, даты, optional, коммутаторы And/Or, мост в `Property` | `localproperty.h` (2034 строки: `LocalProperty<T>`, `LocalPropertyValidatorBuilder`, `LocalPropertyDescInitializationParams`, `LocalPropertyLimitedDecimal<T>` (`LocalPropertyInt/UInt/Int64/Double/Float`), `LocalPropertySequentialEnum<Enum>`, `LocalPropertyPtr`, `LocalPropertySharedPtr` (+ `LocalPropertySharedPtrDispatcherHelper`), `LocalPropertySet`, `LocalPropertyHash`, `LocalPropertyVector`, `LocalPropertyBool`, `LocalPropertyColor/String/Locale/Name`, `LocalPropertyDate/Time/DateTime`, `LocalPropertyOptional` (`…DoubleOptional/IntOptional/StringOptional/DateOptional`), `LocalPropertyPreviousValue`, `LocalPropertyIntCommutator`, `LocalPropertyBoolCommutator`, `LocalPropertiesConnectBoth`, `PropertyFromLocalProperty`, `namespace adapters`; макрос `DECLARE_LOCAL_PROPERTY_TYPE()` и строка `class propertyName : public baseType` — через него, вероятно, объявлены `LocalPropertyFilePath` и `LocalPropertySetFromVariant`, которых нет ни в одном индексе и которые grep по `class/struct/using/typedef` не находит (?)), `localproperty.cpp`, `localpropertydeclarations.h` (`TranslatedStringPtr`) |
| `./` (состояние и вычисления) | Флаг готовности, цепочки параметров с ошибками/захватом, фоновые калькуляторы, неизменяемые данные и двойной буфер | `stateproperty.h/.cpp` (1535 строк: `StateProperty : LocalPropertyBool`, `StatePropertyBoolCommutator`, `StateParameters` (`ChainOptions`, `ChainConnectionOptions`), `make_chained`, `StateParametersProxy`, `StateParametersGroup`, `StateParametersContainerGroup`, `IStateParameterBase`/`StateParameterBase`, `StateParameterProperty<T>`, `StateParameterImmutableData<T>`, `StateParametersContainer<T>`, `StateCalculatorSwitcher`, `StateCalculator<T> : ThreadCalculator<T>`, `StateParametersChainData`, `StateImmutableData<T>`, `StateParametersImmutableData`, `StateDoubleBufferData<T>`, `StateUpdateObjectResult`, `DispatcherConnectionChain`, `StatePropertyCreate`; макросы `SPCO`, `SPCCO`, `IMPLEMENT_DEFAULT_IMMUTABLE_DATA()`) |
| `./` (ошибки) | Набор активных ошибок по `Name`, агрегация из дочерних моделей, описания и важность для UI | `localpropertyerrorscontainer.h/.cpp` (`LocalPropertyErrorsModel : LocalPropertyHash<Name, QSet<…>>`, `LocalPropertyErrorsViewModel`, `LocalPropertyErrorsViewModelDescription`; макросы `DECLARE/IMPLEMENT/COMBINE/OVERRIDE_GLOBAL_ERROR()`) |
| `./` (сериализация) | Специализации `Serializer<…>` / `SerializerXml<…>` и `TextConverter<…>` для свойств | `propertiesserializer.h` (586 строк: `LocalProperty<T>`, `StateProperty`, `StateParameterProperty`, `LocalPropertyFilePath`, `LocalPropertySequentialEnum`, `LocalPropertyLimitedDecimal`, `LocalPropertySet`, `LocalPropertySharedPtr`, `LocalPropertyVector`, `LocalPropertyDate/DateTime`, `LocalPropertyOptional`; `LocalPropertyPalette` — `[if SHARED_LIB_ADD_UI]`; макрос `SERIALIZER_XML_DECLARE_PROPERTY_TYPE()`), `textconverters.h` (`LocalProperty`, `LocalPropertyOptional`, `LocalPropertyLimitedDecimal`) |
| `./` (настройки, переводы, логи, редактирование) | Синглтон настроек приложения, enum ↔ строка, переводимые строки, логгер, ротация файлов, Save/Discard-редактирование | `sharedsettings.h/.cpp` (`SharedSettings : Singletone<SharedSettings>`; группы `NetworkSettings`, `StyleSettings` `[if SHARED_LIB_ADD_UI]` (`InstallQSSReader`), `PathSettings`, `MetricsSettings`, `SaveLoadSettings`, `LanguageSettings` (`LocaleType`), `ControlSettings`), `translatormanager.h/.cpp` (`TranslatorManager`, `TranslatedString : LocalPropertyString`; макросы `TM_TS()`, `TRANSLATED_PTR()`, `TRS()`, `TR_ENUM()`), `propertieslogger.h/.cpp` (`PropertiesLogger : Logger`), `propertiesfilenamesgenerator.h/.cpp` (`PropertiesFileNamesGenerator : FileNamesGeneratorWithGuard`), `editingobject.h/.cpp` (`EditingObject` с `DirtyMode` Save/Discard/Cancel, `EditingObjectComponentBase`, `TEditingObjectComponent<T>`, `CompareXmlObject`) |
| `Models/` (10 файлов) | Обёртки данных для Qt-моделей: таблица, дерево, словарь; цепочечные калькуляторы над `StateParameters`; ошибки строк модели | `wrappers.h` (782 строки: `ModelsWrapperBase` (`ConnectModel/DisconnectModel` `[if SHARED_LIB_ADD_UI]`), `ModelsTreeWrapper`, `ModelsTableWrapper`, `TModelsTableWrapper<Container>`, `ModelsTableView`, `StateChainedContainerCalculator`, `StateChainedValueCalculator`, `StateChainedComplexCalculator`; макросы `DECLARE_MODEL*()`), `modelstree.h/.cpp` (`ModelsTree : ModelsTreeWrapper`), `modelstreeitembase.h/.cpp` (`ModelsTreeItemBase`, `ModelsStandardTreeItem`, `ModelsTreeBuilder`), `modelsvacabulary.h/.cpp` (`ModelsVocabulary : TModelsTableWrapper<QVector<QHash<Name, QVariant>>>`; `CreateListModel` → `TViewModelsListBase<ModelsVocabulary>*` `[if WIDGETS_MODULE_LIB]`), `modelserrorscomponents.h/.cpp` (`ModelsErrorComponent`, `LocalPropertyErrorsModelWithErrorComponentMapper`), `modelsserializer.h` |
| `Ui/` (31 файл; подключается из `internal.hpp` `[if SHARED_LIB_ADD_UI]`) | Qt-модели и виды глобальных свойств, делегаты и их фабрика, связывание `Property`/`LocalProperty` с виджетами, палитра, диалог и окно свойств | `propertiesmodel.h/.cpp` (`PropertiesModel : QAbstractItemModel`, Q_OBJECT), `propertiestablemodel.*` (`PropertiesTableModel`), `propertiesview.*` (`PropertiesView : QTreeView, PropertiesViewBase`), `propertiestableview.*`, `propertiesviewbase.*`, `propertiesdelegate.*` (`PropertiesDelegate : QStyledItemDelegate`), `propertiesdelegatefactory.*` (`PropertiesDelegateFactory::SetFactory/Instance`), `propertiesstyleddelegatelistener.*`, `propertieswidgetsconnector.h/.cpp` (`PropertiesConnectorsContainer`, `PropertiesConnectorBase` и коннекторы CheckBox/ComboBox/LineEdit/SpinBox/DoubleSpinBox/TextEdit/GroupBox/RadioButtonsGroupBox), `localpropertieswidgetconnector.h/.cpp` (`LocalPropertiesWidgetConnectorsContainer`, `LocalPropertiesWidgetConnectorBase : QObject` и коннекторы PushButton/MenuLabel/MenuActions/Label/Slider/CheckBox/ComboBox/LineEdit/SpinBox/DoubleSpinBox/TextEdit/DateTime), `localpropertypalette.h/.cpp` (`LocalPropertyPalette`, `LocalPropertyPaletteBuilder`, `LocalPropertyPaletteObject`), `propertiesdialogbase.*`/`propertiesdialog.*`, `propertieswindow.*` (`PropertiesWindow : QMainWindow`; `.cpp` подключает `ui_propertieswindow.h`, файла `.ui` в индексе нет (?)), `styleutils.*` (`namespace WidgetProperties`), `internal.hpp` |
| `Claude outputs/` (4 файла) | Примеры `StateParameters`/`StateCalculator` и `StateChainedValueCalculator` на подсчёте простых чисел; похоже, не библиотечный код (?) | `prime_counter_stateparams.h/.cpp` (`PrimeCounterInput/Output : StateParameters`, `PrimeCounter`, `PrimeCounterUsageExample()`), `prime_counter_chained.h/.cpp`. При `includeAll(true)` в qmake, вероятно, попадают в сборку библиотеки (?) |

### Зависимости

**От каких папок DevLib зависит**
- **SharedModule** — безусловно, link-зависимость (`LINKS SharedModule` [сборка]):
  - `<SharedModule/internal.hpp>` подключают `property.h`, `propertiessystem.h`, `propertiesscope.h`, `propertieslogger.h`, `propertiesfilenamesgenerator.h`, `Models/modelstree.h`, `Models/modelstreeitembase.h`, `Ui/propertieswindow.h`, `Ui/localpropertieswidgetconnector.h` (он же — `<SharedModule/External/external.hpp>`); точечно: `declarations.h` (`localpropertydeclarations.h`, `Ui/styleutils.h`), `exception.h` (`stateproperty.h`), `stack.h` и `Threads/threadeventshelper.h` (`propertiessystem.cpp`) [индекс].
  - Типы из `uses:` → объявление в индексе SharedModule [grep]: `Singletone`, `FTranslationHandler`, `FMeasurementGetter`, `EnumHelper` → `declarations.h`; `Comparator`; `StringBuilder`; `Flags`, `FlagsHelpers`, `LongFlagsHelpers`; `DelayedCallObject`; `DispatcherConnectionSafePtr`; `StackPointers : Stack<T*>`; `SmartPointerWatchers`; `Logger : Singletone<Logger>`; `FileNamesGeneratorWithGuard`; `TextConverterContext`, `SerializerXmlObject`, `SerializerXmlWriteBuffer`, `SerializerXmlReadBuffer`; `ThreadCalculator`, `ThreadCalculatorData`, `ThreadsBase`; `AsyncResult = Promise`. `ScopedPointer` в индексах не найден, по grep — `SharedModule/smartpointersadapters.h` [grep исходников]. `WithDispatcherConnectionsSafe` не найден ни в индексах, ни grep-ом по `class/struct/using/typedef` (?).
- **SharedGuiModule** — частично условно [индекс]: `property.h` → `<SharedGuiModule/decl.h>` безусловно и `<SharedGuiModule/internal.hpp>` `[if QT_GUI_LIB]`; `externalproperty.cpp` → `<SharedGuiModule/internal.hpp>`; `Ui/propertiesview.cpp` → `"SharedGuiModule/decl.h"`. Используются `Vector3F : QVector3D` и `RectI = QRect`, оба в индексе SharedGuiModule объявлены `[if QT_GUI_LIB]` [grep]; у PropertiesModule они нужны только в `[if QT_GUI_LIB]`-классах (`Vector3FProperty`, `ExternalVector3FProperty`, `RectProperty`). В `LINKS` CMake SharedGuiModule не указан [сборка] — как он линкуется (?).
- **WidgetsModule** — в основном условно, `[if WIDGETS_MODULE_LIB]` (макрос задаётся в `WidgetsModule/WidgetsModule.pri` [grep]); только из `.cpp` [индекс]:
  - условно: `<WidgetsModule/internal.hpp>` в `sharedsettings.cpp` и `Models/modelsvacabulary.cpp`; `<WidgetsModule/Utils/widgethelpers.h>` и `<WidgetsModule/Managers/widgetsdialogsmanager.h>` в `Ui/propertiesdelegatefactory.cpp`; `ModelsVocabulary::CreateListModel` (`TViewModelsListBase<T> : TViewModelsTableBase<T>`);
  - **безусловно**: `<WidgetsModule/internal.hpp>` в `Ui/localpropertieswidgetconnector.cpp` и `Ui/propertiesview.cpp`. Сами эти файлы подключаются в сборку целиком, а `Ui/internal.hpp` — `[if SHARED_LIB_ADD_UI]`; так что по факту UI-слой без WidgetsModule не собирается (?).
  - Типы из `uses:` → индекс WidgetsModule [grep]: `WidgetsDialogsManager : SingletoneGlobal` → `Managers/widgetsdialogsmanager.h`; `WidgetColorDialogWrapper`, `WidgetAbstractButtonWrapper`/`WidgetPushButtonWrapper`, `MenuWrapper`, `ImageWrapper`, `LocalPropertyDoubleDisplay : LocalPropertyDouble` → `Utils/widgethelpers.h`; а также `WidgetsColorPicker`, `ElidedLabel`, `WidgetsDateTimeEdit`, `WidgetsComboBoxLayout`, `WidgetsDateTimeLayout`, `WidgetsDoubleSpinBoxLayout`, `TViewModelsListBase`.
  - Заголовки PropertiesModule WidgetsModule не подключают (в `.h` — только forward-объявления), а WidgetsModule подключает PropertiesModule в 50 файлах → цикл WidgetsModule ↔ PropertiesModule на уровне `.cpp`; безусловный для `Ui/localpropertieswidgetconnector.cpp` и `Ui/propertiesview.cpp`, остальное — при `WIDGETS_MODULE_LIB` [индекс].
- **UnitsModule** — условно, `[if UNITS_MODULE_LIB]` (макрос — `UnitsModule/UnitsModule.pri` [grep]): `<UnitsModule/internal.hpp>` только в `Models/modelsvacabulary.cpp` [индекс]. Тип `Measurement` (в `uses:` у `localproperty.h` и `modelsvacabulary.cpp`) — `class Measurement` в индексе UnitsModule; в `localproperty.h` include на UnitsModule нет, значит там он только forward-объявлен (через `FMeasurementGetter = TCachedExpression<const class Measurement*>` из SharedModule) (?) [grep]. Цикл UnitsModule ↔ PropertiesModule — условный, `UNITS_MODULE_LIB`.
- **VisualizationModule** — `<VisualizationModule/internal.hpp>` `[if VISUALIZATION_MODULE_LIB]` в `sharedsettings.cpp` [индекс]; такой папки в DevLib нет (`ls`), макрос нигде в `.pri`/CMake DevLib не задаётся [grep] — ветка в DevLib неактивна (?).
- **Внутри модуля**: `internal.hpp` → `Ui/internal.hpp` и `propertiesserializer.h` → `Ui/localpropertypalette.h` — оба `[if SHARED_LIB_ADD_UI]` [индекс]. QtWidgets в CMake подключён безусловно (`PRE_BUILD_LINKS Qt5::Core Qt5::Widgets`) [сборка].
- Типы, объявленные внутри модуля во вложенных областях (не внешние): `ImageConnectorParams`, `SubmitType` (`Ui/localpropertieswidgetconnector.h`), `PropertyChangeGuard` (`Ui/propertieswidgetsconnector.h`), `StdHandle` (`Ui/propertiesdialogbase.h`) [grep исходников].

**Кто зависит от PropertiesModule** (по `inc:` индексов других папок)
- Уже описаны в CODEMAP.md:
  - **SharedModule**: `builders.h` → `<PropertiesModule/internal.hpp>` `[if PROPERTIES_LIB]` [индекс]; `PROPERTIES_LIB` задаётся в `PropertiesModule.pri` (`DEFINES += PROPERTIES_LIB`) [сборка]. Условный цикл SharedModule ↔ PropertiesModule разобран в разделе SharedModule.
  - **WidgetsModule**: `<PropertiesModule/internal.hpp>` — 46 файлов, `<PropertiesModule/Ui/internal.hpp>` — 4 файла [индекс]; наследники: `ModelsStandardListModel : TModelsTableWrapper<…>`, `NotifyTreeNode`/`NotifyTreeFolderNode : ModelsTreeItemBase`, `LocalPropertyDoubleDisplay : LocalPropertyDouble` (см. раздел WidgetsModule).
  - **GraphicsToolsModule**: `internal.hpp` в `gtcamera.cpp`, `gtrenderercontroller.h`, `Objects/gtobjectbase.h` [индекс].
  - **CloudModule**: `cloudservice.h` → `<PropertiesModule/internal.hpp>` [индекс].
  - **ComputeGraphModule**: `"PropertiesModule/property.h"` в `computenodebase.h` и 9 заголовках узлов, `propertiesvalidators.h` в `computenodegaussenblur.cpp`/`computenodemedianblur.cpp`, `propertiessystem.h` в `computegraphcore.cpp` [индекс].
  - **SQLiteModule**: `sqlitedatabase.h` → `<PropertiesModule/internal.hpp>` (`TModelsTableWrapper`, `StateProperty`) [индекс].
- Раздела в CODEMAP нет: **UnitsModule** — `measurementunitmanager.h` → `<PropertiesModule/internal.hpp>` (безусловно) [индекс].

### Точки входа и сборочные цели

- Сборка [сборка]: `CMakeLists.txt` — `library(TARGET PropertiesModule PRE_BUILD_LINKS Qt5::Core Qt5::Widgets LINKS SharedModule)`; `PropertiesModule.pri` — `includeAll(true)`, `DEFINES += PROPERTIES_LIB`. SharedGuiModule, WidgetsModule, UnitsModule в `LINKS` не указаны (?).
- Единый заголовок: `#include <PropertiesModule/internal.hpp>`; UI — `<PropertiesModule/Ui/internal.hpp>` (в `internal.hpp` — `[if SHARED_LIB_ADD_UI]`) [индекс].
- `main` нет, это библиотека. Рантайм-входы [индекс]: `PropertiesSystem::Begin/End/BeginPrefix/Load/Save/Store/Restore`; синглтоны `SharedSettings` (`CreateGlobalProperties`, `Release`, `PathSettings::Initialize(productName)`), `TranslatorManager::GetInstance()`, `PropertiesDelegateFactory::Instance()/SetFactory()`; `PropertiesDialogBase::RejectAllDialogs()`/`Initialize()`; `PropertiesLogger`; `PropertiesWindow::LoadDefault()`.
- Условные ветки сборки: `QT_GUI_LIB` (цвет, `Vector3F`, `RectI`), `SHARED_LIB_ADD_UI` (весь `Ui/`, `StyleSettings`, `ConnectModel`, сериализация палитры), `WIDGETS_MODULE_LIB`, `UNITS_MODULE_LIB`, `VISUALIZATION_MODULE_LIB` [индекс].

### Где искать

| Задача | Файлы |
|---|---|
| Реактивное поле класса: значение, подписка, валидатор | `localproperty.h` (`LocalProperty`, `LocalPropertyValidatorBuilder`), `localproperty.cpp` |
| Число с min/max и флагами, enum, optional, указатель, контейнер, дата/время | `localproperty.h` (`LocalPropertyLimitedDecimal`, `LocalPropertySequentialEnum`, `LocalPropertyOptional`, `LocalPropertyPtr/SharedPtr`, `LocalPropertySet/Hash/Vector`, `LocalPropertyDate/Time/DateTime`) |
| Связать свойства в обе стороны, коммутатор bool/int, предыдущее значение | `localproperty.h` (`LocalPropertiesConnectBoth`, `ConnectFrom`, `Local*CommutatorBase`, `LocalPropertyPreviousValue`), `stateproperty.h` (`StatePropertyBoolCommutator`) |
| Глобальное свойство по имени, скоупы, сохранение | `property.h/.cpp`, `propertiessystem.h/.cpp`, `propertiesscope.h/.cpp`, `propertypromise.h/.cpp` |
| Свойство поверх внешней переменной или другого свойства | `externalproperty.h/.cpp` |
| Опубликовать `LocalProperty` как глобальный `Property` | `localproperty.h` (`PropertyFromLocalProperty`), `sharedsettings.h` (`CreateGlobalProperties`) |
| Флаг готовности, ожидание нескольких условий | `stateproperty.h/.cpp` (`StateProperty`, `StatePropertyCreate`) |
| Цепочка параметров → фоновый калькулятор | `stateproperty.h/.cpp` (`StateParameters`, `StateParametersContainer`, `StateCalculator`, `StateImmutableData`, `StateDoubleBufferData`), `Models/wrappers.h` (`StateChained*Calculator`), примеры `Claude outputs/prime_counter_*` |
| Ошибки валидации и их показ | `localpropertyerrorscontainer.h/.cpp`, `Models/modelserrorscomponents.h/.cpp` |
| Сериализация свойств (binary/XML/текст) | `propertiesserializer.h`, `textconverters.h`, `Models/modelsserializer.h` |
| Enum ↔ строка, переводимые строки | `translatormanager.h/.cpp` |
| Настройки приложения: пути, язык и форматы, стиль QSS, сеть | `sharedsettings.h/.cpp` |
| Редактирование с Save/Discard, сравнение через XML | `editingobject.h/.cpp` |
| Таблица/дерево данных для `QAbstractItemModel` | `Models/wrappers.h`, `Models/modelstree.*`, `Models/modelstreeitembase.*`, `Models/modelsvacabulary.*` |
| Привязать `LocalProperty` к виджету | `Ui/localpropertieswidgetconnector.h/.cpp` |
| Привязать глобальный `Property` к виджету | `Ui/propertieswidgetsconnector.h/.cpp` |
| Дерево/таблица свойств, делегаты, свой редактор | `Ui/propertiesmodel.*`, `Ui/propertiesview.*`, `Ui/propertiestablemodel.*`, `Ui/propertiestableview.*`, `Ui/propertiesdelegate.*`, `Ui/propertiesdelegatefactory.*` |
| Диалог или окно свойств | `Ui/propertiesdialogbase.*`, `Ui/propertiesdialog.*`, `Ui/propertieswindow.*` |
| Палитра параметров объектов | `Ui/localpropertypalette.h/.cpp` |
| Логи и имена файлов | `propertieslogger.h/.cpp`, `propertiesfilenamesgenerator.h/.cpp` |
| Анимация значения свойства | `propertyanimation.h/.cpp` |
| Условная компиляция (`QT_GUI_LIB`, `SHARED_LIB_ADD_UI`, `WIDGETS_MODULE_LIB`, `UNITS_MODULE_LIB`) | `internal.hpp`, `property.h`, `externalproperty.h`, `sharedsettings.*`, `propertiesserializer.h`, `Models/modelsvacabulary.*`, `Ui/propertiesdelegatefactory.cpp` |
| Сборка и подключение | `CMakeLists.txt`, `PropertiesModule.pri`, `internal.hpp`, `Ui/internal.hpp` |

## GraphicsToolsModule

> Источники: индекс `.codemap/GraphicsToolsModule/` (`00_overview.md`, `index_01.md`; `codemap: v4` = `python3 codemap.py --version`; 72 файла, 9 278 строк: 35 `.cpp`, 35 `.h`, 1 `.hpp`, 1 `.pri`). Типы из строк `uses:` сверены grep-ом по индексам `.codemap/SharedModule/`, `.codemap/SharedGuiModule/`, `.codemap/PropertiesModule/`, `.codemap/ResourcesModule/`, `.codemap/ControllersModule/` (все `codemap: v4`); типы, которых нет в индексах, найдены grep-ом по исходникам `*.h` [grep src]. Подключения `.pri` и `DEFINES` — grep по `*.pro`/`*.pri` [grep pro]. Пометки: [индекс] — индекс GraphicsToolsModule, [grep X] — найдено grep-ом в индексе папки X, [grep src] — в исходниках, [дамп] — `.codemap/_dump/dump_01.md`. «(?)» — предположение. В текущем дампе исходников GraphicsToolsModule нет (только `Sandbox/gtwindow.cpp`), поэтому сведения о поведении из прежнего дампа помечены «[дамп, прежний] (?)» — по свежим данным они не перепроверены.

### Назначение

Движок 3D/2D-рендеринга DevLib на OpenGL (`OpenGLFunctions = QOpenGLExtraFunctions` [grep SharedGuiModule, `[if QT_GUI_LIB && !QT_NO_OPENGL]`]) поверх Qt5. Рендерер работает в своём потоке (`GtRenderer : GtRendererBase : ThreadComputingBase` [индекс, grep SharedModule]).
В модуле: камера (перспектива, изометрия, project/unproject), сцена с очередями drawable, render path, меши (в т. ч. OBJ/MTL), текстуры (в т. ч. DDS), FBO, тени, шейдеры, материалы с uniform-параметрами из ресурсов, текст из bitmap-шрифтов `.fnt`; кадр OpenCV как текстура — только при `OPENCV`.
Для UI: виджет `GtView` и `GtRendererController` (ввод, кадр, анимация камеры). Публичный API подключается через `internal.hpp` [индекс].

### Подмодули

| Путь | Назначение | Ключевые классы / файлы |
|---|---|---|
| `./` декларации | Алиасы указателей, контекст контроллеров | `decl.h`: `GtShaderProgramPtr`, `GtMeshPtr`, `GtMeshBufferPtr`, `GtRendererPtr`, `GtRendererControllerPtr`, `GtFontPtr`, `GtDrawableBasePtr` + `GtDrawableDeleter` + `make_shared(GtDrawableBase*)`, `GtRenderPathPtr`, `GtRenderProperties = QHash<Name, QVariant>`, `GtControllersContext`, `namespace GtNames`; `internal.hpp` (подключает всё, кроме `gtframetexture.h`, `gtdepthbuffer.h`, `gtshadowmaptechnique.h`, мешей circle/grid/surface и части `Objects/gtmaterialparameter*`) [индекс] |
| `./` рендерер и поток | Рендер в своём потоке (offscreen-контекст); реестр шейдеров, шрифтов, текстур, FBO, мешей, shading-материалов и ресурсов; создание drawable по очередям | `gtrendererbase.h/.cpp` (`GtRendererBase : ThreadComputingBase`, чистые `onInitialize/onDraw/onDestroy`, `initializationMutex`; в `.cpp` — `QOffscreenSurface`, `QOpenGLContext`), `gtrenderer.h/.cpp` (`GtRenderer : GtRendererBase, protected OpenGLFunctions` [Q_OBJECT]: `CreateShaderProgram(Alias)`, `LoadFont`, `CreateFontAlias`, `CreateTexture/CreateTexture3D`, `CreateFrameBuffer`, `CreateResource<T>/GetResource<T>`, `RegisterMaterialMesh`, `RegisterShadingMaterial(s)`, `CreateDrawable(Queued)<T>`, `AddController`, `CreateDefaultController`, `CreateSharedRenderer`, `SetDefaultQueueNumber`, `MoveToThread`, `GetThreadHandler(NoCheck)`; `GtRendererSharedData`) [индекс] |
| `./` контроллер и вид | Связка рендерера с виджетом: сцена, render path, свойства рендера, ввод, картинка кадра, анимация камеры | `gtrenderercontroller.h/.cpp` (`GtRendererController : QObject` [Q_OBJECT], сигнал `imageUpdated`, `GetCurrentImage`, `MapToScreen`, `SetRenderPath`, `SetRenderProperty(ies)`, `SetVisibilityMask`, `*Event`; `GtCameraAnimationEngine` (на `QVariantAnimation`/`QParallelAnimationGroup`); шаблоны `GtCameraPointsPathEngineBase`, `GtCameraPointsPathEngine`, `GtCameraPointsPathFixedDistanceEngine`), `gtview.h/.cpp` (`GtView : QWidget`, `GtViewParams`, `SetController`), `gtviewcontext.h` (`GtViewContext`: рендерер + сцена или контроллер, доступ к шрифтам/шейдерам/материалам, `CreateDrawable(Queued)`) [индекс] |
| `./` камера и управление | Матрицы view/projection/world, project/unproject, фокус, изометрия; мышь и клавиатура; чтение глубины | `gtcamera.h/.cpp` (`GtCameraState` + флаги `State_*`, `GtCameraFocus`, `GtCamera`, `InvertRotationFlags`, `InstallObserver(path)`, `GtCamera::CameraObserverProperties`, `GtCameraStateSaver`), `gtplayercontrollercamera.h/.cpp` (`GtPlayerControllerCamera : ControllerContextedBase<GtControllersContext>`), `gtdepthbuffer.h/.cpp` (`GtDepthBuffer::ValueAt`, `SetFrameBuffer`) [индекс] |
| `./` сцена и render path | Хранение drawable по очередям, порядок отрисовки, custom render stages | `gtscene.h/.cpp` (`GtScene`: `AddDrawable`, `RemoveDrawable`, `DrawAll`, `Draw(queue)`, `DrawFilter`, `(Pre)DrawFilterCustomRenderStage`, `DrawDepth`, `GetDestroyed`), `gtrenderpath.h/.cpp` (`GtRenderPath` абстрактный: `Initialize`, `Render`, `Resize`; `GtDefaultRenderPath`) [индекс] |
| `./` меши | VBO/VAO, меши, загрузка OBJ/MTL, стандартные меши | `gtmeshbase.h/.cpp` (`GtMeshBufferBuilder`, `GtMeshBuffer` + `VertexType`, `GtMesh`, `GtMeshIndices`, `GtStandardMeshs::GetQuadMesh2D`), `gtmeshloader.h/.cpp` (`GtMeshLoader::LoadObj/LoadMaterials`, `Parser`, `Mesh`, `Material`), `gtmeshquad2D.h` (`GtMeshBufferQuad2D`), `gtmeshcircle2D.h` (`Circle2D`, `GtMeshCircle2D`), `gtmeshgrid.h`, `gtmeshsurface.h` [индекс] |
| `./` текстуры, FBO, тени | Текстуры 2D/3D/multisample, framebuffer-объекты, shadow map, кадр OpenCV как текстура | `gttexture2D.h/.cpp` (`GtTextureFormat`, `GtTexture`, `GtTexture2D` (`LoadImg`, `Load` — DDS через `DDS/nv_dds.h`), `GtTexture3D`, `GtTexture2DMultisampled`, `GtTextureBinder`), `gtframebufferobject.h/.cpp` (`GtFramebufferTextureFormat`, `GtFramebufferFormat`, `GtFramebufferObjectBase`, `GtFramebufferObject`, `GtFramebufferObjectMultisampled`, `GtFramebufferObjectBinderScopeGuard`, `ToImage`), `gtshadowmaptechnique.h/.cpp` (`GtShadowMapTechnique`), `gtframetexture.h/.cpp` (`GtFrameTexture`, вход `cv::Mat` — весь класс `[if OPENCV]`) [индекс] |
| `./` готовые drawable | Текст, линии, эллипсы | `gttextdrawable.h/.cpp` (`GtTextDrawable` 3D, `GtTextScreenDrawable`, `GtFont`, `GtTextMap::LoadFromFnt`, `GtTextGlyphBase`, `GtTextDrawableSettings::ConnectFrom`, `GtTextAlign`/`GtTextAligns`), `gtlinesdrawable.h/.cpp` (`GtLinesDrawable`), `gtellipselinedrawable.h/.cpp` (`GtEllipseLineDrawable`) [индекс] |
| `Objects/` | База объектов, шейдерные программы, материалы и их параметры (uniform из ресурсов по имени) | `gtobjectbase.h/.cpp` (`GtObjectBase::MapProperties(QtObserver*)`; `GtDrawableBase(const GtViewContext&)`: `Destroy()/Update()` → `AsyncResult`, `AboutToDestroy`, `MoveToThread` для `LocalProperty<T>` и `CommonDispatcher<...>`, переопределяемые `draw`, `drawDepth`, `preDraw`, `onInitialize`, `onDestroy`, `onAboutToDestroy`, `isCustomRenderStageAccepted`; `GtDrawableDeleter::operator()`), `gtshaderprogram.h/.cpp` (`GtShaderProgram` + `ShaderType`, `GtSharedShaderManager`: `Initialize`, `Extract`, `Merge`), `gtmaterial.h/.cpp` (`GtMaterial : GtObjectBase`: `AddParameter`, `AddMesh`, `ApplyShadingMaterial`, `Draw(Instanced)`), `gtmaterialparameterbase.h/.cpp` (`GtMaterialParameterBase`, `GtMaterialResourceParameterBase`, `GtMaterialParameterConst`, `GtMaterialParameterBasePtr`; алиасы `Vector2FResource`, `GtVector2FArrayResource`, `Vector3FResource`, `Matrix3Resource`, `Matrix4Resource`, `GtTextureResource`, `GtFrameTextureResource`, `GtShadowMapTechniqueResource`, `GtFramebufferObjectResource`, `GtMaterialMeshResource` = `TResource<...>`; `<functional>` `[if WORK]`), параметры: `…matrix` (`GtMaterialParameterMatrix`, `…Matrix3`), `…vector3f` (`…Vector2F`, `…Vector3F`, `…Vector2FArray`), `…texturebase`, `…texture`, `…shadow`, `…frametexture` (`GtMaterialParameterFrameTexture` — `[if OPENCV]`) [индекс] |
| `DDS/` | Загрузчик DDS (код NVIDIA): DXTC1/3/5, RGB, RGBA, cubemap, volume | `nv_dds.h/.cpp` (`nv_dds::DDSImage`, `DDSTexture`, `DDSSurface`, `TextureType`); `UploadTexture*` условные: `UploadTexture2D`/`UploadTextureCubemap` `[if !NV_DDS_NO_GL_SUPPORT]`, `UploadTexture1D` `[if … && !GL_ES_VERSION_2_0 && !GL_ES_VERSION_3_0]`, `UploadTexture3D` `[if … && !GL_ES_VERSION_2_0]`; `defines:` `GL_RGB`, `GL_RGBA`, `GL_LUMINANCE`, `GL_BGR_EXT`, `GL_BGRA_EXT`, `GL_COMPRESSED_*_S3TC_DXT*_EXT` [индекс] |

### Зависимости

От других папок DevLib (по `inc:` и `uses:`):

- **SharedModule** — inc `internal.hpp`, `flags.h`, `array.h`, `stack.h`, `External/external.hpp` [индекс]. Типы [grep SharedModule]: `ThreadComputingBase : QThread, ThreadEventsContainer` — база `GtRendererBase`; `ThreadHandler`, `ThreadHandlerNoThreadCheck` (= `std::function<AsyncResult(const FAction&)>`), `AsyncResult = Promise` — `GtRenderer`, `GtDrawableBase`, `GtRendererController::MapToScreen`; `ThreadsBase` — `gtrenderer.cpp`; `QtObserver : QObject` — `MapProperties`; `Dispatcher = CommonDispatcher<>`, `CommonDispatcher<...>`, `DispatcherConnections`, `DispatcherConnectionsSafe`, `DelayedCallObject`, `Interruptor` — `GtRendererController`, `GtTextDrawableSettings`, `GtDrawableBase`, `GtEllipseLineDrawable`; `guards::LambdaGuard` — `GtRenderer::SetDefaultQueueNumber`; `Name`, `SharedPointer`/`SP`, `ScopedPointer`, `ArrayPointers`, `StackPointers`, `FAction` — повсюду. `Stack` (`SharedModule/stack.h` [grep src]) — `gtframebufferobject.h`. Макрос `DECL_FLAGS` из `flags.h` объявляет локальные `GtCameraState::State`, `GtCamera::InvertRotationFlags` (`gtcamera.h`) и `GtTextAligns` (`gttextdrawable.h`) [grep src].
- **SharedGuiModule** — inc `decl.h`, `internal.hpp` [индекс]. Типы из `SharedGuiModule/decl.h` [grep SharedGuiModule]; все они там условные `[if QT_GUI_LIB]` (`OpenGLFunctions` — `[if QT_GUI_LIB && !QT_NO_OPENGL]`), т. е. GraphicsToolsModule требует сборки с QtGui: `OpenGLFunctions`, `Matrix4 = QMatrix4x4`, `Matrix3`, `Quaternion`, `Point2I`, `Point2F`, `Vector2F`, `Vector3F : QVector3D`, `Point3F = Vector3F`, `Point4F`, `Vector4F`, `Color3F`, `SizeI = QSize`, `SizeF`, `BoundingBox` (одноимённый `struct BoundingBox` есть и в SharedModule [grep SharedModule]; используется вариант из SharedGuiModule (?)), GL-алиасы `gTexID`, `gTexUnit`, `gTexTarget`, `gTexInternalFormat`, `gFboID`, `gRenderbufferID`, `gRenderType`, `gLocID`, вершинные структуры (`ColoredVertex2F`, `ColoredVertex3F`, `TexturedVertex2F`, `TexturedVertex3F`, `Vertex3f3f`, `Vertex3f3f3f`, `Vertex3f2f2f`, `StatedVertex3F`) для `GtMeshBuffer::UpdateVertexArray`.
- **PropertiesModule** — inc `internal.hpp` в `gtcamera.cpp`, `gtrenderercontroller.h`, `Objects/gtobjectbase.h` [индекс]. Типы [grep PropertiesModule]: `LocalProperty<T>`, `LocalPropertyBool`, `LocalPropertyColor = LocalProperty<QColor>`, `LocalPropertyFloat`/`LocalPropertyInt` (= `LocalPropertyLimitedDecimal<…>`), `LocalPropertyVector<T>` — свойства `GtLinesDrawable`, `GtEllipseLineDrawable`, `GtTextDrawable*`, `GtRendererController`, `GtDrawableBase::MoveToThread`; `ExternalVector3FProperty` (`externalproperty.h`, `[if QT_GUI_LIB]`) — `GtCamera::CameraObserverProperties`; `SharedSettings : Singletone<SharedSettings>` — `gtcamera.cpp`, `gtplayercontrollercamera.cpp`. Ранее по дампу: `CameraObserverProperties` держит три `ExternalVector3FProperty` по путям `path+"/eye"`, `/forward`, `/up` (`eye` редактируемое) [дамп, прежний] (?).
- **ResourcesModule** — inc `internal.hpp` в `gtrenderer.h`, `gttexture2D.h`, `Objects/gtmaterialparameterbase.h`; `resourcessystem.h` в `Objects/gtmaterialparameter{matrix,vector3f,texture,shadow}.cpp` [индекс]. Типы [grep ResourcesModule]: `TResource<T> : Resource` — основа алиасов `*Resource` из `Objects/gtmaterialparameterbase.h` и `GtRenderer::GetResource<T>`; `ResourcesSystem` (`RegisterResourceAndGet<T>`, `GetResource<T>`) — поле `GtRenderer` (`uses:` `gtrenderer.h`) и `gtmaterialparameterframetexture.cpp`. `FDelegate` — локальный typedef в `GtMaterialParameterBase` (`std::function<void(QOpenGLShaderProgram*, gLocID, OpenGLFunctions*)>` [grep src]). Ранее по дампу: у каждого `GtRenderer` своя `ResourcesSystem`, у `GtRendererSharedData` — общая; параметры материалов читают матрицы/вектора камеры как ресурсы по имени [дамп, прежний] (?).
- **ControllersModule** — inc `controllerbase.h` в `gtplayercontrollercamera.h`, `internal.hpp` в `gtrenderercontroller.cpp` [индекс]. Типы [grep ControllersModule]: `ControllerBase : QObject`, `ControllerContextedBase<T> : ControllerBase` — база `GtPlayerControllerCamera`; `ControllersContainer : QObject` — параметр конструкторов `GtRendererController` и `GtPlayerControllerCamera`. Ранее по дампу: `GtRendererController` заполняет `GtControllersContext` и пересылает события ввода в `ControllersContainer` в потоке рендера [дамп, прежний] (?).
- Условные и внешние: OpenCV — `<opencv2/opencv.hpp>` и весь `GtFrameTexture` / `GtMaterialParameterFrameTexture` только `[if OPENCV]` [индекс]; `OPENCV` задаёт, например, `Sandbox/Sandbox.pro` (`DEFINES += … OPENCV …`) [grep pro]. Qt `gui-private` (`GraphicsToolsModule.pri`, `<private/qhighdpiscaling_p.h>` в `gtview.cpp`, `gtrenderercontroller.cpp`); `<GL/gl.h>` / `<OpenGL/gl.h>` в `DDS/nv_dds.h` `[if !NV_DDS_NO_GL_SUPPORT && (!)__APPLE__]` [индекс].
- Циклов между папками нет: ни один индекс SharedModule, SharedGuiModule, PropertiesModule, ResourcesModule, ControllersModule не упоминает `GraphicsToolsModule` [grep].

Кто зависит от GraphicsToolsModule: из папок, описанных в CODEMAP.md, — никто; ни один индекс других папок `.codemap/*` не содержит `GraphicsToolsModule` (`GtComputeGraph` в ComputeGraphModule — только совпадение префикса, его `inc:` не ссылаются на GraphicsToolsModule) [grep]. Вне описанных папок `GraphicsToolsModule.pri` подключают проекты `Sandbox`, `GameEngine`, `FlexFluidRefactoring`, `OpenGLMultithreadingTest` [grep pro]; `Sandbox/gtwindow.cpp` включает `<GraphicsToolsModule/internal.hpp>` [дамп].

### Точки входа и сборочные цели

- Сборка: `GraphicsToolsModule.pri` содержит только `QT *= gui-private` [индекс]; своей цели нет, `.pri` включается в `.pro` потребителей (см. выше) [grep pro]. Как туда попадают исходники модуля — в индексе не видно (?). `CMakeLists.txt` нет [индекс].
- Подключение: `#include <GraphicsToolsModule/internal.hpp>` [индекс]; `gtframetexture.h`, `gtdepthbuffer.h`, `gtshadowmaptechnique.h` и меши circle/grid/surface подключаются отдельно.
- Типовой запуск (по сигнатурам) [индекс]: `GtRenderer(defaultShadersPath)` → `CreateDefaultController()` или `GtRendererController(renderer, controllersContainer, context)` → `GtView::SetController(...)`. Объекты создаются через `CreateDrawable<T>(...)` / `CreateDrawableQueued<T>(queue, ...)` у `GtRenderer`, `GtRendererController` или `GtViewContext`. Ранее по дампу новый `GtRendererController` ставит камере `SetProjectionProperties(45, 1, 100000)`, render path по умолчанию — `GtDefaultRenderPath` [дамп, прежний] (?). Связь `defaultShadersPath` с `GtSharedShaderManager::Initialize` не видна (?).

### Где искать

| Задача | Файлы |
|---|---|
| Камера: перемещение, вращение, изометрия, project/unproject | `gtcamera.h/.cpp` |
| Управление камерой мышью и клавиатурой | `gtplayercontrollercamera.h/.cpp`; путь события: `GtView::*Event` → `GtRendererController::*Event` (`gtrenderercontroller.cpp`) → `ControllersContainer` |
| Камера как свойства (eye/forward/up) | `gtcamera.cpp` (`CameraObserverProperties`, `InstallObserver`) |
| Ресурсы рендерера (uniform-данные по имени) | `gtrenderer.h` (`CreateResource`, `GetResource`, `GtRendererSharedData`), `Objects/gtmaterialparameterbase.h` (алиасы `*Resource`), `ResourcesModule/resourcessystem.h` |
| Анимированный перелёт камеры, обход по точкам | `gtrenderercontroller.h` (`GtCameraAnimationEngine`, `GtCameraPointsPath*Engine`) |
| Встроить 3D-вид в окно | `gtview.h/.cpp`, `gtrenderercontroller.h/.cpp`, `gtviewcontext.h` |
| Поток рендера, инициализация GL-контекста | `gtrendererbase.h/.cpp`, `gtrenderer.cpp` (`onInitialize`, `onDraw`, `onDestroy`) |
| Регистрация шейдеров, шрифтов, текстур, FBO, мешей, shading-материалов | `gtrenderer.h/.cpp` |
| Свой drawable-объект | `Objects/gtobjectbase.h/.cpp` (`GtDrawableBase`), примеры `gtlinesdrawable.*`, `gtellipselinedrawable.*` |
| Порядок отрисовки, очереди, маска видимости, render stages | `gtscene.h/.cpp`, `gtrenderpath.h/.cpp`, `GtRendererController::SetVisibilityMask` |
| Шейдеры и общие include шейдеров | `Objects/gtshaderprogram.h/.cpp` (`GtSharedShaderManager::Merge`) |
| Материалы и uniform-параметры | `Objects/gtmaterial.*`, `Objects/gtmaterialparameter*.h/.cpp` |
| Вершинные буферы, свой формат вершин | `gtmeshbase.h/.cpp` (`GtMeshBufferBuilder`, `GtMeshBuffer::VertexType_Custom`) |
| Загрузка OBJ/MTL | `gtmeshloader.h/.cpp` |
| Текстуры, DDS | `gttexture2D.h/.cpp`, `DDS/nv_dds.*` |
| Offscreen-рендер, снимок кадра, буфер глубины | `gtframebufferobject.*` (`ToImage`), `gtdepthbuffer.*`, `GtRendererController::GetCurrentImage` |
| Тени | `gtshadowmaptechnique.*`, `Objects/gtmaterialparametershadow.*` |
| Текст в 3D и на экране, шрифты `.fnt` | `gttextdrawable.h/.cpp` |
| Кадр OpenCV как текстура (только с `OPENCV`) | `gtframetexture.*`, `Objects/gtmaterialparameterframetexture.*` |

## WidgetsModule

> Источники: индекс `.codemap/WidgetsModule/` (`00_overview.md`, `index_01.md`, `index_02.md`; `codemap: v4` совпадает с `python3 codemap.py --version`; 167 файлов, 25 632 строки: 81 `.cpp`, 83 `.h`, 2 `.hpp`, 1 `.pri`). Типы из строк `uses:` сверены grep-ом по индексам `.codemap/SharedModule/`, `PropertiesModule/`, `UnitsModule/` и др.; не найденные в индексах — grep по исходникам. Метки: [индекс] — индекс WidgetsModule, [индекс X] — индекс папки X, [grep] — grep по исходникам/ФС, [дамп] — `.codemap/_dump`. «(?)» — предположение.

### Назначение

Библиотека Qt-виджетов DevLib (QtWidgets): свои табличные и древовидные виды, модели представления (`ViewModels*`/`Models*`), делегаты, диалоги, уведомления, прогресс-бары, подсказки, редакторы даты/времени, составные виджеты «подпись + редактор» [индекс].
Связь с данными — через свойства PropertiesModule: обёртки `WidgetWrapper` и наследники в `Utils/widgethelpers.h` привязывают `LocalProperty*` к виджетам [индекс]. Глобальные менеджеры-синглтоны: действия, диалоги, иконки, фокус, состояние заголовков таблиц, уведомления [индекс].
Агрегатор `internal.hpp` подключает весь публичный API модуля [индекс]. Классы с `namespace Ui` используют формы `.ui` (`ui_*.h`), сами `.ui` в индекс не входят [индекс].

### Подмодули

| Путь | Назначение | Ключевые классы / файлы |
|---|---|---|
| `./` | Агрегатор, общие декларации и параметры, загрузка QSS | `internal.hpp`; `widgetsdeclarations.h` (enum `GroupKeyboardSeparator`, `DecimalKeyboardSeparator`, `DateTimeDisplayFormatEnum`; `DescWidgetsLocationAttachmentParams`; `[if WIDGETS_MODULE_LIB]`: `WidgetsDialogsManagerButtonStruct`, `WidgetsDialogsManagerDefaultButtons`, `DescCustomDialogParams`, `DescShowDialogParams`, `DescTableViewParams`; алиасы `ViewModelsCategoriesContainer`, `ModelsStandardListModelContainer`, `WidgetsGlobalTableActionsScopeHandlersPtr`; `namespace WidgetProperties`); `qtqssreader.h` (`QtQSSReader`: `Install`, `ReadAll`, `SetEnableObserver`) |
| `Actions/` | Реестр `QAction` по скоупам; глобальные действия таблиц (копировать/вставить/удалить) | `ActionsManager : SingletoneGlobal` (`Initialize`, `CreateActionsFromRegisteredScopes`, `FindScope`, `AddTo`, `FindAction`), `ActionsScopeBase`/`ActionsScope`, `WidgetsGlobalTableActionsScope : ActionsScopeBase, Singletone` (`EditHandlers`, `AddDefaultHandlers`, `CreateDefaultDeleteHandler`, `CreateDefaultPasteHandler`, `ClipboardRows`; enum `EnableIfMode`), `WidgetsGlobalTableActionsScopeHandler(s)`, `WidgetsGlobalTableActionId : Latin1Name`; макросы `DECLARE_GLOBAL_ACTION`/`IMPLEMENT_GLOBAL_ACTION` |
| `Attachments/` | Поведения, навешиваемые на существующие виджеты | `WidgetTableViewColumnsAttachment`, `WidgetsMatchingAttachment`, `WidgetsAbstractViewRowAttachment`, `WidgetsActiveTableViewAttachment` (`GetInstance`, `Attach`), `WidgetsLocationAttachment`, `WidgetsMenuLocationAttachment`, `WindowResizeAttachment` (enum `Location`) |
| `Bars/` | Заголовок безрамочного окна и тулбар | `MenuBarMovePane : QFrame` (минимизация/максимизация/закрытие, WinAPI `[if Q_OS_WIN]`), `WidgetsToolBar : QFrame` (`CreateDrawerButton`, `CreateButton`) |
| `Components/` | Размещение компонента относительно цели с отложенным пересчётом | `ComponentPlacer` (`DelayedCallObjectParams`) |
| `Delegates/` | Делегаты редакторов ячеек | `delegates.h`: `QStyledItemDelegateBase`, `DelegatesCombobox`, `DelegatesComboboxCustomViewModel`, `DelegatesIntSpinBox`, `DelegatesDoubleSpinBox`, `DelegatesColor`, `DelegatesLineEdit`, `DelegatesDateTime`/`DelegatesDate`/`DelegatesDateTimePicker`/`DelegatesTimePicker`/`DelegatesMonth`, `DelegatesCheckBox`; `Delegates::AdjustDialogEditorToCell` |
| `Dialogs/` | Базовый диалог и типовые содержимые | `WidgetsDialog : QDialog`, `WidgetsInputDialogView` (`AddInt/AddDouble/AddLineText/AddDate/AddMonth/AddDateTime/AddDateRange/AddDateTimeRange`, `AddMeasurement` `[if UNITS_MODULE_LIB]`; алиас `WidgetsInputDialogConnectors` = `MeasurementWidgetConnectors` `[if UNITS_MODULE_LIB]`, иначе `LocalPropertiesWidgetConnectorsContainer`), `WidgetsHyperlinkedListDialogView`, `WidgetsDebugJsonDialog` |
| `Layouts/` | Раскладка «потоком» | `FlowLayout : QLayout`, `FlowWidget : QScrollArea` |
| `Managers/` | Синглтоны: диалоги, фокус, словарные модели, состояние заголовков | `WidgetsDialogsManager : SingletoneGlobal` (`ShowTempDialog`, `ShowDeleteCancelDialog`, `ShowTextDialog`, `ShowHyperlinkedListDialog`, `GetText/GetInt/GetDate/GetMonth/GetDateTime/GetDateRange/GetColor`, `SelectDirectory`, `SetDefaultParentWindow`, `MakeFrameless`, `AttachShadow`), `FocusManager`, `ModelsVocabularyManager` (`RegisterModel`, `CreateViewModel`, `GetViewModel`), `WidgetsStandardTableHeaderManager : SingletoneGlobal` (`Register`, save/restore состояния) |
| `Models/` | Модели представления: таблицы, списки, деревья; колонки, декораторы, фильтры, поиск | `modelstablebase.h` (1072 стр.: `ModelsAbstractItemModel`, `ViewModelsTableBase`, `TViewModelsTableBase<T>`, `TViewModelsEditTable`, `ViewModelsTableColumnComponents`, `TViewModelsDataInitializerComponent`, `TViewModelsDragAndDropComponent`, `TViewModelsDecoratedTable`, `ModelsTableSearchComponent`, `ViewModelsVerticalCompoundTable`, `ModelsIconsContext`), `viewmodelsdefaultcomponents.h` (970 стр.: `TViewModelsColumnComponentsStandardBuilder`, `TViewModelsColorFormatingRule`, макросы `ViewModelsBuilderColumn*`), `modelsfiltermodelbase.h` (`ViewModelsFilterModelBase : QSortFilterProxyModel`, `ViewModelsTableFilterComponent`, `ViewModelsTreeFilterComponent`, `ViewModelsSelectionFilterModel`), `modelslistbase.h` (`TViewModelsListBase`, `ModelsStandardListModel`, `ViewModelsStandardListModel`), `viewmodeltreebase.h` (`ViewModelsTreeBase`, `TViewModelTreeBase`), `viewmodelsstandard.h` (`ViewModelsStandard`, `ModelsStandardRow`), `viewmodelvacabulary.h` (`ModelsVocabularyViewModel`), `viewmodelsdefaultfieldmodel.h` |
| `Notifier/` | Всплывающие уведомления, консоль сообщений, верхняя панель ошибок | `NotifyManager : QObject` (enum `MessageType {Info, Warning, Error}`, `Notify`, `GetInstance`), `NotifyWidget`, `NotifyButton`, `NotifyConsole` (в `.cpp`: `NotifyConsoleViewModel : TViewModelTreeBase<ModelsTree>`, `NotifyTreeNode`/`NotifyTreeFolderNode : ModelsTreeItemBase`), `TopNotifierFrame`/`NotifierFrame`, `TopNotifierFrameErrorsComponent`, `TopNotifierFrameErrorsFocusComponent`; `notifydeclarations.h` (`NotifyData`, `NotifyConsoleData`, `NotifyConsoleDataWrapper = TModelsTableWrapper<…>`) |
| `ProgressBars/` | Индикаторы прогресса процессов | `MainProgressBar`, `ShadowProgressBar`, `ShadowProgressBarOneForAll`, `QRoundProgressBar`, `ProgressBar : QObject` |
| `TableViews/` (+ `Header/`) | Базовые таблица и дерево; заголовок с ресайзом и выбором колонок | `WidgetsTableViewBase : QTableView` (спаны, рисование), `WidgetsAdjustableTableView`, `WidgetsTreeViewBase : QTreeView`, `WidgetsResizableHeaderAttachment : QHeaderView`, `WidgetsResizableHeaderAttachmentShowColumnsWidget` |
| `ToolTip/` | Подсказки со стрелкой | `ToolTipWidget : QFrame`, `ToolTipArrowWidget` |
| `Utils/` | Обёртки виджетов со свойствами, fluent-построитель UI, SVG-иконки, стили | `widgethelpers.h/.cpp` (852 + 3084 стр.: `ObjectWrapper`, `WidgetWrapper` и `Widget*Wrapper` — LineEdit/TextEdit/CheckBox/Combobox/SpinBox/DoubleSpinBox/TableView/HeaderView/Splitter/Dialog/ColorDialog/Groupbox/TabBarLayout/Label/ElidedLabel/ScrollArea/ProgressBar, `MenuWrapper`, `ActionWrapper`, `ViewModelWrapper`, `ImageWrapper`, `WidgetsObserver`, `EventFilterObject`, `LocalPropertyDoubleDisplay`, enum `HighLightEnum`), `widgetbuilder.h` (`WidgetBuilder`), `iconsmanager.h` (`IconsManager`, `IconsSvgIcon : QIcon`, `IconsPalette`), `widgetsstylesettings.h` (`WidgetsStyleSettings : QWidget`), `widgetstyleutils.h` (`namespace StyleUtils`) |
| `Widgets/` | Отдельные виджеты | `ElidedLabel`, `WidgetsSpinBoxWithCustomDisplay`/`WidgetsDoubleSpinBoxWithCustomDisplay`, `WidgetsColorPicker`, `WidgetsPicturePicker : QLabel`, `WidgetsImportView`, `WidgetsEditingObjectSaveCancelButtons`, `propertiestoolview.h` (`PropertiesToolView`, `PropertiesToolFolderView`, `WidgetPropertiesToolViewWrapper`, `TPropertiesToolWrapper`) |
| `Widgets/DateTime/` | Редакторы и попапы даты, времени, месяца | `WidgetsDateTimeEdit : QDateTimeEdit`, `WidgetsDateTimeWidget`, `WidgetsDatetimePopupPicker`, `WidgetsTimePicker`, `WidgetsTimeWidget`, `WidgetsCalendarWidget`, `WidgetsMonthPicker`, `WidgetsMonthPopupPicker` |
| `Widgets/Layouts/` | Составные «подпись + редактор» | `WidgetsLayoutComponent`, `WidgetsButtonLayout`, `WidgetsComboBoxLayout`, `WidgetsDateTimeLayout`, `WidgetsDoubleSpinBoxLayout`, `WidgetsSpinBoxLayout`, `WidgetsGroupBoxLayout`, `WidgetsHeaderLayout`, `WidgetsLineEditLayout`, `WidgetsSwitchLayout`, `WidgetsTabBarLayout`, `WidgetsTextEditLayout`; агрегатор `internal.hpp` |

### Зависимости

От каких папок DevLib зависит WidgetsModule:

- **SharedModule** (безусловно). `<SharedModule/internal.hpp>` — 18 включений, `<SharedModule/External/external.hpp>` — `qtqssreader.cpp`, `ProgressBars/shadowprogressbar.h`, `TableViews/widgetsadjustabletableview.h`; `<SharedModule/hacks.h>` — `TableViews/Header/widgetsresizableheaderattachment.cpp` [индекс]. Типы из `uses:`, найденные в индексе SharedModule [индекс SharedModule]: `Name`, `Latin1Name`, `FTranslationHandler`, `FAction`, `SharedPointer`, `Singletone`, `SingletoneGlobal` (база `ActionsManager`, `WidgetsDialogsManager`, `WidgetsStandardTableHeaderManager`), `Stack`, `Array`, `ContainerBuilder`, `StringBuilder`, `Dispatcher`/`CommonDispatcher`/`DispatcherConnectionsSafe`, `DispatchersCommutator`, `DelayedCallObject`/`DelayedCallObjectParams`, `ThreadsBase`, `ProcessFactory`/`ProcessValue`/`DescProcessValueState` (прогресс-бары), `ImportExportSource`/`DescImportExportSourceParams` (`WidgetsDialogsManager`), `SerializerXmlVersion`/`SerializerVersion`/`DescSerializationXMLWriteParams`. Не в индексах, найдены grep-ом по исходникам: `ScopedPointer` — `SharedModule/smartpointersadapters.h`; `QuadTreeF` — пространство имён через `#define QUADTREE_NAMESPACE QuadTreeF` в `SharedModule/Trees/quadtreef_decl.h` [grep]. `Optional` (в `uses:` `viewmodelsdefaultcomponents.h`) не найден ни в индексах, ни grep-ом по объявлениям (?).
- **PropertiesModule** (безусловно). `<PropertiesModule/internal.hpp>` — 46 включений, `<PropertiesModule/Ui/internal.hpp>` — `Widgets/widgetspicturepicker.h`, `Widgets/DateTime/widgetsdatetimepopuppicker.cpp`, `Widgets/Layouts/widgetsdoublespinboxlayout.cpp`, `Widgets/Layouts/widgetsgroupboxlayout.cpp` [индекс]. Типы [индекс PropertiesModule]: `LocalProperty` и семейство `LocalPropertyBool/Int/Double/String/Color/Date/DateTime/SequentialEnum/Optional/DoubleOptional`, `StateProperty`, `LocalPropertyBoolCommutator`, `TranslatedString`/`TranslatedStringPtr`, `TranslatorManager`, `LanguageSettings`, `SharedSettings`, `LocalPropertyErrorsViewModel`, `LocalPropertiesWidgetConnectorsContainer`, `LocalPropertiesLabelConnector`, `EditingObject` (`WidgetsEditingObjectSaveCancelButtons`), `ModelsTree`/`ModelsTreeItemBase`/`ModelsTreeWrapperPtr`, `TModelsTableWrapper`, `ModelsVocabulary`/`ModelsVocabularyPtr`. Наследование от классов PropertiesModule: `ModelsStandardListModel : TModelsTableWrapper<…>`, `ModelsVocabularyViewModel` (на `TViewModelsTableBase<ModelsVocabulary>`), `NotifyTreeNode`/`NotifyTreeFolderNode : ModelsTreeItemBase`, `LocalPropertyDoubleDisplay : LocalPropertyDouble` [индекс + индекс PropertiesModule].
- **UnitsModule** — в основном условно. `[if UNITS_MODULE_LIB]`: `<UnitsModule/internal.hpp>` в `Dialogs/widgetsinputdialogview.h`, `Models/modelstablebase.cpp`, `Models/viewmodelsdefaultcomponents.cpp`, `Models/viewmodelvacabulary.cpp`, `TableViews/Header/widgetsresizableheaderattachmentshowcolumnswidget.cpp`, `Widgets/propertiestoolview.h`; `<UnitsModule/measurementunitmanager.h>` в `Models/viewmodelsstandard.cpp`; условные методы `WidgetsInputDialogView::AddMeasurement`, `MenuWrapper::AddMeasurementAction`, `PropertiesToolView::AddDoubleProperty`, `ViewModelsTableBase::AttachDependence/AttachTempDependence` (перегрузки) [индекс]. **Без пометки `[if]`** `<UnitsModule/internal.hpp>` подключают `Managers/modelsvocabularymanager.cpp` и `Utils/widgethelpers.cpp` [индекс]. Типы [индекс UnitsModule]: `Measurement`, `MeasurementManager`, `MeasurementWidgetConnectors`.
- **VisualizationModule** — условно, `[if VISUALIZATION_MODULE_LIB]`: `<VisualizationModule/internal.hpp>` в `Utils/widgetsstylesettings.cpp` [индекс]; папки `VisualizationModule/` в DevLib нет [grep: ls] — внешний модуль (?).
- SharedGuiModule, GraphicsToolsModule, ResourcesModule, ControllersModule и прочие: ни include, ни типов из их индексов в WidgetsModule нет [grep по индексам].

Кто зависит от WidgetsModule (из уже описанных в CODEMAP.md):

- **PropertiesModule**, только из `.cpp` [индекс PropertiesModule]: условно `[if WIDGETS_MODULE_LIB]` — `<WidgetsModule/internal.hpp>` в `sharedsettings.cpp` и `Models/modelsvacabulary.cpp`, `<WidgetsModule/Utils/widgethelpers.h>` и `<WidgetsModule/Managers/widgetsdialogsmanager.h>` в `Ui/propertiesdelegatefactory.cpp` (`uses: WidgetColorDialogWrapper`); безусловно — `<WidgetsModule/internal.hpp>` в `Ui/localpropertieswidgetconnector.cpp` и `Ui/propertiesview.cpp`. Итог — цикл PropertiesModule ↔ WidgetsModule на уровне `.cpp` (частично условный по `WIDGETS_MODULE_LIB`).
- **SharedModule**: `ImportExport/importexport.h` использует в телах шаблонов `WidgetsImportView`, `DescCustomDialogParams`, `WidgetsDialogsManager`, `WidgetsDialogsManagerDefaultButtons`, `WidgetProperties` без include WidgetsModule [индекс SharedModule, дамп — см. раздел SharedModule].
- **StylesModule**: данные QSS рассчитаны на `QtQSSReader` [раздел StylesModule, индекс WidgetsModule].
- GraphicsToolsModule, CloudModule, CommandsModule, ComputeGraphModule, NetworkModule, ProgressModule, SQLiteModule: include `WidgetsModule/` в их индексах нет [grep по индексам].
- Вне описанных: `UnitsModule/Ui/measurementvaluewithunitconnectorscontainer.cpp` безусловно подключает `<WidgetsModule/internal.hpp>` [индекс UnitsModule] — цикл UnitsModule ↔ WidgetsModule (со стороны WidgetsModule безусловны только `modelsvocabularymanager.cpp` и `widgethelpers.cpp`).

### Точки входа и сборочные цели

- `WidgetsModule/internal.hpp` — единый публичный заголовок, подключает все подмодули (в т. ч. `Widgets/Layouts/internal.hpp` и `qtqssreader.h`) [индекс].
- `WidgetsModule/WidgetsModule.pri`: в индексе показана только строка `LIBS *= user32.lib` [индекс]; полностью файл — `includeAll(true)`, `DEFINES += WIDGETS_MODULE_LIB`, `win32 { LIBS *= user32.lib }` [grep: cat]. Макрос `WIDGETS_MODULE_LIB` включает в `widgetsdeclarations.h` структуры диалогов/таблиц и условные include в PropertiesModule. `CMakeLists.txt` в индексе нет [индекс].
- Инициализация во время работы: `ActionsManager::Initialize()`/`CreateActionsFromRegisteredScopes()`, `WidgetsDialogsManager::SetDefaultParentWindow()`, `QtQSSReader::Install(mainQSSFile)`, `IconsManager::RegisterIcon*`, `WidgetsObserver::EnableAutoCollapsibleGroupboxes` [индекс; порядок вызова в приложении (?)].

### Где искать

| Задача | Файлы |
|---|---|
| Привязать `LocalProperty` к виджету, обернуть виджет | `Utils/widgethelpers.h/.cpp` (`Widget*Wrapper`) |
| Собрать форму кодом | `Utils/widgetbuilder.h/.cpp` |
| Показать диалог (подтверждение, ввод текста/числа/даты/цвета, выбор папки, свой диалог) | `Managers/widgetsdialogsmanager.h/.cpp`, `widgetsdeclarations.h` (`DescCustomDialogParams`, кнопки), `Dialogs/widgetsdialog.*`, `Dialogs/widgetsinputdialogview.*` |
| Табличная модель, колонки, редактирование, drag&drop | `Models/modelstablebase.h/.cpp`, `Models/viewmodelsdefaultcomponents.h`, `Models/viewmodelsstandard.*` |
| Фильтрация и поиск в таблице | `Models/modelsfiltermodelbase.h/.cpp`, `ModelsTableSearchComponent` в `modelstablebase.h` |
| Дерево | `Models/viewmodeltreebase.*`, `TableViews/widgetstreeviewbase.*` |
| Настроить таблицу (колонки, видимость, состояние заголовка) | `widgetsdeclarations.h` (`DescTableViewParams`), `Managers/widgetsstandardtableheadermanager.*`, `TableViews/Header/*`, `Attachments/tableviewwidgetattachment.*` |
| Копировать/вставить/удалить строки таблицы (горячие клавиши) | `Actions/widgetsglobaltableactionsscope.*`, `Attachments/widgetsactivetableattachment.*`, `Attachments/widgetsabstractviewrowattachment.*` |
| Редактор ячейки (делегат) | `Delegates/delegates.h/.cpp` |
| Уведомления и консоль ошибок | `Notifier/notifymanager.*`, `Notifier/notifyconsole.*`, `Notifier/topnotifierframe.*` |
| Прогресс долгой операции | `ProgressBars/*` |
| Иконки SVG и их перекраска | `Utils/iconsmanager.*` |
| Стили, QSS, цвета темы | `qtqssreader.*`, `Utils/widgetsstylesettings.*`, `Utils/widgetstyleutils.*` |
| Поля даты, времени, месяца | `Widgets/DateTime/*`, `Delegates/delegates.h` (`DelegatesDateTime*`, `DelegatesMonth`) |
| Поле «подпись + редактор» | `Widgets/Layouts/widgets*layout.*` |
| Позиционирование попапа/подсказки относительно виджета | `Attachments/widgetslocationattachment.*`, `Attachments/widgetsmenulocationattachment.*`, `Components/componentplacer.*`, `ToolTip/*` |
| Безрамочное окно: перемещение и ресайз | `Bars/menubarmovepane.*`, `Attachments/windowresizeattachment.*` |
| Импорт данных в таблицу | `Widgets/widgetsimportview.*` |
| Панель свойств объекта | `Widgets/propertiestoolview.h/.cpp` |
| Фокус | `Managers/widgetsfocusmanager.*` |

## CloudModule

> Источники: индекс `.codemap/CloudModule/` (`00_overview.md`, `index_01.md`; `codemap: v4`, совпадает с `python3 codemap.py --version`; 4 файла, 273 строки: 1 `.cpp`, 1 `.h`, 1 `.hpp`, 1 `.pri`). Типы из строк `uses:` сверены grep-ом по индексам `.codemap/SharedModule/` и `.codemap/PropertiesModule/` (у обоих `codemap: v4`). То, чего нет в индексах, найдено grep-ом по исходникам. Текущий дамп `.codemap/_dump/dump_01.md` файлов CloudModule не содержит. Метки: [индекс] — индекс CloudModule, [индекс X] — индекс папки X, [grep] — grep по исходникам или ФС, [дамп, пред.] — дамп из прошлой версии раздела, заново не сверялся. «(?)» — предположение.

### Назначение

Асинхронный HTTP-клиент к REST-сервису на Qt Network. Класс `CloudService` возвращает `AsyncRequest` из двух методов: `Request(params)` для произвольного запроса и `PostFile(params, filePath)` для загрузки файла (multipart). Перед использованием сервис запускают через `Initialize()`, после — останавливают через `Terminate()` [индекс].
Настройки сервиса лежат в `CloudServiceParams`, указатель на них — `CloudServiceParamsPtr = SharedPointer<CloudServiceParams>`. Отдельный запрос описывает fluent-структура `CloudServiceRequestParams`: метод, маршрут, тело, заголовки и колбэки `FOnRequest`/`FOnMultiPart`/`FOnReplied`/`FOnError` [индекс].
Защищённые виртуальные `onInitialize`/`onTerminate` позволяют сделать конкретный сервис наследником (?).

### Подмодули

| Путь | Назначение | Ключевые классы / файлы |
|---|---|---|
| `./` | Весь модуль, подпапок нет [индекс] | `cloudservice.h/.cpp` [индекс]:<br>• `CloudServiceParams` (только конструктор в индексе);<br>• `CloudServiceRequestParams`: конструкторы `(onReplied)` и `(onReplied, data)`; сеттеры `SetRawHeaders`, `SetPostVerb`, `SetVerb`, `SetRoute`, `SetOnError`, `SetOnRequest`, `SetOnMultiPart`; алиасы `FOnRequest(QNetworkRequest&)`, `FOnMultiPart(QHttpMultiPart&)`, `FOnReplied(const AsyncRequest&, QNetworkReply&)`, `FOnError(const QString&)`;<br>• `CloudService`: `Initialize`, `Terminate`, `Request`, `PostFile` (оба `const`, возвращают `AsyncRequest`), `# onInitialize`, `# onTerminate`; в `.cpp` есть ещё `processReply`.<br>`internal.hpp` — агрегатор, подключает только `cloudservice.h` [индекс]. `CloudModule.pri` [индекс] |

### Как это работает

Ниже — разбор кода из прошлой версии раздела [дамп, пред.]. Индекс с тех пор не изменился: те же 4 файла и 273 строки, те же методы. Но код заново не сверялся.
- **Потоки.** Конструктор создаёт `QNetworkAccessManager` (`m_connection : ScopedPointer`) и переносит его в `m_thread : QThread`. `Initialize()` вызывает `onInitialize()` и запускает поток. `Terminate()` вызывает `onTerminate()`, затем `m_thread.terminate()` и `wait()`. Если поток не запущен, `Request`/`PostFile` сразу резолвят `Result` в `false`.
- **`Request`.** Запрос собирается в потоке вызывающего: `QNetworkRequest(QUrl(Params->Url + Route))`, затем заголовок `ContentType = Params->DefaultContentType`, `RawHeaders` и вызов `OnRequest`. Отправка идёт в потоке менеджера через `ThreadsBase::DoQThreadWorkerWithResult` (`sendCustomRequest(request, Verb, Payload)`), после неё — `processReply`. Индекс подтверждает, что `cloudservice.cpp` использует `ThreadsBase` [индекс].
- **`PostFile`.** URL и `OnRequest` такие же, как у `Request`, но `DefaultContentType`, `RawHeaders`, `Verb` и `Payload` не применяются, метод всегда POST. Тело — `QHttpMultiPart(FormDataType)` с частью `name="file"`, после него вызывается `OnMultiPart`. В заголовке у `SetOnMultiPart` есть пометка «TODO. Works only with post files».
- **`processReply`.** Обрабатывает `QNetworkReply::finished`. При ошибке вызывает `OnError(errorString + ": " + readAll())`, иначе `OnReplied(result, *reply)`. Затем резолвит `Result` и вызывает `reply->deleteLater()`. Колбэки, видимо, выполняются в потоке `m_thread` (?).
- **Особенности.** Если запрос прерван через `ResultInterruptor` до отправки, `Result` не резолвится (?). `OnReplied` вызывается без проверки на `nullptr`. Авторизации нет: `Authorize` закомментирован, а `Routes::Login` объявлен, но не используется. `<QJsonDocument>`/`<QJsonObject>` (они есть в `inc:` `cloudservice.cpp` [индекс]) нужны только этому закомментированному коду.

### Зависимости

От каких папок DevLib зависит CloudModule:

- **PropertiesModule.** Это единственный include вне модуля: `cloudservice.h` подключает `<PropertiesModule/internal.hpp>` [индекс]. Ни одного типа PropertiesModule в строках `uses:` CloudModule нет [индекс; grep по индексу PropertiesModule]. Видимо, include нужен ради транзитивных SharedModule/SharedGuiModule (?). Цепочка: `PropertiesModule/internal.hpp` → `property.h` → `<SharedModule/internal.hpp>`, а также `<SharedGuiModule/decl.h>` и `<SharedGuiModule/internal.hpp>[if QT_GUI_LIB]` [индекс PropertiesModule].
- **SharedModule.** Напрямую модуль не подключается, но все внешние типы из `uses:` объявлены именно там:
  - `AsyncRequest` — `SharedModule/interruptor.h` [индекс SharedModule].
  - `ThreadsBase` (`static AsyncResult DoQThreadWorkerWithResult(location, QObject* threadObject, task, priority)`) — `SharedModule/Threads/threadsbase.h` [индекс SharedModule].
  - `SharedPointer`, `SharedPointerInitialized` — `SharedModule/smartpointersadapters.h` [индекс SharedModule].
  - `ScopedPointer` — в индексе как объявление не найден, только использования. По исходникам: `SharedModule/smartpointersadapters.h:8`, `class ScopedPointer : public std::unique_ptr<T, deleter>` [grep].
  - `FOnError`, `FOnReplied`, `FOnRequest` — собственные алиасы `CloudServiceRequestParams` [индекс].
- **NetworkModule** не используется: в CloudModule нет ни его include, ни его типов. Сеть работает напрямую через Qt Network (`QNetworkAccessManager`, `QNetworkReply`, `QNetworkRequest`, `QHttpMultiPart`) [индекс].
- **Условные зависимости.** В индексе CloudModule нет ни меток `[if X]`, ни строк `defines:` [индекс]. Условная часть приходит только транзитивно: SharedGuiModule подключается через PropertiesModule при `QT_GUI_LIB` [индекс PropertiesModule]. Циклов нет: от CloudModule не зависит никто (см. ниже).

Кто зависит от CloudModule: ни один индекс в `.codemap/` вне папки не упоминает `CloudModule`/`CloudService`, в том числе ни один из модулей, уже описанных в этой карте [grep]. В исходниках (`*.h/*.hpp/*.cpp`), `.pri`/`.pro`/`CMakeLists.txt` вне папки упоминаний тоже нет [grep]. Видимо, модуль используется только приложением вне DevLib (?).

### Точки входа и сборочные цели

- `CloudModule/internal.hpp` — публичный заголовок, подключает `cloudservice.h` [индекс].
- `CloudModule/CloudModule.pri` указан в индексе как build-файл, но его содержимое индекс не показывает. Сам файл состоит из одной строки `includeAll(true)` [grep: cat]. Функция `includeAll` определена через `defineTest(includeAll)` в корневом `Global.pri` [grep]. `QT += network` в `.pri` нет, откуда сборка получает Qt Network, неизвестно (?).
- `CMakeLists.txt` в папке нет, в ней только 4 файла [grep: ls].
- Жизненный цикл: создать `CloudService` или наследника → задать параметры (`CloudServiceParams`) → `Initialize()` → `Request`/`PostFile` → `Terminate()` [индекс; порядок — дамп, пред.].

### Где искать

| Задача | Файлы |
|---|---|
| Задать адрес сервиса и параметры по умолчанию | `cloudservice.h` (`CloudServiceParams`, `CloudServiceParamsPtr`) |
| Отправить HTTP-запрос (GET/POST/PUT/DELETE…) | `cloudservice.h` (`CloudServiceRequestParams`: конструктор с `data`, `SetVerb`/`SetPostVerb`, `SetRoute`, `SetRawHeaders`), `cloudservice.cpp` (`CloudService::Request`) |
| Загрузить файл (multipart/form-data) | `cloudservice.cpp` (`CloudService::PostFile`), `SetOnMultiPart`/`SetOnRequest` |
| Добавить заголовки или токен | `SetRawHeaders`, `SetOnRequest` в `cloudservice.h`. В `PostFile` `RawHeaders`, видимо, не применяются [дамп, пред.] |
| Обработать ответ или ошибку | колбэки `FOnReplied`/`FOnError`; `CloudService::processReply` в `cloudservice.cpp` |
| Отменить запрос или дождаться результата | `AsyncRequest` — `SharedModule/interruptor.h` |
| Поток сети, запуск и остановка | `CloudService::Initialize/Terminate` в `cloudservice.cpp`; `ThreadsBase::DoQThreadWorkerWithResult` в `SharedModule/Threads/threadsbase.h` |
| Написать свой сервис-наследник | `onInitialize`/`onTerminate` в `cloudservice.h` |

## CommandsModule

> Источники: индекс `.codemap/CommandsModule/` (codemap v4 совпадает с `codemap.py --version`; 3 файла, 15 строк), сами сборочные файлы `CommandsModule/CMakeLists.txt`, `CommandsModule/CommandsModule.pri` и `CommandsModule/internal.hpp` (только чтение), grep по `.codemap/` и по исходникам DevLib.

### Назначение

Заготовка модуля команд. Кода в папке нет: ни одного `.h`/`.cpp`, только `CMakeLists.txt`, пустой `CommandsModule.pri` (0 байт) и `internal.hpp` (9 строк) (индекс). `internal.hpp` подключает `"operations.h"`, но такого файла нет ни в DevLib (`find`), ни в одном индексе (grep `.codemap/`). Возможно, это остаток подсистемы команд и операций (например, undo/redo), которую вынесли или ещё не написали (?).

### Подмодули

| Путь | Назначение | Ключевые классы/файлы |
|---|---|---|
| `CommandsModule/` | Корень модуля, подпапок нет (индекс) | `internal.hpp`: общий include модуля, подключает отсутствующий `operations.h`; `CMakeLists.txt`; `CommandsModule.pri` (пустой) |

Классов и функций в индексе нет. Строк `uses:` и `defines:` у `internal.hpp` нет, поэтому внешних типов файл не использует (индекс). В самом файле видно, что `#include "operations.h"` стоит под `#ifdef __cplusplus`, а include guard называется `INTERNAL_H` (файл). Индекс v4 не помечает это условие как `[if __cplusplus]`.

### Зависимости

- **Сборка:** `CMakeLists.txt` объявляет `library(TARGET CommandsModule PRE_BUILD_LINKS Qt5::Core Qt5::Widgets LINKS SharedModule)` (файл сборки). Со SharedModule связь есть только на уровне линковки: в индексе нет ни include `<SharedModule/...>`, ни типов SharedModule (индекс).
- **По include:** только `"operations.h"`, причём только в C++-режиме (`__cplusplus`). Файла нет ни в индексах (grep `.codemap/`), ни в исходниках (`find . -name 'operations.h*'`). Поэтому код, который включит `internal.hpp`, скорее всего не соберётся (?).
- **Кто зависит от CommandsModule:** `ControllersModule/CMakeLists.txt:3` содержит `LINKS SharedModule CommandsModule` (grep). Но ни один исходник DevLib не включает `<CommandsModule/...>` (grep по `*.h/*.hpp/*.cpp/*.pri/CMakeLists.txt`), и ни в одном чужом индексе CommandsModule не упоминается (grep `.codemap/`). Из модулей, уже описанных в этой карте, от CommandsModule не зависит ни один (grep `CODEMAP.md` и `.codemap/`). Раздела ControllersModule в карте пока нет.

### Точки входа и сборочные цели

- Цель CMake `CommandsModule` (функция `library`; Qt5::Core, Qt5::Widgets, SharedModule) (файл сборки).
- qmake: `CommandsModule.pri` пустой, поэтому в qmake-сборку модуль ничего не добавляет (индекс, файл).
- Точек входа (классов, функций) нет (индекс).

### Где искать

| Задача | Файл |
|---|---|
| Подключить модуль команд | `CommandsModule/internal.hpp` (тянет за собой отсутствующий `operations.h`) |
| Поменять зависимости цели CommandsModule | `CommandsModule/CMakeLists.txt` |
| Подключить модуль в qmake-проект | `CommandsModule/CommandsModule.pri` (пустой) |
| Понять, кто линкуется с модулем | `ControllersModule/CMakeLists.txt` |
| Найти реализацию команд и операций | В DevLib её нет; вероятно, она вне DevLib, в wps-client (?) |

## ComputeGraphModule

> Источники: индекс `.codemap/ComputeGraphModule/` (codemap v4, 38 файлов, 1651 строка: 18 `.cpp`, 18 `.h`, 1 `.hpp`, 1 `.pri`); типы из `uses:` сверены по индексам `.codemap/SharedModule/`, `.codemap/PropertiesModule/`, `.codemap/SharedGuiModule/` (все v4) и grep по исходникам (помечено «grep»).

### Назначение

Граф обработки кадров (OpenCV `cv::Mat`): цепочка узлов `GtComputeNodeBase`, каждый берёт выход предыдущего узла (`SetInput`), пересчитывает себя (`onInputChanged`/`update`) и отдаёт `GetOutput()`.
Граф считается в отдельном потоке (`GtComputeGraph : ThreadComputingBase`, `idealFrameTime = 30`). Источник кадров — файл-запись (`InputFrameStream`) или датчик глубины OpenNI (`OpenniSensor`).
Предметная область — обработка карты глубины и распознавание (`ComputeNodeVolcanoRecognition`); что именно распознаётся — (?).

### Подмодули

Папка плоская (все 38 файлов в `./`, индекс).

| Путь | Назначение | Ключевые классы/файлы |
|---|---|---|
| `computegraphbase.*` | Поток, вычисляющий граф | `GtComputeGraph` [Q_OBJECT] (`AddCalculationGraph`, `compute`); хранит узлы в `StackPointers` |
| `computegraphcore.*` | Синглтон-сборщик графа: создаёт все узлы, задаёт корень | `ComputeGraphCore` (`Instance`, `Initialize(InputFrameStream*)`, `GetRootNode`, `GetNodes`); макросы `ADD_PROPERTY_NODEA()`/`ADD_PROPERTY_NODE()` генерируют геттеры `Get##NodeClass##AdditionalName()` |
| `computenodebase.*` | Базовый узел: имя, enabled, вход/выход, учёт памяти | `GtComputeNodeBase` (флаги `F_Default`, `F_NeedUpdate`; `IsSkippeable`, `totalMemoryUsage`; `BoolProperty` — вероятно, для enabled (?)) |
| `computenodethreadsafe.*` | Узел с выдачей выхода под мьютексом для другого потока | `GtComputeNodeThreadSafe` (`GetThreadOutput`), `MatGuard` |
| `computenode{blur,gaussenblur,medianblur}.*` | Фильтры размытия | `ComputeNodeBlur`, `ComputeNodeGaussenBlur`, `ComputeNodeMedianBlur` |
| `computenode{resize,minresize,crop}.*` | Геометрия кадра | `ComputeNodeResize`, `ComputeNodeMinResize`, `ComputeNodeCrop` (`validateRect`) |
| `computenode{holesfilter,blackhole,delay}.*` | Очистка карты глубины, задержка/маска движения | `ComputeNodeHolesFilter` (`horizontalInterpolation`), `ComputeNodeBlackHole`, `ComputeNodeDelay` (`SetMotionMask`) |
| `computenodevolcanorecognition.*` | Распознавание | `ComputeNodeVolcanoRecognition` (`SetParams(idealFramesPerSecond)`, `getIdealFramesPerSecond`) |
| `computenodedepthsensor.*`, `opennisensor.*` | Источник: датчик OpenNI | `ComputeNodeDepthSensor`, `OpenniSensor` (`Initialize`, `CreateOutput(openni::SensorType)`, `Start/Stop/Update`, `GetOutput(type)`) |
| `computenodedepthfakesensor.*`, `inputframestream.*` | Источник: запись из файла | `ComputeNodeDepthFakeSensor` (`SetInputStream`), `InputFrameStream` (`SetFileName`, `ReadFrame`, `Repeat`, `SetPause`, `MoveToThread`, `GetAvailableInputs`) |
| `internal.hpp` | Публичный агрегирующий заголовок | включает `inputframestream.h`, `computegraphcore.h`, `computenodeblur.h`, `computenodedelay.h` |
| `ComputeGraphModule.pri` | Сборка | `includeAll(true)`, `HEADERS += $$PWD/internal.hpp` (файл .pri) |

### Зависимости

От других папок DevLib (по `inc:` и `uses:`):

- **SharedModule** — `ThreadComputingBase` (`SharedModule/Threads/threadcomputingbase.h`, индекс) — база `GtComputeGraph`; `StackPointers` (`SharedModule/stack.h`, индекс) и `Stack` (`SharedModule/stack.h:94`, grep); `Array` (`SharedModule/array.h`, индекс); `Flags` (`SharedModule/flags.h`, индекс); `ScopedPointer` (`SharedModule/smartpointersadapters.h:8`, grep); также `profile_utils.h`, `timer.h`, `internal.hpp` (inc).
- **PropertiesModule** — `BoolProperty`, `IntProperty`, `UIntProperty`, `FloatProperty` (алиасы в `PropertiesModule/property.h`, индекс) — параметры узлов; `PropertiesSystem` (`propertiessystem.h`, индекс) — в `computegraphcore.cpp`; `PropertiesValidators` (`propertiesvalidators.h`, индекс) — в Gaussen/Median blur.
- **SharedGuiModule** — `decl.h` включён в `computenodebase.h` (inc); `Point2I` = `QPoint` объявлен в `SharedGuiModule/decl.h` только `[if QT_GUI_LIB]` (индекс) — используется в `computenodevolcanorecognition.cpp`, т.е. эта зависимость условная: без `QT_GUI_LIB` тип недоступен.
- Внешние: OpenCV (`<opencv2/opencv.hpp>`), OpenNI2 (`<OpenNI.h>`, `<OniEnums.h>`), Qt (`QThread`, `QMutex`, `QTimer`, `QFile`, `QDir`, `QDataStream`).

Кто зависит от неё: ни один из модулей, уже описанных в CODEMAP.md, не упоминает `ComputeGraphModule` (grep по `.codemap/*/index_*.md`). Подключает `Sandbox/Sandbox.pro:18` — `include($$CABOODLE/ComputeGraphModule/ComputeGraphModule.pri)` (grep).

### Точки входа и сборочные цели

- Своей сборочной цели нет: подключается как `.pri` в чужой `.pro` (пример — `Sandbox/Sandbox.pro`, grep).
- Вход в код: `#include "ComputeGraphModule/internal.hpp"` → `ComputeGraphCore::Instance()->Initialize(stream)` → `GetRootNode()` / геттеры узлов; поток запускается через `GtComputeGraph` (`ThreadComputingBase::Start`, индекс SharedModule) (точный порядок вызовов — (?)).

### Где искать

| Задача | Файлы |
|---|---|
| Добавить новый узел обработки | `computenodebase.h` (переопределить `onInputChanged`/`update`/`getMemoryUsage`), образец — `computenodeblur.*`; зарегистрировать в `computegraphcore.cpp/.h` (макросы `ADD_PROPERTY_NODE*`) |
| Изменить порядок/состав цепочки | `computegraphcore.cpp` (`Initialize`) |
| Частота кадров, цикл потока | `computegraphbase.*` (`idealFrameTime`, `compute`) |
| Чтение выхода графа из GUI/другого потока | `computenodethreadsafe.*` (`GetThreadOutput`, `MatGuard`) |
| Работа с датчиком глубины | `opennisensor.*`, `computenodedepthsensor.*` |
| Воспроизведение записи вместо датчика | `inputframestream.*`, `computenodedepthfakesensor.*` |
| Параметры узлов (свойства) | заголовки `computenode*.h` (`IntProperty` и др.), валидаторы — `computenodegaussenblur.cpp`, `computenodemedianblur.cpp` |
| Распознавание | `computenodevolcanorecognition.*` |
| Учёт памяти узлов | `computenodebase.cpp` (`totalMemoryUsage`, `getMemoryUsage`) |

## NetworkModule

> Источники: индекс `.codemap/NetworkModule/` (`00_overview.md`, `index_01.md`, `codemap: v4` — совпадает с `python3 codemap.py --version`; 15 файлов, 775 строк; 8 `.h`, 5 `.cpp`, 1 `.hpp`, 1 `.pri`). Типы из строк `uses:` сверены grep-ом по индексам `.codemap/SharedModule/` и `.codemap/SharedGuiModule/` (оба `codemap: v4`); то, чего нет в индексах, — grep-ом по исходникам. Метки: [индекс] — индекс NetworkModule, [индекс X] — индекс папки X, [grep] — grep по исходникам или ФС. «(?)» — предположение. Условных (`[if X]`) include и объявлений в самом NetworkModule индекс не показывает; строк `defines:` нет.

### Назначение

Многопоточный TCP-сервер и клиентское соединение поверх `QTcpServer`/`QTcpSocket` с собственным пакетным протоколом: `NetworkPackage` (заголовок `NetworkPackageHeader` + данные, контрольная сумма) [индекс].
Сервер `NetworkServerBase` раздаёт входящие сокеты пулу потоков `NetworkThreadPool` → `NetworkThread` (по умолчанию 4 потока) [индекс].
Базовые структуры запросов двух протоколов: `RIProtocholQueryBase` и `SimplifiedProtocholQueryBase` (+ `SimplifiedProtocholErrorQuery`) [индекс].

### Подмодули

| Путь | Назначение | Ключевые классы/файлы |
|---|---|---|
| `./` (11 файлов, 568 строк) | Публичный API: пакет, соединение, сервер, базовые запросы протоколов | `networkpackage.h/.cpp` (`NetworkPackageHeader`: `IsSynchronized`, `Serialize`; `NetworkPackage`: `Pack(QByteArray)`, `Pack<T>`, `Read<T>`, `GenerateCheckSum`, `CheckSum`, `GetData`, `IsEmpty`, `ToByteArray`, `Serialize`); `networkconnection.h/.cpp` (`NetworkConnection : QObject`: `Write<T>`/`Write(NetworkPackage)`, `Connect`, `ConnectWithResult` → `AsyncResult`, `Disconnect`, `IsConnected`, `SetSocketDescriptor`, `GetSocketDescriptor`, `LessThan`, сигнал `disconnected`, слоты `onReadyRead`, `onSocketStateChanged`); `network_decl.h` (интерфейс `INetworkConnectionOutput::onPackageRecieved(descriptor, package)`, protected pure virtual); `networkserverbase.h/.cpp` (`NetworkServerBase : QTcpServer, INetworkConnectionOutput`: ctor `(threadsCount = 4, parent = 0)`, `StartServer`, `IsRunning`, `SetHostPort`, `SetHost`, `incomingConnection` final, виртуальный `onConnectionLost`, protected `write<T>`/`write(descriptor, NetworkPackage)`); `riprotocholquerybase.h` (`RIProtocholQueryType {RIProtocholQuery_Error}`, `RIProtocholQueryBase`); `simplifiedprotocholquerybase.h` (`SimplifiedProtocholQueryType {Unknown, Error, Last}`, `SimplifiedProtocholQueryBase`, `SimplifiedProtocholErrorQuery::CreatePackage(const Name& errorId)`); `internal.hpp` — агрегатор (`networkserverbase.h`, `networkpackage.h`, `networkconnection.h`, `network_decl.h`); `NetworkModule.pri` [индекс] |
| `Private/` (4 файла, 207 строк) | Пул сетевых потоков сервера | `networkthreadpool.h/.cpp` (`NetworkThreadPool`: `Start`, `Quit`, `IsRunning`, `AddSocket(id)`, `Write(id, package)`, приватный `findEmtiestThread` — выбор наименее загруженного потока (?, по имени)); `networkthread.h/.cpp` (`NetworkThread : QThread`: `Write`, `AddSocket`, `GetConnectionsCount`, слоты `onDisconnected`, `onRemoveInvalidConnections`) [индекс] |

### Зависимости

От других папок DevLib:
- **SharedModule** — `Private/networkthread.h` включает `<SharedModule/internal.hpp>` [индекс]. Используемые типы (из `uses:`):
  - `AsyncResult` (`alias AsyncResult = Promise`, `SharedModule/Threads/Promises/promise.h`) — результат `NetworkConnection::ConnectWithResult` [индекс SharedModule];
  - `ThreadTimer` (`SharedModule/Threads/threadtimer.h`) — в `networkconnection.cpp` [индекс SharedModule];
  - `ThreadsBase` (`SharedModule/Threads/threadsbase.h`) — в `Private/networkthread.cpp` [индекс SharedModule];
  - `SerializerReadBuffer`/`SerializerWriteBuffer` (алиасы `TSerializer*Buffer<QStreamBufferDataStream>`, `SharedModule/Serialization/qserializer.h`) — сериализация в `networkpackage.h` [индекс SharedModule];
  - `Name` (`SharedModule/name.h`) — параметр `SimplifiedProtocholErrorQuery::CreatePackage` [индекс SharedModule];
  - `ArrayPointers` (`SharedModule/array.h`) и `ScopedPointer` — поля `NetworkThread` [индекс SharedModule; объявление `ScopedPointer` в индексе не найдено, только использования — grep: `SharedModule/smartpointersadapters.h:8`, `class ScopedPointer : public std::unique_ptr<T, deleter>`];
  - `StackPointers` (`SharedModule/stack.h`) — в `NetworkThreadPool` [индекс SharedModule].
- **SharedGuiModule** — `networkpackage.h` включает `<SharedGuiModule/internal.hpp>` [индекс]; тот подключает `decl.h`, а `decl.h` — `<SharedModule/internal.hpp>` безусловно и Qt GUI/OpenGL-заголовки условно (`[if QT_GUI_LIB]`, `[if QT_GUI_LIB && !QT_NO_OPENGL]`) [индекс SharedGuiModule]. Ни один тип из `uses:` NetworkModule не объявлен в SharedGuiModule [grep по индексу SharedGuiModule], так что include, по-видимому, нужен только транзитивно ради SharedModule (сериализатор) (?). Без `QT_GUI_LIB` GUI-часть `decl.h` не подтягивается [индекс SharedGuiModule].
- Qt: `QtNetwork` (`QTcpSocket`, `QTcpServer`, `QHostAddress`) [индекс]. `NetworkModule.pri` состоит из одной строки `includeAll(true)`, `QT += network` в нём нет [grep: cat]; откуда подключается Qt Network, неизвестно (?).

Кто зависит от NetworkModule (из уже описанных):
- **CloudModule** — не использует, сеть идёт напрямую через `QNetworkAccessManager` [раздел CloudModule этой карты].
- Grep по исходникам DevLib вне `NetworkModule/` (`*.h`, `*.hpp`, `*.cpp`, `*.pri`, `*.pro`) не нашёл ни одного упоминания `NetworkModule` [grep]; grep по индексам других папок — ни одного упоминания `NetworkPackage`/`NetworkServerBase`/`NetworkConnection`/`NetworkModule` [grep]. Видимо, модуль используется только вне DevLib (wps-client) (?).

### Точки входа и сборочные цели

- Сборка: `NetworkModule/NetworkModule.pri` = `includeAll(true)` [индекс, grep]. Отдельной цели (`.pro`/`TARGET`) нет [индекс].
- Подключение: `#include <NetworkModule/internal.hpp>` [индекс].
- Сервер: наследовать `NetworkServerBase`, переопределить `onPackageRecieved(descriptor, package)` (из `INetworkConnectionOutput`) и при необходимости `onConnectionLost`; затем `SetHost`/`SetHostPort` → `StartServer()`; ответ клиенту — `write(descriptor, query)` [индекс].
- Клиент: `NetworkConnection(output)` с реализацией `INetworkConnectionOutput` → `Connect`/`ConnectWithResult(host, port)` → `Write(query)` [индекс].
- Поток данных на сервере: `incomingConnection(handle)` → `NetworkThreadPool::AddSocket` → `NetworkThread::AddSocket` → `NetworkConnection::SetSocketDescriptor`; чтение `onReadyRead` → `INetworkConnectionOutput::onPackageRecieved` (?, порядок вызовов выведен из сигнатур и `uses:`, тела не смотрели).

### Где искать

| Задача | Файлы |
|---|---|
| Формат пакета, заголовок, контрольная сумма, (де)сериализация | `NetworkModule/networkpackage.h`, `networkpackage.cpp` |
| Клиентское подключение, отправка, чтение из сокета | `NetworkModule/networkconnection.h`, `networkconnection.cpp` |
| Callback о полученном пакете | `NetworkModule/network_decl.h` (`INetworkConnectionOutput`) |
| Свой TCP-сервер, хост/порт, обработка отключения | `NetworkModule/networkserverbase.h`, `networkserverbase.cpp` |
| Число потоков сервера, распределение соединений | `NetworkModule/Private/networkthreadpool.h/.cpp`, `Private/networkthread.h/.cpp` |
| Новые типы запросов «Simplified»-протокола, пакет ошибки | `NetworkModule/simplifiedprotocholquerybase.h` |
| Базовый запрос RI-протокола | `NetworkModule/riprotocholquerybase.h` |
| Что подключается одним include | `NetworkModule/internal.hpp` |

## ProgressModule

> Источники: индекс `.codemap/ProgressModule/` (`00_overview.md`, `index_01.md`, `codemap: v4` — совпадает с `python3 codemap.py --version`; 7 файлов, 405 строк; 3 `.h`, 2 `.cpp`, 1 `.hpp`, 1 `.pri`). Типы из строк `uses:` сверены grep-ом по индексам других папок (`.codemap/*/index_*.md`); не найденные там — grep-ом по исходникам. Метки: [индекс] — индекс ProgressModule, [индекс X] — индекс папки X, [grep] — grep по исходникам или ФС. «(?)» — предположение.

### Назначение

Небольшая модель прогресса долгих операций без собственного UI: интерфейс `IProgress` (начать, задать заголовок, шаг, проверить отмену), реализации `Progress` и `ProgressDummy` и RAII-помощник `ProgressIncrementGuard` [индекс].
Синглтон `ProgressFactory` создаёт объекты состояния `ProgressValue` (неопределённый) и `ProgressDeterminateValue` (с шагами), обычные и «теневые» (shadow), и вызывает зарегистрированные callback-и `ProgressValue::FCallback` (`typedef std::function<void (ProgressValue*)>`, `progressfactory.h:22`). Через эти callback-и UI рисует прогресс [индекс, grep].
По составу классов модуль похож на `ProcessBase`/`ProcessFactory`/`ProcessValue` из `SharedModule/Process/`. Видимо, это более старый или параллельный вариант той же подсистемы (?).

### Подмодули

| Путь | Назначение | Ключевые классы/файлы |
|---|---|---|
| `./` (7 файлов, 405 строк) | Весь модуль: интерфейс, реализации, фабрика и состояния прогресса | `iprogress.h` (`IProgress`: `BeginProgress(title, isShadow)`, `BeginProgress(title, count, isShadow)`, `SetProgressTitle`, `IncreaseProgressStepsCount`, `IncrementProgress`, `IsProgressCanceled` — все чисто виртуальные; `ProgressIncrementGuard(IProgress*)` с inline-деструктором, который, судя по имени, вызывает `IncrementProgress` (?)); `progress.h/.cpp` (`Progress : IProgress`, по умолчанию `shadow = false`, использует `ScopedPointer`, реализация через `ProgressFactory`; `ProgressDummy : IProgress` — `final`-методы без вывода); `progressfactory.h/.cpp` (`DescProgressValueState::IsShouldStayVisible`; `ProgressValue`: `GetState`, `GetDepth`, `GetTitle`, `IsCanceled`, `IsNextProgressExpected`, `IsFinished`, `AsDeterminate`, protected `setTitle`/`finish`/`setNextProgressExpected`/`incrementStep`/`init`, в `.cpp` ещё `depthCounter`; `DescProgressDeterminateValueState : DescProgressValueState`; `ProgressDeterminateValue : ProgressValue` (`GetCurrentStep`, `GetStepsCount`, `increaseStepsCount`; `typedef ProgressValue Super`, `progressfactory.h:66`); `ProgressFactory::Instance`, `Set{,Shadow}{Determinate,Indeterminate}Callback`, `create{,Shadow}{Indeterminate,Determinate}`; статический `DoNothingCallback(ProgressValue*)` в `.cpp` — callback по умолчанию (?)); `internal.hpp` — агрегатор (`progressfactory.h`, `progress.h`); `ProgressModule.pri` [индекс, grep] |

Замечание: `progressfactory.cpp` подключает `"Progressfactory.h"` (с заглавной `P`), а файл на диске — `progressfactory.h` [индекс, grep]. На регистрозависимой ФС это не скомпилируется (?).

### Зависимости

От других папок DevLib:
- **SharedModule** — `iprogress.h` и `progressfactory.h` подключают `<SharedModule/internal.hpp>` [индекс]. Из `uses:` к SharedModule относится только `ScopedPointer` (`progress.h`): в индексе SharedModule он встречается лишь в сигнатурах (`ToReference(ScopedPointer<T>&)`, `make_scoped`, `make_unique`) [индекс SharedModule], объявлен в `SharedModule/smartpointersadapters.h:8` (`class ScopedPointer : public std::unique_ptr<T, deleter>`) [grep].
- Остальные типы из `uses:` — свои: `FCallback` (`progressfactory.h:22`), `Super` (`progressfactory.h:66`), `ProgressFactory`, `ProgressValue` [индекс, grep]. Одноимённые `FCallback`/`Super` в индексах SharedModule, NetworkModule, PropertiesModule, GraphicsToolsModule — другие типы, к ProgressModule не относятся [индекс X, grep].
- Классы `SharedModule/Process/*` (`ProcessFactory`, `ProcessValue`) ProgressModule не использует [индекс].
- Условных (`[if X]`) include и объявлений в индексе нет; макросов (`defines:`) модуль не объявляет [индекс].
- Qt: `QString` в сигнатурах; STL: `<functional>`, `<memory>`, `<atomic>`, `<string>` [индекс].

Кто зависит от ProgressModule:
- Из уже описанных в этой карте модулей (SharedModule, PropertiesModule, GraphicsToolsModule, WidgetsModule, CloudModule, CommandsModule, ComputeGraphModule, NetworkModule, SQLiteModule, StylesModule) — никто: grep по `.codemap/*/*.md` не нашёл `ProgressModule`/`IProgress`/`ProgressFactory`/`ProgressValue`/`ProgressIncrementGuard` вне индекса самого модуля [grep]. Прогресс-бары WidgetsModule (`ProgressBars/*`) работают с `ProcessFactory`/`DescProcessValueState` из SharedModule, а не с этим модулем [раздел WidgetsModule этой карты].
- По исходникам DevLib — только `OpenGLMultithreadingTest` (не проиндексирован): `OpenGLMultithreadingTest.pro:21` — `include($$CABOODLE/ProgressModule/ProgressModule.pri)`; `glrender.cpp:3` и `main.cpp:5` — `#include <ProgressModule/internal.hpp>`; `main.cpp:27,42` — `ProgressFactory::Instance().SetDeterminateCallback/SetIndeterminateCallback([...](ProgressValue* value){...})` [grep].

### Точки входа и сборочные цели

- Сборка: `ProgressModule/ProgressModule.pri` = `includeAll(true)` [grep]; раздел «Build files» в индексе пуст [индекс]. Своей цели (`.pro`/`TARGET`) нет. Модуль подключают через `include($$CABOODLE/ProgressModule/ProgressModule.pri)` [grep].
- Подключение в коде: `#include <ProgressModule/internal.hpp>` [индекс].
- Код операции: `Progress p; p.BeginProgress(title, count); ... p.IncrementProgress(); if (p.IsProgressCanceled()) ...`, либо `ProgressIncrementGuard`; если прогресс не нужен — `ProgressDummy` [индекс].
- UI: `ProgressFactory::Instance().Set*Callback([](ProgressValue* v){ ... v->GetState() / v->AsDeterminate() ... })` [индекс, grep].

### Где искать

| Задача | Файлы |
|---|---|
| Интерфейс прогресса для функций, RAII-инкремент | `ProgressModule/iprogress.h` |
| Реализация прогресса, заглушка без вывода | `ProgressModule/progress.h`, `progress.cpp` |
| Подключить UI (callback-и обычного и теневого прогресса) | `ProgressModule/progressfactory.h` (`ProgressFactory`), `progressfactory.cpp` |
| Состояние прогресса: шаги, глубина вложенности, отмена, видимость | `ProgressModule/progressfactory.h` (`ProgressValue`, `ProgressDeterminateValue`, `Desc*State`), `progressfactory.cpp` |
| Callback по умолчанию (ничего не делает) | `ProgressModule/progressfactory.cpp` (`DoNothingCallback`) |
| Пример использования | `OpenGLMultithreadingTest/main.cpp` |
| Что подключается одним include | `ProgressModule/internal.hpp` |

## SQLiteModule

> Источники: индекс `.codemap/SQLiteModule/` (`00_overview.md`, `index_01.md`; `codemap: v4` совпадает с `python3 codemap.py --version`; 4 файла, 677 строк); зависимости сверены grep-ом по индексам `.codemap/SharedModule/` и `.codemap/PropertiesModule/` (оба `codemap: v4`). Условных (`[if X]`) include/объявлений и строк `defines:` в индексе нет.

### Назначение

Тонкая обёртка над Qt SQL (`QT += sql`) для работы с базой SQLite в фоновом потоке: класс `SQLiteDatabase` (наследник `ThreadPool`) выполняет транзакции и запросы асинхронно, возвращая `AsyncResult` [индекс].
Плюс шаблонный слой отображения C++-типов в поля БД (`SQLiteTypeFor`, `SQLiteMapping`) и табличная модель `TSQLiteModelsTableModel<T>` на базе `TModelsTableWrapper<QVector<T>>` из PropertiesModule [индекс].
Почти вся логика в одном заголовке `sqlitedatabase.h` (481 строка), большая часть — шаблоны [индекс].

### Подмодули

| Путь | Назначение | Ключевые классы/файлы |
|---|---|---|
| `SQLiteModule/` (единственная папка) | Асинхронный доступ к SQLite, маппинг типов, табличная модель | `sqlitedatabase.h`: `SQLiteDatabase` (+ `enum Op {In, NotIn}`), `SQLiteDatabaseFieldType {Undefined, Text, Int, Blob}`, `SQLiteTypeFor<T>` (специализации для `Name`, `qint32`, `QByteArray`, `QDateTime`, `QString`, `bool`), `SQLiteMappingValue`, `SQLiteMapping<T>` (`QMap<QString, SQLiteMappingValue<T>>`), `SQLiteModelsTableModelParams`, `TSQLiteModelsTableModel<T>`; `sqlitedatabase.cpp` — реализация `SQLiteDatabase`; `internal.hpp` — агрегирующий include (только `sqlitedatabase.h`); `SQLiteModule.pri` |

### Зависимости

От каких папок DevLib зависит (все зависимости безусловные):

- **SharedModule** — include `<SharedModule/Threads/ThreadFunction/thread.h>` в `sqlitedatabase.cpp` [индекс]; в `sqlitedatabase.h` типы SharedModule приходят транзитивно через `<PropertiesModule/internal.hpp>` (?). Типы из `uses:` / базовые классы, найденные в индексе SharedModule [grep]:
  - `ThreadPool` — `SharedModule/Threads/ThreadFunction/threadpool.h`, базовый класс `SQLiteDatabase`;
  - `ThreadsBase` — `SharedModule/Threads/threadsbase.h`;
  - `AsyncResult` (= `Promise`) — `SharedModule/Threads/Promises/promise.h`;
  - `AsyncRequest` — `SharedModule/interruptor.h`;
  - `EPriority {High, Low, Count}` — `SharedModule/declarations.h` (используется в `sqlitedatabase.cpp`);
  - `Name` — `SharedModule/name.h` (тип id в БД);
  - `StringBuilder` — `SharedModule/builders.h`;
  - `DispatcherConnectionsSafe` (= `QVector<DispatcherConnectionSafePtr>`) — `SharedModule/dispatcher.h`;
  - `SP<T>` (= `SharedPointer<T>`) — `SharedModule/smartpointersadapters.h`.
- **PropertiesModule** — include `<PropertiesModule/internal.hpp>` в `sqlitedatabase.h` [индекс]. Типы [grep по индексу PropertiesModule]:
  - `TModelsTableWrapper` — `PropertiesModule/Models/wrappers.h`, базовый класс `TSQLiteModelsTableModel<T>`;
  - `StateProperty` — `PropertiesModule/stateproperty.h`.
- Внешние: Qt SQL (`QSqlDatabase`, `QSqlQuery`, `QSqlError`) — [индекс], `SQLiteModule.pri`.

Кто зависит от SQLiteModule: среди индексов `.codemap/*` (кроме самого модуля) упоминаний `SQLiteModule`/`SQLiteDatabase` нет [grep]; в описанных разделах CODEMAP.md модуль упоминается только как потребитель PropertiesModule. Потребители, вероятно, вне DevLib (?).

### Точки входа и сборочные цели

- Сборка: `SQLiteModule/SQLiteModule.pri` — подключается в .pro потребителя, добавляет `QT += sql`; собственной цели (`.pro`/CMake) нет [индекс].
- Точка входа в API: `SQLiteDatabase::Create(connection, user, password)` → `SP<SQLiteDatabase>`; подключение через `#include <SQLiteModule/internal.hpp>` [индекс].

### Где искать

| Задача | Файл / место |
|---|---|
| Открыть БД, выполнить запрос/транзакцию | `sqlitedatabase.h` — `SQLiteDatabase::Create`, `Query`, `Transaction`; реализация в `sqlitedatabase.cpp` |
| Проверить наличие id / выбрать по списку id | `SQLiteDatabase::IdExists`, `QueryWhereIdIn(..., Op In/NotIn)` |
| Остановить поток БД | `SQLiteDatabase::Terminate`, `~SQLiteDatabase` (`sqlitedatabase.cpp`) |
| Добавить поддержку нового C++-типа поля | специализация `SQLiteTypeFor<T>` (`ToVariant`/`FromVariant`) в `sqlitedatabase.h` |
| Описать маппинг полей структуры на колонки | `SQLiteMapping<T>`, `SQLiteMappingValue` — `Fields`, `FieldNames`, `FieldValues` |
| Табличная модель для UI из таблицы БД | `TSQLiteModelsTableModel<T>` — `Connect`, `Disconnect`, `Select`; опции `SQLiteModelsTableModelParams::AddUpdatedAt`, `AddMarkedAsDeleted` |
| Подключение Qt SQL к проекту | `SQLiteModule.pri` |

## StylesModule

> Источники: индекс `.codemap/StylesModule/00_overview.md` (`codemap: v4`, совпадает с `python3 codemap.py --version`). В индексе 1 файл (`StylesModule.pri`, 2 строки); файлов `index_*.md` нет, строк `inc:`/`uses:` нет: в папке нет ни одного `.h`/`.cpp`. Остальное взято из листинга ФС, grep по исходникам и индексам других папок (`WidgetsModule/index_01.md`, `PropertiesModule/index_01.md`) и из дампа прежнего прогона (`WidgetsModule/qtqssreader.cpp`, `style/main.qss`, `style/ads.qss`, `aispaces_dark.qss`, `Sandbox/gtwindow.cpp`). Текущий `.codemap/_dump/dump_01.md` содержит только SharedModule. Метки: [индекс] — индекс StylesModule, [индекс X] — индекс папки X, [ФС] — листинг файлов, [grep], [дамп] — дамп прежнего прогона. «(?)» — предположение.

### Назначение

Ресурсная папка без C++-кода: шаблоны таблиц стилей Qt (QSS) и SVG-иконки, собранные в `main.qrc` (`prefix="/"`, 123 записи `<file>`) [индекс, ФС, grep].
QSS написан на расширенном синтаксисе, который понимает только `QtQSSReader` из WidgetsModule [индекс WidgetsModule, дамп]:
- `@cИмя: значение;` — палитра; `@cbg.*`, `@cbr.*`, `@ctext.*`, `@cIcon.*`, `@cpal.*`, `@c.*` подставляются во все файлы;
- `@import url("...");` — склейка файлов;
- `@resource.N::каталог;` — каталог иконок-шаблонов;
- `@temp` — временная папка приложения.

Точки входа тем: `style/main.qss` — базовая тёмная тема; `aispaces_dark.qss` / `aispaces_light.qss` — темы продукта AiSpaces (база `style/*` плюс надстройка `aispaces/*`) [дамп, grep].

### Подмодули

| Путь | Назначение | Ключевые классы/файлы |
|---|---|---|
| `./` | Сборка, ресурсный файл, корневые файлы тем AiSpaces | `StylesModule.pri` (`includeAll(true)`) [индекс, grep]; `main.qrc` [ФС]; `aispaces_dark.qss`, `aispaces_light.qss`: палитра `@c*`, `@cPath:@temp`, `@cLogoPath:@temp/full_logo_white.svg` (dark) / `full_logo_black.svg` (light), `@resource.1::/style/res`, `@resource.2::/aispaces/res`, 7 `@import "style/*.qss"` (вместе с `progressbar.qss`) + `aispaces/style.qss`, `aispaces/palette.qss` [grep]; `tmp_dark.qss` в `main.qrc` не входит — черновик (?) [grep] |
| `style/` (8 `.qss` + `res/`, 92 `.svg`) | Базовая тема и стили виджетов | `main.qss` — точка входа базовой темы: палитра, `@cPath:@temp`, `@resource.1::/style/res`, `@import` `style`, `ads`, `palette`, `widgets`, `notify`, `visualization` (без `progressbar.qss`) [дамп]; `ads.qss` — стили Qt Advanced Docking System (`ads--CDockManager`, `CDockWidgetTab`, `CDockAreaWidget[a_focus_view=true]`, `CDockAreaTitleBar`; иконки `url(@cPath/close.svg)` и др.) [дамп]; `style.qss`, `widgets.qss`, `palette.qss`, `notify.qss`, `visualization.qss`, `progressbar.qss` [ФС]; `res/*.svg` — иконки-шаблоны (стрелки, chevron, checkbox, lock, pin, close, detach и т. п.) [ФС, дамп] |
| `aispaces/` (2 `.qss` + `res/`, 19 `.svg`) | Надстройка темы AiSpaces | `style.qss`, `palette.qss`; `res/`: `logo.svg`, `full_logo*.svg`, `save`/`discard`, `cloud_upload`/`cloud_download`, `view_3d`/`view_left`/`view_top`, `insertBefore`/`insertAfter` и др. [ФС] |

### Как применяется тема (`QtQSSReader`, `WidgetsModule/qtqssreader.h/.cpp`)

API по индексу: `Install(const QString& mainQSSFile)`, `ReadAll()`, `SetEnableObserver(bool)`; `qtqssreader.cpp` подключает `<SharedModule/External/external.hpp>`, `uses: SharedSettings ThreadsBase` [индекс WidgetsModule]. Алгоритм `ReadAll` [дамп]:
1. В тексте главного файла `@temp` заменяется на `SharedSettings::GetInstance().PathSettings.TempDir.absolutePath()` (`SharedSettings` объявлен в `PropertiesModule/sharedsettings.h` [индекс PropertiesModule]).
2. Палитра собирается regex-ом `(@c[\d\w_\.]+):([^;]+)` только из главного файла. Если цвет не найден, подставляется `red` и пишется `qDebug "Can't find %1 color"`.
3. Каждый файл из каталога `@resource...` (например, `:/style/res`) копируется в `TempDir` с подставленными цветами. Поэтому в QSS иконки берутся как `url(@cPath/имя.svg)`, где `@cPath` = `@temp`.
4. `@import` разрешаются относительно папки главного файла. Содержимое склеивается с подстановкой цветов, итог уходит в `qApp->setStyleSheet`. Если файла нет, пишется `qCWarning(LC_SYSTEM) "No such file"`.
5. `SetEnableObserver(true)` включает `QtObserver` (500 мс): при изменении главного или импортируемого файла `Install` вызывается заново — горячая перезагрузка стилей.

### Зависимости

От других папок DevLib:
- Зависимостей по include нет: в папке нет C++-файлов, `uses:` отсутствует [индекс, ФС].
- Данные рассчитаны на препроцессор `QtQSSReader` (WidgetsModule) [индекс WidgetsModule].
- Через него QSS косвенно зависит от `SharedSettings::PathSettings.TempDir` (`PropertiesModule/sharedsettings.h`) [индекс PropertiesModule, дамп].
- Пути `:/style/res` и `:/aispaces/res` в `@resource` требуют, чтобы `main.qrc` был вкомпилирован в приложение [grep].

Кто зависит от StylesModule:
- Grep по индексам всех папок `.codemap/*` не нашёл `StylesModule`, `main.qrc`, `aispaces` или `.qss` [grep]. В `.pri`/`.pro`/`.cpp` DevLib нет `main.qrc` и `Q_INIT_RESOURCE` [grep]. Ресурс подключает внешнее приложение (wps-client) (?).
- WidgetsModule (уже описан) — единственный потребитель формата: `QtQSSReader` [индекс WidgetsModule].
- PropertiesModule (уже описан): `StyleSettings::InstallQSSReader(const QString& path, bool dynamic = true)` и `Release()` — условно, `[if SHARED_LIB_ADD_UI]`; `sharedsettings.cpp` подключает `<WidgetsModule/internal.hpp>` `[if WIDGETS_MODULE_LIB]` [индекс PropertiesModule]. Внутри создаётся `QtQSSReader`, вызываются `SetEnableObserver(dynamic)` и `Install(path)` [grep]. Какой файл темы передаёт приложение, в DevLib не видно (?).
- Не описаны в карте: `Sandbox/gtwindow.cpp` — свойство `main_qss("Common/QSS import file", GT_STYLES_PATH "main.qss")`, где `GT_STYLES_PATH` = `GT_CONTENT_PATH "CSS/"` (`SharedGuiModule/decl.h:844`); `Install(main_qss.Native())` при старте и по `Subscribe` [grep]. Файл читается с диска из `Content/CSS/`; по имени это копия `style/main.qss` (?). `DatabaseBrowser/databasebrowser.cpp` — свойство `m_qssFile("Style/Path", "")`, `SetEnableObserver(true)`, `Install(m_qssFile)` по `Subscribe` [grep]; подключает `<SharedModule/External/qtqssreader.h>`, которого в `SharedModule` нет [grep] — устаревший include (?).

### Точки входа и сборочные цели

- `StylesModule/StylesModule.pri` = `includeAll(true)` [индекс, grep]. Своей цели нет.
- Ресурсы `main.qrc`: `:/style/...`, `:/aispaces/...`, `:/aispaces_dark.qss`, `:/aispaces_light.qss` [grep].
- Главные файлы тем для `QtQSSReader::Install(path)` / `StyleSettings::InstallQSSReader(path)`: `style/main.qss`, `aispaces_dark.qss`, `aispaces_light.qss` [дамп, grep]. Вложенные `.qss` сами по себе не работают: палитра объявлена только в главном файле [дамп].

### Где искать

| Задача | Файлы |
|---|---|
| Цвета темы (переменные `@cbg.*`, `@ctext.*`, `@cIcon.*` и др.) | `StylesModule/aispaces_dark.qss`, `aispaces_light.qss`, `style/main.qss` (палитра только здесь) |
| Стили стандартных виджетов | `StylesModule/style/style.qss`, `style/widgets.qss`, `style/palette.qss` |
| Док-панели (Qt ADS): вкладки, заголовки, кнопки close/detach | `StylesModule/style/ads.qss` |
| Уведомления, прогресс-бары, визуализация | `style/notify.qss`, `style/progressbar.qss`, `style/visualization.qss` |
| Специфика AiSpaces, логотипы | `StylesModule/aispaces/style.qss`, `aispaces/palette.qss`, `aispaces/res/*.svg` |
| Добавить иконку | SVG в `style/res/` или `aispaces/res/` + запись в `main.qrc`; в QSS `url(@cPath/имя.svg)`; цвета в SVG можно задавать через `@c*` |
| Добавить новый QSS-файл | файл + `@import` в главный файл темы (+ `main.qrc`) |
| Разбор `@import`/`@c*`/`@resource`, горячая перезагрузка | `WidgetsModule/qtqssreader.cpp` |
| Установить тему из настроек приложения | `PropertiesModule/sharedsettings.h/.cpp` (`StyleSettings::InstallQSSReader`) |
| Пример подключения темы | `Sandbox/gtwindow.cpp` (`GtWindow::GtWindow`), `DatabaseBrowser/databasebrowser.cpp` |
