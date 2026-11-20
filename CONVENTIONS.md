# Соглашения по коду DevLib

Единый источник правил для кода, который использует DevLib. Скиллы и промпты ссылаются сюда; при расхождении прав этот файл.

Как читать: целиком не нужно. Один раздел — `python3 codemap.py section РАЗДЕЛ --show --file CONVENTIONS.md`. Где лежит класс — `python3 codemap.py find ИМЯ`. Номера строк здесь намеренно не указаны: они устаревают; место находится через `find`.

Перед написанием или ревью кода: прочитать разделы по теме и всегда «Известные ошибки» и «Чеклист перед коммитом»; после кода пройти чеклист по каждому пункту — выполнен / не относится / нарушен (где). В ответе указывать источник: карта, индекс, соглашения, grep или исходник. Список разделов — `python3 codemap.py section ? --show --file CONVENTIONS.md` (раздела «?» нет, скрипт печатает имеющиеся) [проверено]. Код самого wps-client вне `libs/DevLib` подчиняется ещё и `CONVENTIONS.md` в корне wps-client.

Пометки: **[проверено]** — подтверждено чтением кода; **[вывод]** — следует из кода, не компилировалось; **[не проверено]** — со слов или не перепроверялось. Сверено с рабочей копией 2026-11-02 (выборочно, см. каждое правило).

## Оглавление

