---
name: "wps-devlib-ui-patterns"
description: "Use when writing wps-client C++ code with DevLib reactive tables, table views, WidgetBuilder windows or .pro files for a DevLib test. Not for property or calculator internals."
---

# Шаблоны DevLib: реактивная таблица, UI и .pro

Правила живут в `CONVENTIONS.md` в корне `D:\Work\wps-client\libs\DevLib` — единственный источник. Здесь их не дублировать; при расхождении прав `CONVENTIONS.md`.

В Cowork — через `mcp__remote-devices__device_bash` с префиксом `cd "$HOME/mnt/DevLib" && ` (нет доступа — см. `wps-devlib-folder-access`). Файлы в `/mnt/user-data/uploads` — неполная выборка, по ним нельзя делать вывод «класса нет».

## Порядок

1. Классы и методы: `python3 codemap.py find ИМЯ`; если NOT FOUND — `grep -rn -w "ИМЯ" --include=*.h --include=*.hpp --include=*.cpp .`; потом читать найденный заголовок и сверять сигнатуры.
2. Разделы соглашений (`python3 codemap.py section "РАЗДЕЛ" --show --file CONVENTIONS.md`):
   - реактивная таблица, `DECLARE_MODEL_BY_TYPE`, `TViewModelsTableBase`, колонки `Add<Type>`, `IconsManager` → «UI: реактивные таблицы» (плюс «Вычисления» для калькулятора таблицы);
   - окна, `.ui`-формы, `WidgetBuilder`, отступы → «UI: окна и WidgetBuilder»;
   - `.pro` для теста DevLib, порядок строк, `STATIC_LINK`, `includeAll` → «Сборка теста»;
   - всегда — «Известные ошибки» и «Чеклист перед коммитом».
3. После кода пройти по чеклисту: выполнен / не относится / нарушен (где).

Если DevLib недоступна, минимум для напоминания: не писать свой `QAbstractTableModel` при наличии `TViewModelsTableBase`; до моделей создать `IconsManager`; держать `Capture()` выхода расчёта; `WidgetBuilder` вызывать на пустом окне; в `.pro` — `DEFINES += STATIC_LINK` и `Global.pri` до модулей. Перед коммитом всё равно свериться с `CONVENTIONS.md`.