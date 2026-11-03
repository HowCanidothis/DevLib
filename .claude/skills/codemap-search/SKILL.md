---
name: codemap-search
description: Use when the user asks about code in D:\Work\wps-client or its submodules (DevLib, WPSModule, RModule) - finding a class, module or dependency, or before writing code there; look up the codemap (find, map sections, CONVENTIONS.md) before reading sources.
---

# Поиск по карте кода wps-client и соглашения

## Доступ

1. `ls "$HOME/mnt/"`: есть `wps-client` → КОРЕНЬ = `$HOME/mnt/wps-client`; есть только `DevLib` → КОРЕНЬ = `$HOME/mnt/DevLib` (пути без `libs/DevLib/`). Тогда доступ не запрашивай.
2. Иначе ОДИН раз `device_request_folder_access` для `D:\Work\wps-client` с короткой причиной на русском. Нет такой папки на машине — спроси путь. Отказ — не повторяй; предложи «Add folder» в приложении, прикрепить файлы или вставить код.
3. Индексы и разделы карт меняются только так: `ensure` и автосоздание раздела (раздел «Раздел карты» ниже); `python3 codemap.py sync && python3 codemap.py reindex`, если `find` пишет, что индексов нет, или `status` показывает `NOINDEX`/`OLD` (бесплатно); остальное — скиллами `codemap` и `codemap-update`.

Команды — `mcp__remote-devices__device_bash` с префиксом `cd "КОРЕНЬ" && `, `python3`. Без Cowork — терминал в корне (`python`/`py`). Если в корне wps-client нет `codemap.py` (корневая карта не настроена на этой машине) — работай из `libs/DevLib`: `find` там видит только DevLib, остальное ищи grep-ом.

## Где искать (важно)

Источник правды — код НА КОМПЬЮТЕРЕ пользователя. Файлы в облачном контейнере (`/mnt/user-data/uploads/...`) — случайная выборка; по ним НЕЛЬЗЯ делать вывод «такого класса нет».

Карты: у корня wps-client (локальная) и у сабмодулей `libs/DevLib`, `libs/WPSModule`, `libs/RModule` (в git) — свои `CODEMAP.md` и `.codemap/`. Из корня команды работают по всем картам, пути — от корня.

- `python3 codemap.py find ИМЯ` — где ИМЯ объявлено, реализовано, кто использует, строки карт; результаты сгруппированы по проектам. Если ИМЯ — поле класса или класс сторонней библиотеки (Qt-Ads-Custom, Eigen и т.п., исключены из карты), `find` сам это покажет (строки `field` и «declared outside the maps»). Сторонний список строится один раз командой `python3 codemap.py extern`; если `find` пишет, что его нет, — выполни её.
- `python3 codemap.py section ПАПКА --show` — раздел карты о папке (`WPSClient`, `libs/BHAModule`, `libs/DevLib/PropertiesModule`, `libs/WPSModule`). Без раздела — скрипт покажет список имеющихся.
- `python3 codemap.py status` — какие папки проиндексированы.
- Соглашения DevLib — `libs/DevLib/CONVENTIONS.md` (единственный источник правил по DevLib): `python3 codemap.py section РАЗДЕЛ --show --file libs/DevLib/CONVENTIONS.md` (список — `section ? --show --file libs/DevLib/CONVENTIONS.md`). Если у другого проекта есть свой `CONVENTIONS.md` в корне — он обязателен для его кода.

## Раздел карты: читать или создать (автоматически)

Когда нужен обзор папки или вопрос касается конкретного файла или папки:

1. `python3 codemap.py ensure ПУТЬ` (файл или папка, путь от корня; вызывай `device_bash` с `timeout_ms: 180000`, при таймауте повтори один раз — первый проход по диску бывает медленным). Скрипт сам определит ЧАСТЬ карты, к которой относится путь (большие папки вроде WPSClient поделены на части — `units` в `codemap.conf`), при необходимости бесплатно создаст или обновит её индекс и напечатает:
   `UNIT ЧАСТЬ` / `INDEX КАТАЛОГ_ИНДЕКСА (OK|created|updated)` / `SECTION 1|0`.
2. `SECTION 1` — `python3 codemap.py section ЧАСТЬ --show` и работай по нему.
3. `SECTION 0` — раздела ещё нет, создай его, НЕ спрашивая пользователя: напиши одну строку «Создаю раздел карты для ЧАСТЬ», загрузи скилл `codemap` (инструмент Skill) и выполни его шаги 4–5 для ЧАСТЬ с этим КАТАЛОГОМ_ИНДЕКСА (шаг 3 уже сделан `ensure`). Затем `section ЧАСТЬ --show`. Один раз на часть; если создание не удалось или нет инструмента Agent — скажи об этом одной строкой и продолжай по `find` и исходникам.

## Как искать класс, функцию или тип

1. `python3 codemap.py find ИМЯ`.
2. `NOT FOUND` — папка могла быть не проиндексирована: `grep -rn -w "ИМЯ" --include=*.h --include=*.hpp --include=*.cpp WPSClient libs Launcher` (сторонние библиотеки в `libs/` тоже попадут — учитывай).
3. Не нашлось и там — его нет в проекте. `ui_X.h` не ищи — его генерирует Qt из `X.ui`.
4. Исходник читай точечно — только найденные файлы.

Вопрос про модуль целиком — раздел «Раздел карты» выше. Не подменяй модуль: раздел `SharedModule` не отвечает про `PropertiesModule`.

## Перед написанием или ревью кода

Прочитай разделы `CONVENTIONS.md` по теме и всегда «Известные ошибки» и «Чеклист перед коммитом». Скиллы `wps-properties-*` и `wps-devlib-ui-patterns` — отсылки туда; при расхождении прав `CONVENTIONS.md`. После кода пройди чеклист.

В ответе указывай источник (карта, индекс, соглашения, grep, исходник). Индекс и карта могут отставать от кода; сигнатуры сверяй с исходником.

## Субагенты (обязательно)

Субагент не видит этот скилл. Поручая ему что-то про код проекта (например, Opus в режиме team), добавь в промпт дословно (подставь КОРЕНЬ):

> Код лежит на компьютере пользователя: в Cowork работай через mcp__remote-devices__device_bash с префиксом `cd "КОРЕНЬ" && `, пути — от корня wps-client. Файлы в /mnt/user-data/uploads — неполная выборка, не делай по ним вывод «нет такого класса». Класс, функцию или тип ищи так: 1) `python3 codemap.py find ИМЯ` (ищет во всех картах: корень и сабмодули); 2) если NOT FOUND — `grep -rn -w "ИМЯ" --include=*.h --include=*.hpp --include=*.cpp WPSClient libs`; 3) потом читай найденные файлы. Обзор папки: `python3 codemap.py section ПАПКА --show`. Код, использующий DevLib, обязан соответствовать `libs/DevLib/CONVENTIONS.md`: разделы читай `python3 codemap.py section РАЗДЕЛ --show --file libs/DevLib/CONVENTIONS.md`, в конце пройди «Чеклист перед коммитом». Карты, CONVENTIONS.md и .codemap не редактируй.

## Пример

«Собери форму через WidgetBuilder»: `python3 codemap.py find WidgetBuilder` → `libs/DevLib/WidgetsModule/Utils/widgetbuilder.h`, раздел `## WidgetsModule` в карте DevLib. Затем `section "UI: окна и WidgetBuilder" --show --file libs/DevLib/CONVENTIONS.md`, прочитать заголовок, писать код.
