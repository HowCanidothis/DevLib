# Соглашения по коду DevLib

Единый источник правил для кода, который использует DevLib. Скиллы и промпты ссылаются сюда; при расхождении прав этот файл.

Как читать: целиком не нужно. Один раздел — `python3 codemap.py section РАЗДЕЛ --show --file CONVENTIONS.md`. Где лежит класс — `python3 codemap.py find ИМЯ`. Номера строк здесь намеренно не указаны: они устаревают; место находится через `find`.

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

## Поиск кода

1. `python3 codemap.py find ИМЯ` — где объявлено, реализовано, кем используется, что сказано в CODEMAP.md.
2. Если `NOT FOUND` — `grep -rn -w "ИМЯ" --include=*.h --include=*.hpp --include=*.cpp .`
3. Обзор модуля — `python3 codemap.py section МОДУЛЬ --show` (раздел CODEMAP.md).
4. Только потом читать найденный заголовок; сигнатуры, на которые опирается новый код, сверять с ним.

- Вывод «такого класса нет» делается только после шагов 1–2 по рабочей копии DevLib, а не по частичным копиям файлов.
- `ui_X.h` в исходниках нет: его генерирует uic из `X.ui` в той же папке.
- Типы из макросов (`DECLARE_MODEL_BY_TYPE`, `DECLARE_WITH_FIELD`, `SHARED_DECLARE_EVENT_FOR_ENUM` и др.) в индексе видны только как `defines:`; ищутся grep'ом по имени макроса.

## Выбор механизма

| Задача | Что брать | Статус |
|---|---|---|
| Состояние объекта, настройки, модель UI | `LocalProperty<T>` и наследники (`PropertiesModule/localproperty.h`) | **Основной. Новый код только на нём.** |
| Фоновый расчёт по входам | `StateParameters` + `StateChained*Calculator` (`PropertiesModule/stateproperty.h`, `PropertiesModule/Models/wrappers.h`) | Единственный разрешённый способ |
| Доступ по строковому пути: INI, дерево свойств | `Property`, `PropertiesSystem`, `PropertyPromise` (`property.h`, `propertiessystem.h`) | **Устаревший.** Новых `Property`/`TProperty`/`IntProperty` не создавать; существующий код переводится на `LocalProperty` |

- Нужен INI или дерево свойств — мост `PropertyFromLocalProperty::Create(name, localProperty)` (`localproperty.h`) [проверено]. Адаптер регистрируется в текущем scope в конструкторе `Property`, а `~Property()` регистрацию не снимает — контейнер адаптеров должен жить не меньше scope [не проверено повторно].
- `PropertiesModel` и Save/Load работают только через `Property*` в scope, поэтому прямая регистрация `LocalProperty` не работает — только через мост [не проверено повторно].
- `StateProperty` — это `LocalPropertyBool` с `SetState`, к реестру не относится.

## LocalProperty

- `SetValue(v)`: валидатор → min/max → сравнение → запись → `Invoke`; по умолчанию синхронно.
- Тихие пути без уведомления: `SetSilent`, `EditSilent`, `SetSilentWithValidators`. После серии тихих правок вызвать `Invoke()` самому.
- `LocalPropertySet/Hash/Vector`: `Insert/Remove/Clear/operator=` обходят валидатор; `operator=` уведомляет даже при равных значениях (кроме «оба пустые»).
- Подписка:
  - `Subscribe(fn)` / `SetAndSubscribe(fn)` **отменить нельзя** [проверено]. Только если подписчик живёт не меньше свойства.
  - Иначе — `Connect(CDL, h)`, `ConnectAndCall(CDL, h)`, `ConnectAction(CDL, a)` и `.MakeSafe(...)` только если обработчик может быть вызван для уничтоженного объекта (см. «Подписки и время жизни»).
- `ConnectFrom(CDL, h, a, b…)` и `ConnectProperties` захватывают источники **по ссылке** — источники должны пережить соединение.
- `ConnectBoth` — двусторонняя связь, рекурсия гасится флагом.
- Типы:
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

Пример (по `PropertiesModule/Claude outputs/prime_counter_chained.*`, **не скомпилирован**):

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
- **`DispatcherConnection` сам не отключается.** `.MakeSafe(m_connections)` в конце `Connect`, `ConnectAndCall`, `ConnectFrom`, `ConnectCombined` ставится не всегда, а только если обработчик потенциально может быть вызван для уже уничтоженного объекта: захвачен `this` или указатели на объекты, которые могут умереть раньше источника, либо целевое свойство `ConnectFrom` принадлежит такому объекту [проверено: `MakeSafe` в `SharedModule/dispatcher.h`].
  - Источник и всё, к чему обращается обработчик (включая целевое свойство `ConnectFrom`), принадлежат одному объекту и умирают вместе — `MakeSafe` не нужен (например, подписка на свойство кнопки, которая меняет ту же кнопку).
  - Глобальный источник (синглтон, `SharedSettings`) и обработчик захватывает `this` — `MakeSafe` нужен.
  - Если источник уничтожается раньше контейнера, подписка исчезает вместе с ним, поздний `Disconnect` из `MakeSafe` безопасен.
- `m_connections` — член `DispatcherConnectionsSafe` (`= QVector<DispatcherConnectionSafePtr>`) [проверено], объявленный **после** всех свойств, на которые ссылаются лямбды (уничтожится раньше них).
- Хранить соединения рядом с объектом — `WithDispatcherConnectionsSafe` (`DECLARE_WITH_FIELD(DispatcherConnectionsSafe, Connections)` в `dispatcher.h`) [проверено].
- `obj.OnChanged += { this, handler }` / `-= this` — снимать явно; повтор ключа — ассерт.
- `CDL` (= `CONNECTION_DEBUG_LOCATION`) передавать во все подписки.
- Лямбды в `DoMain` / `DelayedCall`, захватившие `this`, могут выполниться после удаления объекта — захватывать `SharedPointer` на данные или проверять guard.
- `SharedPointer::Cast<T2>()` — `reinterpret_cast` между умными указателями (UB) [проверено: `smartpointersadapters.h`]. В новом коде — `std::static_pointer_cast`.
- `Dispatcher::SetAutoThreadSafe(mutex)` при неудачном `tryLock` идёт без блокировки — использовать `SetAutoThreadSafe()` без аргумента [проверено: обе перегрузки в `dispatcher.h`; поведение — не проверено повторно].

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
- `TEditingObjectComponent` с `testChanged=true` на каждом `OnChanged` сериализует объект в XML во временный файл — дорого.
- У `SharedSettings::OnChanged` один источник — `NetworkSettings`.

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
| `PropertiesScope::GetProperty<T>` (`propertiesscope.h`) | Пустое тело (устаревшая система) | [не проверено повторно] |
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
