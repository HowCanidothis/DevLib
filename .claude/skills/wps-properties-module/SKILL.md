---
name: "wps-properties-module"
description: "Use for wps-client C++/Qt code that uses PropertiesModule or SharedModule: LocalProperty, StateParameters calculators, Dispatcher, threads, serialization."
---

# PropertiesModule и SharedModule (wps-client)

Правила работы с PropertiesModule и SharedModule живут в одном месте — `CONVENTIONS.md` в корне `D:\Work\wps-client\libs\DevLib`. Здесь их не дублировать: копии расходятся. При расхождении прав `CONVENTIONS.md`.

## Как пользоваться

В Cowork — через `mcp__remote-devices__device_bash`, команды с префиксом `cd "$HOME/mnt/DevLib" && ` (нет доступа к DevLib — см. скилл `wps-devlib-folder-access`).

1. Прочитай разделы по теме: `python3 codemap.py section РАЗДЕЛ --show --file CONVENTIONS.md`
   - какой механизм брать (LocalProperty / Property / мост PropertyFromLocalProperty) → «Выбор механизма»;
   - SetValue, тихие пути, подписки, типы свойств, коммутаторы → «LocalProperty»;
   - StateParameters, StateChained*Calculator, Initialize, захват выхода, исключения, отмена, пример PrimeCounter → «Вычисления»;
   - главный поток, снимки, DoMain, Wait → «Потоки»;
   - MakeSafe, DispatcherConnectionsSafe, CDL, время жизни, Cast → «Подписки и время жизни»;
   - Serialize, Sect/Attr, режимы, версии → «Сериализация»;
   - EditingObject, SharedSettings → «EditingObject и SharedSettings».
2. Всегда: «Известные ошибки» (какие места нельзя использовать) и «Чеклист перед коммитом».
3. Классы и методы ищи `python3 codemap.py find ИМЯ`, сигнатуры сверяй с заголовком. Номера строк в соглашениях намеренно не указаны — место находит `find`.

## Кратко (для случаев, когда DevLib недоступна)

Новый код — только `LocalProperty`; расчёты — только `StateParameters` + `StateChained*Calculator` с публичными входами/выходами, `Initialize()` и без ручного `Enabled`; всё — на главном потоке; каждая подписка — `.MakeSafe(...)`. Это лишь напоминание: перед написанием кода попроси доступ к DevLib и прочитай `CONVENTIONS.md`.