- [Поиск кода](#поиск-кода)
- [Выбор механизма](#выбор-механизма)
- [LocalProperty](#localproperty)
- [Вычисления](#вычисления)
- [Потоки](#потоки)
- [Подписки и время жизни](#подписки-и-время-жизни)
- [Сериализация](#сериализация)
- [EditingObject и SharedSettings](#editingobject-и-sharedsettings)
- [UI: реактивные таблицы](#ui-реактивные-таблицы)
- [UI: окна и WidgetBuilder](#ui-окна-и-widgetbuilder)
- [Сборка теста](#сборка-теста)
- [Стиль кода](#стиль-кода)
- [Известные ошибки](#известные-ошибки)
- [Чеклист перед коммитом](#чеклист-перед-коммитом)
- [Правка .ui-файлов (Qt Designer)](#правка-ui-файлов-qt-designer)
- [Git](#git)
- [Стили (QSS)](#стили-qss)

## Поиск кода

1. `python3 codemap.py find ИМЯ [ИМЯ2 ...]` — где объявлено, реализовано, кем используется, что сказано в CODEMAP.md; несколько имён — одним вызовом. `@N` у записи — строка исходника, где начинается объявление; `impl X: Метод@N` — строки определений в `.cpp`. Где используется — добавить `--lines`: точные строки кода по всем картам (список «used by files» неполный: он строится по строке `uses:` индекса, где только 40 самых частых типов файла). `--all` — без обрезки списков.
2. Если `NOT FOUND` — `grep -rn -w "ИМЯ" --include=*.h --include=*.hpp --include=*.cpp .`
3. Обзор модуля — `python3 codemap.py section МОДУЛЬ --show` (раздел CODEMAP.md).
4. Только потом читать найденные файлы — точечно, только найденные на шагах 1–2; сигнатуры, на которые опирается новый код, сверять с заголовком (индекс и карта могут отставать от кода).

- Вывод «такого класса нет» делается только после шагов 1–2 по рабочей копии DevLib, а не по частичным копиям файлов.
- `ui_X.h` в исходниках нет: его генерирует uic из `X.ui` в той же папке.
- Типы из макросов (`DECLARE_MODEL_BY_TYPE`, `DECLARE_WITH_FIELD`, `SHARED_DECLARE_EVENT_FOR_ENUM` и др.) в индексе видны только как `defines:`; ищутся grep'ом по имени макроса.
- Состояние индексов: `python3 codemap.py status` (OK / NOINDEX / OLD / CHANGED / MISSING). Если `find` пишет, что индексов нет, или есть статус NOINDEX, OLD или CHANGED (новый клон, новая версия скрипта, изменились исходники) — сразу, без вопроса пользователю: есть `OLD` — `python3 codemap.py sync && python3 codemap.py reindex`; только `NOINDEX` или `CHANGED` без `OLD` — `python3 codemap.py reindex`. `sync` только при `OLD`: он копирует `codemap.py` в сабмодули (файл в git). Это бесплатно, без ИИ и пишет только `.codemap/` и `codemap.folders`; `reindex` пересобирает все папки, кроме OK. `CODEMAP.md` этим не пересобирается: устаревшие разделы (`SECTION_STALE`) обновляет Опус по разделу «Карты кода: обслуживание» в `CONVENTIONS.md` корня wps-client.
- MISSING — папки больше нет, сообщить пользователю.
- Сторонние библиотеки (Qt-Ads-Custom, Eigen и т.п.) исключены из карты. Если `find` не показывает их класс как сторонний, выполнить `python3 codemap.py extern` один раз. Если ИМЯ — поле или член класса, `find` покажет его в блоке `not in indexes; found as field/member…` [проверено: `codemap.py`].
- Не подменять модуль: раздел карты `SharedModule` не отвечает про `PropertiesModule`; индекс одного модуля не заменяет другой. Вопрос про модуль целиком: сначала `section МОДУЛЬ --show`, затем при необходимости индекс `.codemap/МОДУЛЬ/`; нет ни раздела, ни индекса — сказать об этом и предложить пересобрать раздел через Опуса по процедуре из `CONVENTIONS.md` корня wps-client (раздел «Карты кода: обслуживание»; из корня wps-client индекс и признак раздела даёт `python3 codemap.py ensure ПУТЬ`).
- При NOT FOUND grep искать не только в DevLib: в корне wps-client — `WPSClient`, `libs` и `Launcher` (сторонние библиотеки в `libs/` тоже попадут — учитывать).
- Доступ (Cowork). Код лежит на компьютере пользователя; файлы в `/mnt/user-data/uploads` — случайная выборка. Команды — `mcp__remote-devices__device_bash` с префиксом `cd "$HOME/mnt/wps-client" && ` (пути от корня wps-client) или, если подключена только DevLib, `cd "$HOME/mnt/DevLib" && ` (пути без `libs/DevLib/`); интерпретатор `python3`. Без Cowork — терминал в той же папке (`python` / `py`). Сначала `ls "$HOME/mnt/"`: нужная папка есть — доступ не запрашивать. Иначе один раз `device_request_folder_access` для `D:\Work\wps-client` с короткой причиной на русском; такой папки на машине нет — спросить путь. Отказ не повторять: предложить кнопку «Add folder», прикрепить файлы или вставить фрагменты. Файл вне подключённой папки — сначала спросить пользователя, расширять ли доступ. Нет `codemap.py` в корне wps-client — работать из `libs/DevLib`: `find` видит только DevLib, остальное искать grep'ом.
- Файлы карты в корне DevLib: `CODEMAP.md` — по разделу `## Модуль` на модуль (подразделы «Назначение», «Подмодули», «Зависимости», «Где искать» и др.) [проверено: раздел `PropertiesModule`]; `codemap.folders` — список проиндексированных папок (в git); `.codemap/МОДУЛЬ/` — индекс модуля, генерируется, в git не хранится (`.gitignore`) [проверено]. В индексе: классы, сигнатуры, `inc:`, `uses:` (чужие типы, которые использует файл), `alias` (typedef/using), `defines:`, пометки `[if X]` (есть только при макросе X); вложенная папка — через `__` (`.codemap/WidgetsModule__Models/`) [проверено: `codemap.py`]. `.codemap/_dump/` — выгрузки исходников командой `codemap.py dump`, это не полный код.
- Субагент не видит навыков и этого файла. Поручая ему код, передать дословно: где лежит код и префикс `device_bash`; что `/mnt/user-data/uploads` — неполная выборка и по ней нельзя делать вывод «такого класса нет»; порядок поиска (шаги 1–4); что код обязан соответствовать этому файлу (разделы — `section РАЗДЕЛ --show --file …/CONVENTIONS.md`) и в конце нужен проход по чеклисту; что `CODEMAP.md`, `CONVENTIONS.md` и `.codemap/` он не редактирует.

## Выбор механизма

| Задача | Что брать | Статус |
|---|---|---|
| Состояние объекта, настройки, модель UI | `LocalProperty<T>` и наследники (`PropertiesModule/localproperty.h`) | **Основной. Новый код только на нём.** |
| Фоновый расчёт по входам | `StateParameters` + `StateChained*Calculator` (`PropertiesModule/stateproperty.h`, `PropertiesModule/Models/wrappers.h`) | Единственный разрешённый способ |
| Доступ по строковому пути: INI, дерево свойств | `Property`, `PropertiesSystem`, `PropertyPromise` (`property.h`, `propertiessystem.h`) | **Устаревший.** Новых `Property`/`TProperty`/`IntProperty` не создавать; существующий код переводится на `LocalProperty` |
| Событие виджета для кода вне его (запрос действия, уведомление о выборе) | Публичное поле `CommonDispatcher<Args...>` / `Dispatcher<Args...>`; подписка через `Connect(CDL, ...)`, обработчик в `MakeSafe(...)`, если подписчик может умереть раньше виджета | **Основной. Новый код только на нём.** |
| Нативные сигналы Qt (`Q_OBJECT`, `Q_SIGNALS` / `signals`) | Только если без них нельзя: сигнал требует сама Qt (делегаты и модели, `QAction`, метаобъектный вызов слота), нужен `QObject::connect` с `Qt::QueuedConnection` между потоками, или перехватывается встроенный сигнал стандартного виджета (`clicked`, `textChanged` и т.п.) | **Исключение. В остальных случаях — диспетчер.** |

- Исходящее событие виджета объявляется полем-диспетчером без `Q_OBJECT`, как в `WPSCTemplateSelector`: `CommonDispatcher<const WPSCViewTemplateItemPtr&> OnApplyTemplateRequested;`. Встроенный сигнал стандартного виджета ловится внутри класса и отдаётся наружу через диспетчер. Если `Q_OBJECT` всё же нужен, рядом с объявлением пишется, какое требование Qt его вызвало.
- Нужен INI или дерево свойств — мост `PropertyFromLocalProperty::Create(name, localProperty)` (`localproperty.h`) [проверено]. Адаптер регистрируется в текущем scope в конструкторе `Property`, а `~Property()` регистрацию не снимает — контейнер адаптеров должен жить не меньше scope [не проверено повторно].
- `PropertiesModel` и Save/Load работают только через `Property*` в scope, поэтому прямая регистрация `LocalProperty` не работает — только через мост [не проверено повторно].
- `StateProperty` — это `LocalPropertyBool` с `SetState`, к реестру не относится.
- Батч-обновления и ленивые производные (`LocalPropertyComputed`, счётчик ревизий, batch guard) **не реализованы**; решение о варианте не принято. Анализ вариантов — `docs/lazy-property-options.md`. `QProperty` не использовать: в Qt 5 его нет.

### Справка: опции, валидаторы, SharedSettings, EditingObject

Сигнатуры смотреть через `codemap.py find`; поведение проверять по `.cpp`. Пометка [по заголовкам] — поведение не проверено.

- Опции именованных свойств: `Option_IsExportable` (сохранять в файл), `Option_IsPresentable` (показывать в модели свойств), `Option_IsReadOnly`; готовые наборы `Options_Default`, `Options_ReadOnlyPresentable`, `Options_InternalProperty` [по заголовкам].
- Валидация: `CreateValidatorBuilder().Add(...).SetEpsilonEquality(eps).Install(property)` [по заголовкам]; у `LocalProperty` — `SetValidator(fn)`.
- Числовые диапазоны: `TDecimalProperty<T>(path, initial, min, max)`; у `LocalPropertyLimitedDecimal` — `SetMinMax`.
- `ExternalProperty` оборачивает существующую переменную по ссылке без копии; `TExternalProperty<T>(path, getter, setter)` — геттер и сеттер; `Property::Clone(newName)` — алиас на другое свойство [по заголовкам].
- Группы `SharedSettings` (синглтон): `NetworkSettings`, `PathSettings`, `MetricSettings`, `SaveLoadSettings`, `LanguageSettings`, `ControlSettings`, `OnChanged`; `StyleSettings` — только при `SHARED_LIB_ADD_UI`. Открыть группу как именованные свойства: `CreateGlobalProperties(prefix, container)` [по заголовкам]. Поля групп, например: `NetworkSettings` — `ProxyHost`, `ProxyPort`, `ProxyUserName`, `ProxyPassword`; `SaveLoadSettings` — `AutoSaveInMinutes`, `AutoSaveEnabled`, `ApplicationRestoreStateAfterRerun`; `LanguageSettings` — `LocaleIndex`, `ApplicationLocale`, `DateFormat` и др.; `ControlSettings` — `ZoomSensitivity`, `MouseSensitivity`; член `MetricSettings` имеет тип `MetricsSettings` [проверено: `sharedsettings.h`].
- Сериализация помимо бинарного (`SerializeToArray`) и XML (`SerializeToXML` / `DeSerializeFromXML`, `SharedModule/Serialization/string/xmlserializer.h` [проверено]): `SerializeToJsonObject` / `DeSerializeFromJson`; текст значений — `TextConverter<T>::ToText` / `FromText` [по заголовкам].
- `EditingObject`: объект хранит `EditingObject EditObject;` (`EditingObjectPtr` = `SharedPointer<EditingObject>`); методы `SetActive(active, disableModes = {})` → `bool`, `Save()`, `Discard()`, `IsDirty()`, `SetDirty(dirty = true)`, свойство `ForceDirty`, `OnDirtyChanged()` [проверено: `editingobject.h`]; подключение — `EditObject.OnDiscarded.Connect(CDL, fn)`; сигналы `OnEnteredEditingMode`, `OnLeavedEditingMode`, `OnAboutToBeSaved`, `OnAboutToBeDiscarded`, `OnDiscarded`, `DiffRequest`, `OnLeavingDirtyEditingMode` (с `DirtyMode`: Save, Discard, Cancel) [по заголовкам]. `TEditingObjectComponent<T, SerializationMode>(object, testChanged)` требует у `T` поля `EditObject`, `GetEditId()` и `OnChanged` [проверено]; сохраняет состояние при входе и восстанавливает при Discard.

### Устаревший Property (только поддержка существующего кода)

Новых `Property`, `TProperty`, `IntProperty` и т.п. не создавать. Ниже — справочник для поддержки уже существующего кода. Сигнатуры сверены с заголовками. Поведение из `.cpp` проверено только там, где стоит [проверено] (2026-11-16); остальное [не проверено].

- Именованное свойство: `IntProperty speed("Group/Speed", 10);`; путь с `/` задаёт группировку. Scope — `PropertiesSystem::Begin(scope)` / `End()` или `PropertiesSystemScopeGuard`.
- Значения читать через `PropertiesSystem::GetProperty<T>(path, scope)`, а не через метод `PropertiesScope::GetProperty<T>` (он пустой, см. «Известные ошибки»).
- `PropertyPromise<T>` — ссылка на свойство, которое может быть создано позже (другой модуль, позже по времени); `IsValid()` перед использованием. Методы: `GetProperty()` → `T*`, `Native()`, `operator const value_type&`, `= value` (запись), `Subscribe(fn)`, `InstallObserver(...)`; алиасы `IntPropertyPtr`, `BoolPropertyPtr`, `StringPropertyPtr` и др. [проверено: `propertypromise.h`].
- `SetDirect(v)` меняет значение без уведомлений: подписчики не узнают об изменении. Не использовать, если на свойство что-то подписано.
- Сохранение: `PropertiesSystem::Save/Load` пишут INI через QSettings в UTF-8. Сохраняются только свойства с `Option_IsExportable`; `SaveSelected` с несохраняемым именем срабатывает `Q_ASSERT` [проверено: `propertiesscope.cpp`]. `Save(file, scope, clearFile = true)` по умолчанию сначала удаляет файл; `Load(file, scope)` возвращает `false`, если файла нет, неизвестные ключи только пишет в лог, читает лишь `Option_IsExportable` и вызывает `Invoke()`, даже если значение не изменилось [проверено]. Ещё `SaveSelected`, `Store(scope)` → `QByteArray`, `Restore(data, scope)`, `Clear(scope)`, `ClearWithoutDeleting(scope)`.
- Запись из другого потока — через `Handler()` или `PropertiesSystem::Begin(thread, scope)`, а не прямым `SetValue`.
- `ExternalProperty` [проверено: `externalproperty.h`]: по ссылке на переменную — `ExternalIntProperty p("Name", myInt, min, max)` (так же `ExternalUIntProperty`, `ExternalDoubleProperty`, `ExternalFloatProperty` = `TExternalDecimalProperty<T>`), `ExternalStringProperty`, `ExternalBoolProperty`, `ExternalStdWStringProperty`, `ExternalTextFileNameProperty`; `ExternalColorProperty` — только при `QT_GUI_LIB`. Геттер и сеттер — `TExternalProperty<T>(path, getter, setter)`, сеттер получает `(newValue, oldValue)`. `ExternalPropertyProperty(path, Property*)` или `(path, propertyPath, scope)` — алиас на другое свойство.
- Типы [проверено: `property.h`]: `BoolProperty`, `StringProperty`, `UrlProperty`, `ByteArrayProperty` = `TProperty<T>`; `IntProperty`, `UIntProperty`, `DoubleProperty`, `FloatProperty` = `TDecimalProperty<T>`; классы `FileNameProperty`, `NamedUIntProperty`, `UrlListProperty`, `ListProperty<K>`, `SetProperty<K>`, `HashProperty<K,V>`, `PointerProperty<T>`, `SharedPointerProperty<T>`, `VariantProperty<T>` (хранит текст через `TextConverter<T>`); `ColorProperty`, `RectProperty`, `Vector3FProperty` — только при `QT_GUI_LIB`. Конструктор обычно `(path, initial, options = Options_Default)`; у `HashProperty` и `SetProperty` — `(path, options)`.
- Запись: `SetValue(QVariant)` → валидатор → сравнение со старым → запись через `Handler()` → `Invoke()`; возвращает `true`, только если значение изменилось [проверено: `property.cpp`]. `prop = value` вызывает `SetValue`. `Native()` и `operator const T&` возвращают ссылку без копии.
- `Validator()` — `std::function<void (const QVariant& old, QVariant& newValue)>`, меняет `newValue` до записи. `Handler()` — `std::function<void (const FSetter&)>`, по умолчанию просто выполняет запись; `Begin(scope)` возвращает обработчик, который получат все свойства, созданные до `End()` (например, для записи в другом потоке) [проверено: `propertiessystem.h`, `PropertiesScope::addProperty`].
- `TDecimalProperty<T>(path, initial, min, max, options)` зажимает значение валидатором; `SetMinMax(min, max)` меняет границы и сразу зажимает текущее значение [проверено: `property.h`].
- Контейнеры [проверено: `property.h`]: `HashProperty` и `SetProperty` — `Insert`, `Remove`, `Clear`, `contains`, `find`, `size`; `ListProperty` — `Add`, `Clear`, `Set`. Методы пишут мимо валидатора и уведомляют только при изменении, кроме `ListProperty::Add` и `ListProperty::Set` — они уведомляют всегда.
- Подписка: `OnChanged += {observer, action}` / `InstallObserver(observer, action)`, `ConnectAction(CDL, fn)` → `DispatcherConnection`; `Subscribe(fn)` отменить нельзя [проверено: `property.cpp`].
- Области: `PropertiesSystem::Global`, `InitProperties`, `Temp`, `Empty`; своя область создаётся по имени при первом обращении [проверено: `getOrCreateScope`]. Префиксы имён — `BeginPrefix("Prefix/")` / `EndPrefix()`.
- Поиск по имени: `GetValue(path, scope)`, `IsExists(path, scope)`, `Subscribe(path, fn)`, `ForeachProperty(fn)`. `GetValue(path)` и `SetValue(path, value)` аргумента scope не имеют и работают с текущей областью (между `Begin` и `End`) [проверено: `propertiessystem.cpp`].
- Тот же путь в одной области дважды — `Q_ASSERT_X` в `PropertiesScope::addProperty`; в release новое свойство молча заменяет прежнее в реестре [проверено: `QHash::insert`]. Перед созданием проверять `IsExists`.
- `PointerProperty` / `SharedPointerProperty` создаются с `Options_InternalProperty` (в файл не сохраняются), указатель хранится как число — между сессиями смысла не имеет [проверено: `property.h`].

## LocalProperty

- `SetValue(v)`: валидатор → min/max → сравнение → запись → `Invoke`; по умолчанию синхронно.
- Тихие пути без уведомления: `SetSilent`, `EditSilent`, `SetSilentWithValidators`. После серии тихих правок вызвать `Invoke()` самому.
- Другие записи [проверено: `localproperty.h`]: `TrySetValue(v)` — валидатор, min/max и сравнение; возвращает `true`, если значение изменилось. `Set(v, silent)` — при `silent` это `SetSilentWithValidators`, иначе `SetValue`. `Change(fn)` — `fn(T&)` меняет значение на месте и возвращает `true`, если изменило, тогда `Invoke()`; валидатор и min/max не вызываются. `SetValueForceInvoke(v)` уведомляет всегда.
- `LocalPropertySet/Hash/Vector`: `Insert/Remove/Clear/operator=` обходят валидатор; `operator=` уведомляет даже при равных значениях (кроме «оба пустые»).
- Подписка:
  - `Subscribe(fn)` / `SetAndSubscribe(fn)` **отменить нельзя** [проверено]. `SetAndSubscribe` сразу же вызывает `fn` один раз [проверено]. Только если подписчик живёт не меньше свойства.
  - Иначе — `Connect(CDL, h)`, `ConnectAndCall(CDL, h)`, `ConnectAction(CDL, a)` и `.MakeSafe(...)` только если обработчик может быть вызван для уничтоженного объекта (см. «Подписки и время жизни»).
- `ConnectFrom(CDL, h, a, b…)` и `ConnectProperties` захватывают источники **по ссылке** — источники должны пережить соединение.
- `ConnectBoth` — двусторонняя связь, рекурсия гасится флагом.
- Типы:
  - `LocalPropertyBool` (класс); `LocalPropertyString/Color/Locale/Name` = `LocalProperty<QString/QColor/QLocale/Name>`; `LocalPropertyPtr<T>`, `LocalPropertySharedPtr<T>` — для указателей [проверено: `localproperty.h`];
  - `LocalPropertyInt/UInt/Double/Float/Int64` = `LocalPropertyLimitedDecimal<T>` с `SetMinMax`;
  - `LocalPropertySequentialEnum<Enum>` требует `Enum::First/Last` и `EnumHelper<Enum>`;
  - `LocalPropertyOptional<P>` — поля `Value` и `IsValid`;
  - `LocalPropertyDate/Time/DateTime` — невалидная дата означает открытую границу.
- `LocalPropertyBoolCommutator` / `LocalPropertyIntCommutator` (And/Or) пересчитываются отложенно, не синхронно [проверено: alias на `…CommutatorBase<DispatchersCommutator>`].
- `ConnectFromDispatchers(…, QVector<Dispatcher*>)` у коммутаторов объявлен, но не определён — даст ошибку линковки [не проверено повторно].

## Вычисления

Правила:

1. **Вход** — публичный наследник `StateParameters`. Поля `StateParameterProperty<LocalPropertyX>`, инициализация `Field(this)`. Сложные данные — `StateParametersContainer` / `StateImmutableData` через `DECLARE_MODEL*` (`Models/wrappers.h`) [проверено: макрос `DECLARE_MODEL_BY_TYPE` в `wrappers.h`].
2. **Входы и выходы — всегда `public`.** Выход — `Result` обёртки, наружу отдаётся как `Output`.
3. **У класса расчёта обязательно есть `Initialize()`**, вызывается ровно один раз (повтор — ассерт).
4. **Обёртка — из семейства** `PropertiesModule/Models/wrappers.h` [проверено]:
   - `StateChainedContainerCalculator<Model>` — результат-контейнер (двойной буфер);
   - `StateChainedValueCalculator<LocalPropertyX>` — `Result->Value` типа `StateParameterProperty<LocalPropertyOptional<X>>`;
   - `StateChainedComplexCalculator<Result>` — несколько выходов через `RegisterOutput`.
5. **Не передавать параметр `enable` и не ставить `Enabled = true` вручную** (в т. ч. в `Initialize`). `Enabled` включается захватом выхода (`StateCalculatorSwitcher`). Не использовать `StateCalculator::InitializeByDoubleBuffer(…, bool enable = true, …)` — он сам ставит `Enabled` [проверено: сигнатура в `stateproperty.h`].
6. **Захват выхода обязателен:** на верхнем уровне `auto capture = Output->Capture();` и держать, пока нужен расчёт; внутри цепочки — передать `Output` аргументом в `Initialize` следующего калькулятора (захватит сам). Порядок «захват до/после `Initialize`» не важен.
7. Ручная сборка `StateCalculator` (`SetCalculator`, `Connect` к `LocalProperty`, свои preparator/releaser) — legacy, в новом коде запрещена.

В рантайме:

- До первой инициализации `params->Field = v` пишет тихо, после — через `InputValue` с проверкой главного потока.
- Изменение входа → прерывание текущего расчёта, `Valid = false`, перезапуск при валидных зависимостях.
- На время расчёта входы заблокированы (`Lock`/`Unlock` на главном потоке); снимок `GetImmutable()` в это время не обновляется.
- Ошибка расчёта — бросать `Exception` / `throwException(errorId)`; ловится только `const Exception&` и уходит в цепочку ошибок `Result`.
- Отмена — `GetInterruptor()`, в цикле проверять `IsInterrupted()`.

Пример (**не скомпилирован**, исходник в репозитории не хранится; папка `PropertiesModule/Claude outputs` удалена):

```cpp
class PrimeCounterInput : public StateParameters
{
public:
    PrimeCounterInput() : Limit(this) {}
    StateParameterProperty<LocalPropertyInt> Limit;
};

class PrimeCounter
{
    using Calculator = StateChainedValueCalculator<LocalPropertyInt>;
public:
    using value_type = Calculator::value_type;
    using OutputParameter = Calculator::ResultParameter;

    PrimeCounter() { Output = m_calculator.Result; }
    void Initialize(); // ровно один раз

    SharedPointerInitialized<PrimeCounterInput> Input;
    SP<OutputParameter> Output;

private:
    static std::optional<value_type> countPrimes(value_type limit, const Interruptor& interruptor);
    Calculator m_calculator;
};

void PrimeCounter::Initialize()
{
    auto interruptor = m_calculator.GetInterruptor();
    m_calculator.Initialize(CDL, [interruptor](const SP<PrimeCounterInput>& input) {
        return countPrimes(input->Limit.GetImmutable(), interruptor); // рабочий поток: только снимок
    }, Input);
}
// Главный поток: counter.Initialize(); auto capture = counter.Output->Capture(); counter.Input->Limit = 1000;
```

## Потоки

- **Все `LocalProperty`, `StateParameters`, `Property` и UI создаются, меняются и читаются только на главном потоке.** `THREAD_ASSERT_IS_MAIN` — `Q_ASSERT`, в release пустой; нарушение в release ничем не ловится.
- Рабочий поток читает только снимки: `Field.GetImmutable()`, `GetImmutableData()` [проверено: есть в `stateproperty.h`]. `GetInputData` и `operator T&` — только на главном.
- В лямбде расчёта не захватывать `this` и не трогать `InputValue`.
- В главный поток: `ThreadsBase::DoMain` / `DoMainWithResult` (асинхронно), `ThreadHandlerMain` (синхронно, если уже на главном).
- `DoMainAwait` из рабочего потока блокирует его до выполнения на главном — взаимная блокировка, если главный ждёт этот поток [вывод].
- **На главном потоке не ждать** (`Promise::Wait`, `FutureResult::Wait`): они крутят `ProcessMainEvents`, обработчики вызываются посреди вашего кода. Использовать `Then` или подписки.
- Логгер пишет через `DoMain` — при падении главного потока последние строки теряются.

## Подписки и время жизни

- Свойство — поле объекта-модели по значению.
- **`DispatcherConnection` сам не отключается.** `.MakeSafe(m_connections)` в конце `Connect`, `ConnectAndCall`, `ConnectFrom`, `ConnectBoth`, `ConnectCombined` ставится не всегда, а только если обработчик потенциально может быть вызван для уже уничтоженного объекта: захвачен `this` или указатели на объекты, которые могут умереть раньше источника, либо целевое свойство `ConnectFrom` принадлежит такому объекту [проверено: `MakeSafe` в `SharedModule/dispatcher.h`].
  - Источник и всё, к чему обращается обработчик (включая целевое свойство `ConnectFrom`), принадлежат одному объекту и умирают вместе — `MakeSafe` не нужен (например, подписка на свойство кнопки, которая меняет ту же кнопку).
  - Глобальный источник (синглтон, `SharedSettings`) и обработчик захватывает `this` — `MakeSafe` нужен.
  - Внешние объекты, которые живут дольше своего: глобальные и общие настройки, `MeasurementDistance`, общие свойства менеджера, другой вид и свойства другого вида, драуэр, кнопка в чужой панели. Связь с ними, которая пишет в свой объект или захватывает `this`, получает `MakeSafe` к контейнеру своего объекта сразу, в той же строке [проверено: такие связи в `WPSClient/Ui/Bars/wpscvisualizationfooter.cpp`].
  - Признак пропущенного `MakeSafe`: связь хранит лямбду и ссылки на оба конца; когда один конец умер, следующее изменение второго вызывает код на удалённом объекте — падение в `LocalProperty::SetValue` или в `operator=` внутри лямбды связи [не проверено: по разбору падения из практики].
  - Если источник уничтожается раньше контейнера, подписка исчезает вместе с ним, поздний `Disconnect` из `MakeSafe` безопасен.
- `m_connections` — член `DispatcherConnectionsSafe` (`= QVector<DispatcherConnectionSafePtr>`) [проверено], объявленный **после** всех свойств, на которые ссылаются лямбды (уничтожится раньше них).
- Хранить соединения рядом с объектом — `WithDispatcherConnectionsSafe` (`DECLARE_WITH_FIELD(DispatcherConnectionsSafe, Connections)` в `dispatcher.h`) [проверено].
- `obj.OnChanged += { this, handler }` / `-= this` — снимать явно; повтор ключа — ассерт.
- `CDL` (= `CONNECTION_DEBUG_LOCATION`) передавать во все подписки.
- Лямбды в `DoMain` / `DelayedCall`, захватившие `this`, могут выполниться после удаления объекта — захватывать `SharedPointer` на данные или проверять guard.
- `SharedPointer::Cast<T2>()` — `reinterpret_cast` между умными указателями (UB) [проверено: `smartpointersadapters.h`]. В новом коде — `std::static_pointer_cast`.
- `Dispatcher::SetAutoThreadSafe(mutex)` при неудачном `tryLock` идёт без блокировки — использовать `SetAutoThreadSafe()` без аргумента [проверено: обе перегрузки в `dispatcher.h`; поведение — не проверено повторно].

- `m_connections` живёт вместе с объектом и не сбрасывается в методах. `m_connections.Clear()` запрещён; нарушение — ошибка ревью.
- Методы, которые переподключают связи при каждом вызове (`SetTarget(...)`, `SetSource(...)` и подобные), используют отдельный контейнер, названный по данным, например `m_targetConnections`: в начале метода `m_targetConnections.Clear();`, затем все связи метода с `.MakeSafe(m_targetConnections)`. Связи таких методов в `m_connections` не писать: он накопит устаревшие связи при каждом вызове.
- Копия вида (отстыкованное окно, копия из DD Space) держит все свои связи в собственном контейнере. Закрытие копии рвёт только её связи; базовый вид и общие объекты не трогаются.
- Пример (`WPSCVisualizationFooter` в wps-client): футор вида создавал bool через `LocalPropertyBool` и связывал его с общей настройкой (`ShowOffsets`) через `ConnectBoth` без `MakeSafe`. Закрытие копии (копия 2D View) удаляло футор и его bool, а глобальная настройка продолжала вызывать связь и падала. Исправление: связь получила `MakeSafe(m_connections)` футора, список bool'ов передан драуэру через `SharedPointer`, остальные связи футора с внешними объектами получили `MakeSafe` сразу.

- Коннекторы (`LocalPropertiesCheckBoxConnector`, `LocalPropertiesPushButtonConnector`, `MeasurementWidgetConnectors`, `LocalPropertiesWidgetConnectorsContainer`) разрывают свои связи сами при уничтожении. `MakeSafe` для них не нужен; важно только, чтобы контейнер коннекторов не жил дольше связанных виджетов.
- Члены одного класса уничтожаются в обратном порядке объявления (это гарантирует C++), поэтому связь, объявленная в `m_connections` после свойств, снимается раньше них — на это можно опираться. Дочерние `QWidget` удаляет Qt, их порядок не гарантирован, поэтому связи с дочерними виджетами держать через `MakeSafe`, а не на порядке удаления.
- Если свойство принадлежит одному объекту, а на него ссылается другой, владение общее: `SharedPointer`, и держатель передаёт список другому объекту (пример: драуэр регистрирует чекбоксы футора через `RegisterCheckbox`, а футор передаёт ему свои bool'ы через `KeepPropertiesAlive`) [проверено: `wpscvisualizationfooter.cpp`].

## Сериализация

- Контракт: `template<class Buffer> void Serialize(Buffer& buffer)` с `buffer << buffer.Sect("Имя", поле)` и `buffer << buffer.Attr("имя", поле)` — один метод на чтение и запись.
- Двоичный формат (`SerializeToArray` / `DeSerializeFromArray`) позиционный, имена игнорируются.
- XML при чтении тоже позиционный: **порядок полей при записи и чтении должен совпадать**. Отсутствующий атрибут бросает `Exception("Parsing failed")`.
- Режимы `SerializationMode_*`: `InvokeProperties` — чтение через `prop = value` (валидатор, уведомление), без него пишется тихо; `MinMaxProperties` — до значения пишутся `m_max`/`m_min`.
- Версии — `SerializeToArrayVersioned` / `DeSerializeFromArrayVersioned` (`SharedModule/Serialization/qserializer.h`) [проверено]. Хеш-сумма не проверяется.
- `DeSerializeFromArray` не проверяет ошибки потока и размеры коллекций — не читать им непроверенные внешние файлы.

## EditingObject и SharedSettings

- `EditingObject::SetActive(false)` с несохранёнными изменениями без обработчика `OnLeavingDirtyEditingMode` делает **Discard**. Если молча откатывать нельзя — задать обработчик.
- `Save()` выходит из режима редактирования и сразу входит снова.
- `TEditingObjectComponent(object, testChanged = true)` — `testChanged` по умолчанию `true` [проверено: `editingobject.h`]; при нём на каждом `OnChanged` объект сериализуется в XML во временный файл — дорого. С `false` любое `OnChanged` просто ставит `SetDirty(true)` [проверено].
- У `SharedSettings::OnChanged` один источник — `NetworkSettings`.
- Экземпляр — `SharedSettings::GetInstance()` (`Singletone<SharedSettings>`) [проверено: использование в `GraphicsToolsModule/gtcamera.cpp`]. `SharedSettings::Serialize` пишет только `NetworkSettings`, `PathSettings`, `SaveLoadSettings`, `LanguageSettings`, `ControlSettings`; `MetricSettings` и `StyleSettings` не сериализуются [проверено: `sharedsettings.h`].

## UI: реактивные таблицы

- Не писать свой `QAbstractTableModel` и не подписываться вручную на сигналы обёртки, если есть DevLib-класс. Для реактивных таблиц — TableWrapper (`TModelsTableWrapper`, `DECLARE_MODEL_BY_TYPE`). `LocalProperty<QVector<…>>` для таблиц не использовать.
- Строка таблицы — структура с полями. В заголовке `DECLARE_MODEL_BY_TYPE(Row)` даёт `RowContainer` и `RowModel` (`PropertiesModule/Models/wrappers.h`) [проверено].
- Калькулятор: `StateChainedContainerCalculator<RowModel>`, `Initialize(CDL, lambda, Input)`; лямбда читает только `GetImmutable()` входа и возвращает `RowContainer`.
- `calc.Result->Capture()` обязателен; объект захвата объявить раньше окна, чтобы уничтожался позже.
- Вид: `class TableView : public TViewModelsTableBase<RowModel>` (`WidgetsModule/Models/modelstablebase.h`) [проверено].
  - Колонки в конструкторе: `auto builder = Builder(); builder.AddInt(col, TR(...), &Row::V1);` Типизированные `AddString`, `AddName`, `AddColor`, `AddDouble`, `AddInt`, `AddBool` — из макроса `ViewModelsColumnComponentsBuilder_DECLARE_TYPE` (`WidgetsModule/Models/viewmodelsdefaultcomponents.h`) [проверено]. Не писать `AddColumn` с двумя лямбдами, если есть типизированный метод.
  - Заголовки строк: `m_roleVerticalHeaderDataHandlers.insert(Qt::DisplayRole, …)` [проверено: член `ViewModelsTableBase`].
  - Данные: `viewModel->SetData(calc.Result->GetInputData()->GetData())` из главного потока.
- **До создания любой модели создать `IconsManager`** (ровно один экземпляр; без него модель падает в `IconsManager::GetInstance()`) [проверено: `GetInstance` в `WidgetsModule/Utils/iconsmanager.h`].

## UI: окна и WidgetBuilder

- Qt-интерфейс — через `.ui`-форму (Qt Designer, `FORMS += файл.ui`, `ui_*.h` генерирует uic). Интерфейс в коде — `WidgetBuilder` (`WidgetsModule/Utils/widgetbuilder.h`) [проверено].
- `WidgetBuilder(QWidget* parent, qint32 margin = 9)` [проверено] удаляет существующий layout родителя вместе с виджетами — вызывать на пустом окне.
- **Порядок:** `WidgetBuilder` создаётся первым на пустом окне; все вложенные виджеты (таблицы, кнопки, layout) создаются после него и добавляются через builder (`AddWidget`, `AddButton`, `StartLayout`). Виджеты, созданные до builder, в его layout не попадают и не управляются им.
- Методы: `StartLayout(params, handler)`, `StartSplitter`, `StartLabeledLayout`, `AddWidget(QWidget*)`, `AddButton(label)` (→ `WidgetAbstractButtonWrapper`, у него `SetOnClicked`), `AddLabel`, `AddLineEdit`, `AddCheckBox`, `AddCombobox`, `AddSpinBox`, `AddDoubleSpinBox`, `AddSpacer(w, h)` [проверено по индексу].
- **Настройка — в лямбде builder.** Виджет, созданный `Add*`, настраивается внутри той же лямбды `StartLayout`/`StartSplitter`, сразу после создания: методами обёртки (`SetControl`, `SetIcon`, `SetMenu` и т. д.), а то, чего у обёртки нет (`setObjectName`, `setAttribute`), — через `->` обёртки. После лямбды созданные в ней виджеты не донастраиваются.
- Виджет, нужный вне конструктора, хранится в члене-обёртке (`WidgetPushButtonWrapper m_btn;`, в списке инициализации `m_btn(nullptr)` — конструктора без аргументов у обёртки нет), присваивается в лямбде (`m_btn = button;`) и дальше используется через методы обёртки, без `qobject_cast` к конкретному классу [проверено: `wpscvisualizationfooter`].
- Отступы: корневой layout уже даёт отступ 9; вложенный `StartLayout` добавит второй — для него `WidgetBuilderLayoutParams(...).SetMargin(0)` [проверено: `SetMargin`] или не вызывать `StartLayout`.
- Таблица в окно — `AddWidget(view)`, где `view` — `new WidgetsTableViewBase()` с `setModel`.
- Обход детей: `WidgetWrapper::ForeachChildWidget(handler, recursive = true)` (`WidgetsModule/Utils/widgethelpers.h`, сигнатура проверена). При `recursive = false` обходятся только прямые дети. Обход выполняется в момент вызова [вывод]: виджеты, добавленные после него, в обработчик не попадают.

## Сборка теста

`.pro` для теста DevLib, порядок строк важен:

1. `QT += core gui widgets concurrent printsupport network xml svg gui-private`
2. `DEFINES += STATIC_LINK` (без него `_Export` → dllimport: C2491/LNK2019)
3. `CONFIG += console c++17`
4. `DEV_LIB_PATH = <путь к DevLib>` (абсолютный путь к папке DevLib на этой машине)
5. `include($$DEV_LIB_PATH/Global.pri)` — до модулей (даёт `includeAll`, `setBuildDirectory`)
6. `include` модулей: SharedModule, PropertiesModule, UnitsModule, WidgetsModule (их `.pri`)
7. `INCLUDEPATH += $$DEV_LIB_PATH`
8. `SOURCES += main.cpp`

- Окружение: `win32-msvc`, VS 2022, Qt 5.15.2 msvc2019_64 [не проверено повторно].
- `includeAll(true)` подтягивает все `.cpp` папки — чужие примеры внутри папки ломают линковку (LNK1181).
- Часть кода собирается только MSVC в режиме `/permissive` (см. «Известные ошибки»).

## Стиль кода

- Проверка указателя всегда с явным сравнением: `if(ptr != nullptr) { ... }` или `if(ptr == nullptr) { ... }`. Запись `if(ptr)` и `if(!ptr)` не использовать.
- Для `std::function`, `SharedPointer`, `std::optional` и других не-bool типов проверка тоже явная: `handler != nullptr`, `value.has_value()`.
- Условие без сравнения (`if(x)`) допускается только если `x` — переменная типа `bool` или вызов функции/метода, возвращающий `bool`, например `if(isVisible) { ... }` и `if(map.contains(key)) { ... }`.
- Только что созданный объект (`new`, `Add*` у `WidgetBuilder`, результат конструктора) на `nullptr` не проверяется — ни через `if`, ни через `Q_ASSERT`.
- Инвариант, нарушение которого означает ошибку программиста (аргумент, который не может быть `nullptr`; обязательное состояние), проверяется `Q_ASSERT(условие)`, а не `if` с ранним выходом.
- Результат `qobject_cast` и других приведений, которые могут вернуть `nullptr`, разыменовывается только внутри ветки `if(ptr != nullptr) { ... }`. Если неудача приведения — ошибка программиста, у ветки есть `else { Q_ASSERT(false); }`: в debug ошибка видна, в release нет разыменования `nullptr`. Если неудача допустима, `else` обрабатывает её или отсутствует. Разыменовывать результат вне такой ветки нельзя.

## Известные ошибки

Места в коде DevLib, которые нельзя использовать как есть. Место находить через `codemap.py find`.

| Где | Что | Статус |
|---|---|---|
| `LocalPropertyDate::operator+=` (`localproperty.h`) | Вычитает вместо сложения | Не исправлено [проверено 2026-11-02] |
| `LocalPropertyLimitedDecimal::FlagAdd/FlagRemove` (`localproperty.h`) | Раньше были перепутаны | **Исправлено** в рабочей копии [проверено 2026-11-02] |
| `SharedPointer::Cast` (`smartpointersadapters.h`) | UB через `reinterpret_cast` | Не исправлено [проверено]. Использовать `std::static_pointer_cast` |
| `ScopedPointer` (`smartpointersadapters.h`) | Игнорирует собственный deleter | [не проверено повторно] |
| `Dispatcher::SetAutoThreadSafe(mutex)` (`dispatcher.h`) | При неудаче `tryLock` идёт без блокировки | [не проверено повторно]. Брать перегрузку без аргумента |
| `Logger` (`SharedModule/Logger/logger.cpp`) | `fprintf(stderr, msg)` — текст как строка формата; `checkDate` пересоздаёт файл в потоке вызывающего | [не проверено повторно]. Не логировать сырой `%` |
| `interruptor.h` | Функции без `inline` в заголовке — multiple definition | [не проверено повторно] |
| `Ui/localpropertieswidgetconnector.h` | Опечатка `spin` вместо `spinBox` в шаблоне — не соберётся при инстанциации | [не проверено повторно] |
| `propertiesserializer.h` | `vector.Invoke()` у `QVector` — не соберётся при инстанциации | [не проверено повторно] |
| `PropertiesScope::GetProperty<T>` (`propertiesscope.h`) | Пустое тело (устаревшая система) | Не исправлено [проверено 2026-11-16] |
| `StateParametersImmutableDataCreate` (`stateproperty.h`), первая перегрузка | Обращается к несуществующему `InputValue` | [не проверено повторно]. Не использовать |
| `sharedsettings.cpp` | `Q_ASSERT(dir.removeRecursively())` — в release не удаляет; `TimeFromVariant` путает 12 AM/PM | [не проверено повторно] |
| `localproperty.h`, `Models/wrappers.h` | Конструкции, которые собирает только MSVC `/permissive` | [не проверено повторно] |
| Коммутаторы `ConnectFromDispatchers` | Объявлен, не определён — ошибка линковки | [не проверено повторно] |

## Чеклист перед коммитом

1. Классы и методы найдены через `codemap.py find` или grep, а не угаданы; сигнатуры сверены с заголовком.
2. Новых `Property` / `TProperty` нет; для INI/дерева — `PropertyFromLocalProperty`, контейнер адаптеров живёт достаточно долго.
3. `.MakeSafe(...)` стоит у каждой подписки с `CDL`, обработчик которой может быть вызван после уничтожения объекта (захвачен `this` или указатели, целевое свойство `ConnectFrom` живёт меньше источника, глобальный источник с захватом `this`); у подписок, где источник и всё, к чему обращается обработчик, принадлежат одному объекту, его нет; `Subscribe` и `+=` — только где подписчик живёт не меньше источника.
4. `DispatcherConnectionsSafe` объявлен после свойств, на которые ссылаются лямбды.
5. Расчёт: `StateParameters` + `StateChained*Calculator`; входы и выходы публичные; есть `Initialize()`; нет `enable` и ручного `Enabled`; нет `SetCalculator` и ручных `Connect`.
6. Лямбда расчёта читает только `GetImmutable()`, не захватывает `this`, не трогает `InputValue` / `GetInputData`.
7. Ошибки расчёта — `Exception`; отмена учитывает `Interruptor`; выход захвачен (`Capture()` или аргумент следующего калькулятора).
8. Изменения свойств и UI — только на главном потоке; нет `Wait` на главном; нет `DoMainAwait`, где главный может ждать этот поток.
9. В `Serialize` порядок полей одинаков для записи и чтения; режим (`InvokeProperties`, `MinMaxProperties`) выбран осознанно.
10. Таблицы: нет самописного `QAbstractTableModel` при наличии `TViewModelsTableBase`; нет `AddColumn` с лямбдами там, где есть `Add<Type>`; `IconsManager` создан до моделей; объект захвата живёт до конца окна.
11. Не используются места из «Известных ошибок»; `Cast` заменён на `std::static_pointer_cast`.
12. Для `EditingObject` есть `OnLeavingDirtyEditingMode`, если молча откатывать нельзя.
13. UI: `WidgetBuilder` создан до вложенных виджетов; окно пустое на момент конструктора; `StartLayout` вложенный (с `SetMargin(0)`), а не корневой; виджеты настроены внутри лямбды builder; кнопка, нужная вне конструктора, хранится как обёртка (`WidgetPushButtonWrapper`), а не как `QPushButton*` из `qobject_cast`.
14. Указатели и обработчики сравниваются с `nullptr` явно; `if(x)` без сравнения — только для `bool` (см. «Стиль кода»). Только что созданное (`new`, `Add*` у builder, конструктор) на `nullptr` не проверяется; инварианты — `Q_ASSERT(условие)`; результат `qobject_cast` разыменовывается только внутри `if(ptr != nullptr) { ... }`, а если неудача приведения — ошибка программиста, ветка `else { Q_ASSERT(false); }`.
15. Связи: `m_connections.Clear()` и сбросов `m_connections` в методах нет; методы, которые переподключают связи, используют свой контейнер с `Clear()` в начале; нет связей, которые держат ссылку на локальную переменную функции или на свойство другого вида без общего владения; у копии вида связи лежат в контейнере копии, а не базового вида.

## Правка .ui-файлов (Qt Designer)

- **Главное правило: один виджет на один `<item>`.** В layout-элементе каждый `<item>` содержит ровно один дочерний элемент: виджет, спейсер или вложенный layout. Если два `<widget>` стоят в одном `<item>`, uic молча не генерирует поля для части виджетов. Каждый виджет — в своём `<item>`.
- Симптомы: «X is not a member of Ui::Class», следом «__this is not a member» или «no appropriate default constructor». Первая проверка — сам `.ui`: через `xml.etree.ElementTree` перебрать `item` и посмотреть `findall('widget')`. Каскадные ошибки в `.cpp` исчезают, когда поле снова генерируется.
- Проверка после каждой правки: XML разбирается без ошибок; каждый `<item>` содержит один виджет или спейсер; окончания строк сохранены (проверить `grep -c $'\r$'` против `wc -l`, у многих файлов CRLF); новое имя встречается ровно один раз.
- Патчи: скрипт с dry-run по умолчанию, `--apply` для записи, `assert count == 1` на каждом якоре. Вставка относительно границ найденного блока, а не по одной строке; проверить, что закрывающий тег стоит там, где ожидается.

## Git

Git в репозиториях ведёт только пользователь. Ассистент не выполняет команды, которые меняют состояние git, если пользователь не попросил этого явно в том же сообщении.

- Разрешено (только чтение): `git status`, `git log`, `git diff`, `git show`, `git branch --list`, `git rev-parse`, `git blame`.
- Запрещено без явной просьбы: создавать, переименовывать, удалять или переключать ветки (`git branch`, `git checkout`, `git switch`, `git worktree`); коммитить, amend, rebase, reset, revert, stash, cherry-pick, merge; push, pull, fetch с записью, менять remotes или сабмодули; удалять файлы `.git` (в том числе lock-файлы); запускать `git gc`, `git clean`, `git restore`; откатывать или перезаписывать незакоммиченные изменения.
- Если нужна git-операция, не выполнять её. Описать точную команду, причину и последствия, и попросить пользователя выполнить её или явно подтвердить.
- Правки исходников в рабочей копии допустимы, когда пользователь их попросил. Все изменения оставлять незакоммиченными и перечислять изменённые файлы.
- Если git сообщает о lock-файле или маркере сбоя, не удалять его. Показать сообщение и оставить решение пользователю.

## Стили (QSS)

- В корне `StylesModule/` лежат файлы тем (`aispaces_dark.qss`, `aispaces_light.qss`, `tmp_dark.qss`), `main.qrc`, `StylesModule.pri` и папки `aispaces/` (`palette.qss`, `style.qss`, `res/`) и `style/`. Файлы `*.bak_dockimport` в корне и `style/dock.qss.bak_qss2` — резервные копии от правок, не часть модуля [проверено: ls 2026-11-16].
- Общие QSS (стили компонентов, например `ads.qss`, `style.qss`, `widgets.qss`, `dock.qss`) лежат в папке `StylesModule/style/`, а иконки — в `style/res/`.
- Ресурсы подключаются через `main.qrc` с путём `style/<имя>`; в темах общий файл импортируется как `@import url("style/<имя>");`.
- В общих QSS нет символа `@` вне токенов тем: `QtQSSReader` подставляет любой `@c…` из импортируемого файла.
- В QSS-файлах (включая `libs/DevLib/StylesModule/**/*.qss` и темы `aispaces_*.qss`) комментарии `/* ... */` не добавлять; пояснения выносить в коммит-сообщение или описание задачи.
