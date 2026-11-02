---
name: wps-properties-rules
description: Use when writing or reviewing wps-client code that uses PropertiesModule or SharedModule: which property type to use, how to write calculators, and which patterns are forbidden.
---

# Правила PropertiesModule и SharedModule (wps-client)

Правила живут в `libs/DevLib/CONVENTIONS.md` (`D:\Work\wps-client\libs\DevLib`) — единственный источник. Здесь их не дублировать; при расхождении прав `CONVENTIONS.md`.

В Cowork — через `mcp__remote-devices__device_bash` с префиксом `cd "$HOME/mnt/wps-client" && ` (если подключена только DevLib — `cd "$HOME/mnt/DevLib" && ` и пути без `libs/DevLib/`; нет доступа — см. `codemap-search`):

- какой тип свойства брать → `python3 codemap.py section "Выбор механизма" --show --file libs/DevLib/CONVENTIONS.md`;
- как писать калькуляторы → `section Вычисления --show --file libs/DevLib/CONVENTIONS.md`;
- что запрещено и что проверять → `section "Известные ошибки"` и `section "Чеклист перед коммитом"` (с `--show --file libs/DevLib/CONVENTIONS.md`).

Ревью или новый код заканчивай проходом по «Чеклисту перед коммитом»: по каждому пункту — выполнен / не относится / нарушен (где).

Если код недоступен, минимум для напоминания: новый код только на `LocalProperty` (новых `Property`/`TProperty` нет); расчёты только `StateParameters` + `StateChained*Calculator`; входы и выходы публичные; есть `Initialize`; нет `SetCalculator`, ручных `Connect`, параметра `enable` и ручного `Enabled = true`. Перед коммитом всё равно свериться с `CONVENTIONS.md`.