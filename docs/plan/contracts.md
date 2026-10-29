# Контракты hangui

Единый источник имён, которые используют несколько пачек. Меняет только та
пачка, что вводит элемент; изменение — отдельной строкой в PROGRESS.md.

## 1. Узел (`hangui_core::Node`)

JSON-вид (поле опускается, если ОС его не дала; `null` не пишется):

```json
{
  "ref": "e12",
  "role": "button",
  "native_role": "push button",
  "source": "a11y",
  "confidence": 1.0,
  "name": "Scan",
  "description": "Start a full scan",
  "value": "42",
  "native_id": "scanButton",
  "class": "QPushButton",
  "app": "kesl-gui",
  "pid": 1234,
  "bounds": { "x": 10, "y": 20, "w": 80, "h": 24 },
  "states": ["enabled", "focusable", "visible"],
  "actions": ["invoke", "focus"],
  "child_count": 0,
  "children": []
}
```

- `ref` — есть только в MCP/TUI (D7). В CLI отсутствует.
- `source` — `a11y` | `probe` | `cdp` | `vision` (architecture.md).
  `confidence` — только у `vision` (0..1); у остальных поле опускается.
- `class` — класс тулкита (`QPushButton`, `KLButton_QMLTYPE_10`, UIA
  `ClassName`, DOM-тег для `cdp`).
- `native_id` — UIA `AutomationId`, AT-SPI `accessible_id` (атрибут `id`),
  AX `AXIdentifier`.
- `bounds` — экранные координаты в физических пикселях (Windows, X11) или
  точках (macOS); на Wayland — отсутствует, если ОС не дала глобальных
  координат (не подставлять оконные).
- `children` — только в выводе `tree`; в остальных — `child_count`.

**Состояния** (`states`): `enabled`, `visible`, `showing`, `offscreen`,
`focusable`, `focused`, `selectable`, `selected`, `checkable`, `checked`,
`mixed`, `expandable`, `expanded`, `editable`, `read_only`, `modal`, `busy`,
`default`.

## 2. Роли (`hangui_core::Role`, snake_case)

`application`, `window`, `dialog`, `pane`, `group`, `button`,
`toggle_button`, `check_box`, `radio_button`, `combo_box`, `edit`, `text`,
`document`, `link`, `image`, `list`, `list_item`, `tree`, `tree_item`,
`table`, `row`, `cell`, `column_header`, `tab_list`, `tab`, `tab_panel`,
`menu_bar`, `menu`, `menu_item`, `tool_bar`, `status_bar`, `progress_bar`,
`slider`, `spin_button`, `scroll_bar`, `separator`, `tooltip`, `heading`,
`unknown`.

Таблицы соответствия (AT-SPI role / UIA ControlType / AX role) живут в
коде платформенного крейта рядом с тестом, проверяющим, что каждая
нормализованная роль кем-то покрыта.

## 3. Селекторы

```
selector := step ( combinator step )*
combinator := ">"        прямой потомок
            | ">>"       любой потомок
step     := (role | "*") filter* pseudo*
filter   := "[" attr op "\"" value "\"" "]"
attr     := name | id | class | value | description | native_role | app | pid | source
op       := "="  точное | "~=" подстрока без учёта регистра | "^=" начало | "$=" конец
pseudo   := ":nth(N)"  (с 1, среди совпавших у одного родителя)
          | ":enabled" | ":focused" | ":visible" | ":checked" | ":selected"
```

Первый шаг матчится с приложениями (`application[name="kesl-gui"]`), если
роль `application` или есть фильтр `app`/`pid`; иначе поиск идёт по всем
приложениям. Примеры:

```
application[name="kesl-gui"] >> button[name="Scan"]
window[name~="kaspersky"] >> tab_list > tab:nth(2)
*[id="scanButton"]
```

`find` возвращает все совпадения; действия требуют ровно одно совпадение,
иначе ошибка `ambiguous` со списком кандидатов.

## 4. Действия

Семантические (через API доступности): `invoke`, `focus`, `set_value`
(аргумент `value`), `toggle`, `expand`, `collapse`, `select`,
`scroll_into_view`.

Ввод (синтетический, D1): `click` (`button`: left/right/middle, `count`),
`type` (`text`), `key` (`chord`: `ctrl+s`, `alt+f4`, `enter`; имена клавиш —
по позиции, evdev-коды как канонические, как в lumepeer ADR 0065/0108).

Результат действия:
```json
{ "ok": true, "action": "invoke", "via": "semantic", "target": { ...Node } }
```
`via`: `semantic` (API доступности или семантика зонда), `in_process`
(событие тулкита, отправленное зондом внутри процесса), `input` (ввод ОС по
координатам). Порядок попыток — этот же; `--no-fallback` разрешает только
`semantic`. Перед действием узел перечитывается: нет `enabled` → `disabled`.

## 5. CLI (`hangui`)

```
hangui doctor [<app>]                       окружение; для app — доступные ступени
hangui launch [--probe auto|qt5|qt6|none] [--cdp] -- <cmd> [args]
                                            запуск приложения с зондом/CDP
hangui apps                                 [--format text|json]
hangui tree [<selector>] [--depth N] [--all] [--source S] [--format text|json]
hangui screenshot (<selector> | --app A) -o <file.png>
hangui find <selector>                      [--format text|json]
hangui inspect (<selector> | --at X,Y | --focused) [--format text|json]
hangui watch [--focus] [--hover]            поток событий до Ctrl+C
hangui do <action> <selector> [--value V] [--no-fallback] [--dry-run]
hangui click (<selector> | --at X,Y) [--button B] [--count N]
hangui type <text> [--into <selector>]
hangui key <chord>
hangui wait <selector> [--gone] [--state S] [--timeout MS]
hangui serve [--port N] [...]               локальный HTTP API (§10)
hangui a11y (status | enable)               только Linux
hangui mcp                                  MCP-сервер по stdio
hangui tui                                  интерактивный инспектор
hangui version [--format text|json]         {"name":"hangui","version":"…","protocol":1,"os":"linux","arch":"x86_64"}
hangui fetch ocr                            скачать модели OCR в кэш заранее (architecture.md «Поставка»)
```

`--fixture <file.json>` (скрытый) — подставляет провайдер-фикстуру вместо ОС.
По умолчанию `tree` скрывает `offscreen`/невидимые узлы; `--all` показывает.

Текстовый формат дерева (для человека и LLM, одна строка на узел):
```
window "Kaspersky Endpoint Security" [focused]
  tab_list
    tab "Monitoring" [selected]
    button "Scan" #scanButton (invoke)
```
`#…` — `native_id`, `[…]` — значимые состояния, `(…)` — действия.

## 6. Ошибки и коды выхода

| Код ошибки | Exit | Когда |
|---|---|---|
| — | 0 | успех |
| `internal` | 1 | непредвиденная ошибка |
| `usage` | 2 | неверные аргументы, синтаксис селектора |
| `not_found` | 3 | селектор ничего не нашёл |
| `ambiguous` | 4 | действие, а совпадений > 1 |
| `unsupported` | 5 | ОС/оболочка не даёт нужного API (D9) |
| `permission_denied` | 6 | ОС отказала, есть что включить; несёт `fix` |
| `timeout` | 7 | `wait` или действие не дождались |
| `stale` | 8 | ссылка/элемент больше не существует |
| `disabled` | 9 | элемент выключен (`enabled` нет), действие не выполнялось |
| `port_in_use` | 10 | `hangui serve`: порт занят (§10.1) |
| `forbidden` | — | только HTTP: нет токена, `Origin`, чужой `Host` (§10.2) |

JSON ошибки: `{ "error": "permission_denied", "message": "...", "fix": "..." }`.

## 7. MCP-инструменты (`hangui mcp`)

| Инструмент | Аргументы | Возвращает |
|---|---|---|
| `list_apps` | — | приложения (имя, pid, доступные ступени) |
| `launch` | `command`, `args?`, `probe?` | pid; приложение с зондом |
| `snapshot` | `app?`, `selector?`, `depth?`, `source?` | текстовое дерево со ссылками `[e12]`; для `vision` — плюс PNG окна |
| `screenshot` | `app` \| `ref` | PNG |
| `click_at` | `app`, `x`, `y` (в окне) | результат §4, `via: input` |
| `find` | `selector` | узлы со ссылками |
| `inspect` | `ref` \| `selector` | все свойства узла |
| `act` | `ref` \| `selector`, `action`, `value?` | результат §4 |
| `type_text` | `text`, `ref?` | результат §4 |
| `press_key` | `chord` | результат §4 |
| `wait_for` | `selector`, `gone?`, `timeout_ms?` | узел или `timeout` |
| `doctor` | — | список проверок |
| `selector_for` | `ref` | устойчивый селектор для теста (§10.3 `selector`; пачка 16) |

Ссылки живут до следующего `snapshot` в этой сессии (D7).

## 8. Трейт `Provider` (форма; точные сигнатуры фиксирует пачка 01)

```rust
pub trait Provider {
    fn apps(&self) -> Result<Vec<Node>>;
    fn children(&self, of: &Handle) -> Result<Vec<(Handle, Node)>>;
    fn node(&self, h: &Handle) -> Result<Node>;            // перечитать
    fn at_point(&self, x: i32, y: i32) -> Result<Option<(Handle, Node)>>;
    fn focused(&self) -> Result<Option<(Handle, Node)>>;
    fn perform(&self, h: &Handle, action: &Action) -> Result<ActionOutcome>;
    fn doctor(&self) -> Vec<Check>;
}
```
`Handle` — непрозрачный платформенный указатель (AT-SPI: bus name + object
path; UIA: элемент + RuntimeId; AX: `AXUIElementRef`; зонд: pid + id объекта;
vision: окно + прямоугольник). События (`watch`) добавляет пачка 03
отдельным методом. Несколько провайдеров (a11y, зонд, CDP, vision)
объединяет `hangui-core` (пачка 05 вводит слияние).

## 9. Протокол зондов (`probes/*` ↔ `hangui-probe`)

Сокет: Linux/macOS — `$XDG_RUNTIME_DIR/hangui/<toolkit>-<pid>.sock`
(иначе `$TMPDIR`), права только владельца; Windows — named pipe
`\\.\pipe\hangui-<toolkit>-<pid>`. Одна JSON-строка на запрос и на ответ.

| Запрос | Ответ |
|---|---|
| `{"cmd":"hello"}` | `{"toolkit":"qt5","version":"5.9.5","pid":…,"app":"…","protocol":1}` |
| `{"cmd":"tree","depth":N?}` | `{"windows":[{id,class,title,visible,geometry,root}]}`; узел: `id`, `class`, `objectName?`, `visible`, `enabled`, `rect` (в окне), `props{}`, `acc{role,name,description?,value?,states[],actions[]}?`, `children[]?` |
| `{"cmd":"act","id":…,"action":"invoke"\|"focus"\|"set_value"\|"toggle",…}` | `{"ok":true,"via":"semantic"\|"in_process"}` или ошибка §6 |
| `{"cmd":"key","id"?:…,"chord":"ctrl+a"}`, `{"cmd":"type","id"?:…,"text":"…"}` | как `act` |

`id` живёт, пока жив объект; мёртвый → `stale`. PoC (`probes/qt5`, команды
`tree`/`click`) — предшественник; пачка 05 приводит его к этой таблице.

## 10. HTTP API (`hangui serve`, пачка 16)

Тела — те же JSON, что у CLI `--format json` (§1, §4, §6). Адресация —
только селекторы §3 (D7, D15); ссылок `eN` в HTTP нет.

### 10.1 Запуск

```
hangui serve [--port N] [--host 127.0.0.1] [--token T] [--fixture F]
             [--keep-apps] [--exit-with PID]
```

- `--port`: по умолчанию **7357** (D15); `0` — свободный порт от ОС.
  Порт занят → exit 10 `port_in_use` (в `fix` — кто занимает, если ОС
  сказала, и `--port 0`).
- `--exit-with PID`: сервер завершается, когда процесс PID умер (SDK
  передаёт свой pid — упавший прогон тестов не оставляет сервер).
- Готовность — одна JSON-строка в stdout, остальные логи — в stderr:
  `{"event":"listening","url":"http://127.0.0.1:7357","port":7357,"pid":…,"state_file":"…"}`
- Файл состояния `serve-<port>.json` (`{url,port,pid,token,version,protocol}`,
  права только владельца): Linux/macOS — `$XDG_RUNTIME_DIR/hangui/`
  (иначе `$TMPDIR/hangui-<uid>/`), Windows — `%LOCALAPPDATA%\hangui\run\`.
  Удаляется при остановке.
- Остановка: `POST /v1/shutdown`, Ctrl+C/SIGTERM, смерть `--exit-with`.
  Приложения, запущенные через `launch`, закрываются, если нет `--keep-apps`.

### 10.2 Безопасность

- По умолчанию слушает только `127.0.0.1`. `--host` не-loopback — только
  вместе с `--token`.
- Каждый запрос, кроме `GET /v1/health`, несёт
  `Authorization: Bearer <token>`; токен — `--token` или случайный (в файле
  состояния).
- Отказ `403 forbidden`: нет/неверный токен; заголовок `Origin` есть
  (запрос из браузера); `Host` не `127.0.0.1:<port>`/`localhost:<port>`
  (DNS rebinding).

### 10.3 Эндпоинты

| Запрос | Тело | Ответ |
|---|---|---|
| `GET /v1/health` | — | `{"name":"hangui","version":"…","protocol":1,"pid":…,"fixture":false}` |
| `GET /v1/doctor?app=…` | — | `{"checks":[…]}` |
| `GET /v1/apps` | — | `{"apps":[Node]}` (у каждого — `sources`) |
| `POST /v1/launch` | `command`, `args?`, `env?`, `cwd?`, `probe?` (`auto`\|`qt5`\|`qt6`\|`none`), `cdp?`, `timeout_ms?`=10000 | `{"pid":…,"app":Node,"sources":[…]}` — после появления окна |
| `POST /v1/close` | `pid`, `timeout_ms?`=5000 | `{"ok":true,"killed":false}` |
| `POST /v1/tree` | `selector?`, `depth?`, `all?`, `source?`, `format?` (`json`\|`text`) | Node с `children` \| `{"text":"…"}` |
| `POST /v1/find` | `selector`, `timeout_ms?`=0 | `{"nodes":[Node]}` |
| `POST /v1/inspect` | `selector` \| `at:{x,y}` \| `focused:true`, `timeout_ms?`=5000 | Node |
| `POST /v1/act` | `action` (§4), `selector?`, `at?`, `value?`, `text?`, `chord?`, `button?`, `count?`, `no_fallback?`, `timeout_ms?`=5000 | результат §4 |
| `POST /v1/wait` | `selector`, `gone?`, `state?`, `timeout_ms?`=5000 | Node \| `{"gone":true}` |
| `POST /v1/screenshot` | `selector?` \| `app?`, `timeout_ms?` | `image/png` |
| `POST /v1/selector` | `at:{x,y}` \| `focused:true` \| `selector` | `{"selector":"…","unique":true,"alternatives":[…]}` |
| `POST /v1/fixture/reset` | — | `{"ok":true}`; без `--fixture` → `unsupported` |
| `POST /v1/shutdown` | — | `{"ok":true}`, затем выход |

Ошибка — тело §6, статус: `usage` 400, `forbidden` 403, `permission_denied`
403, `not_found` 404, `timeout` 408, `ambiguous` 409, `stale` 410,
`disabled` 422, `internal` 500, `unsupported` 501. Клиент различает ошибки
по полю `error`, статус — для людей и `curl`.

### 10.4 Автоожидание

`act`, `inspect`, `screenshot` с селектором и `find`/`wait` с
`timeout_ms > 0` перечитывают дерево каждые 100 мс до выполнения условия:
`act` — ровно одно совпадение с `visible` и `enabled`; `inspect`,
`screenshot` — ровно одно; `find` — хотя бы одно; `wait` — по `gone`/`state`.
`timeout_ms: 0` — одна попытка, ошибка сразу. По истечении — `timeout`, в
котором `cause` — последняя причина (`not_found`, `ambiguous` с
`candidates`, `disabled`).

## 11. Общий контракт SDK (`sdk/*`, пачки 17–20)

Имена ниже — нейтральные; каждый SDK пишет их в стиле языка
(`wait_gone` / `WaitGone` / `waitGone` / `WaitGoneAsync`).

### 11.1 Настройки (первое найденное побеждает)

| Что | Явный аргумент | Окружение | Дальше |
|---|---|---|---|
| адрес | `url` или `port` | `HANGUI_URL`, затем `HANGUI_PORT` | `http://127.0.0.1:7357` |
| токен | `token` | `HANGUI_TOKEN` | файл состояния порта (только loopback) |
| бинарь | `binary` | `HANGUI_BIN` | `PATH`, кэш, скачивание — §11.6 |
| таймаут действий | `timeout` | `HANGUI_TIMEOUT_MS` | 5000 мс |

### 11.2 Жизненный цикл

- `is_running(url?)` — `GET /v1/health` ≤1 с; `true`, только если ответил
  hangui. Не бросает.
- `connect(opts?)` — подключиться к работающему; нет → `not_running` с
  `fix` («запустите `hangui serve` или используйте `start()`»); `protocol`
  другой → `protocol_mismatch` (какую версию поставить).
- `start(opts?)` — если на порту уже hangui → подключиться (сервер чужой,
  не останавливать); порт занят не hangui → `port_in_use`; иначе найти или
  скачать бинарь (§11.6) и запустить
  `hangui serve --port P --exit-with <свой pid> [--fixture F]`, ждать
  строку готовности ≤10 с (`HANGUI_START_TIMEOUT_MS`), stderr — в лог SDK.
  `port: 0` — всегда свой сервер на свободном порту (параллельные воркеры).
- `close()` — закрыть приложения, запущенные этим клиентом; сервер — только
  если его запустил этот клиент (`shutdown`, затем kill через 5 с).
- `launch(command, …)` верхнего уровня (без клиента) = `start()` +
  `client.launch(...)`; возвращённый `App` владеет клиентом: `app.close()`
  закрывает приложение и свой сервер (чужой — не трогает). Для скрипта в
  три строки, где setup-фикстуры нет.
- `ensure_binary(opts?)` → путь к бинарю по §11.6 без запуска сервера (для
  подготовки образов CI и кэша заранее).

### 11.3 Поверхность

- **Клиент:** `apps`, `launch(command, args?, probe?, cdp?, env?, cwd?)` →
  `App`, `tree`, `find`, `inspect`, `wait`, `screenshot`, `doctor`,
  `selector_for`, `locator(selector)`, `type(text)`, `press(chord)`,
  `reset_fixture`.
- **App:** `pid`, `name`, `locator(selector)` (префикс
  `application[pid="N"] >> `), `tree`, `screenshot`, `close`.
- **Locator** (ленивый: селектор вычисляется при каждом вызове):
  `invoke`, `click(button?, count?)`, `type(text)`, `press(chord)`,
  `set_value`, `toggle`, `focus`, `select`, `expand`, `collapse`,
  `scroll_into_view`, `wait(state?)`, `wait_gone`, `exists` (без ожидания),
  `inspect`, `text` (`value`, иначе `name`), `count`, `screenshot`.
  У каждого действия — `timeout` и `no_fallback`.

### 11.4 Ошибки

Один базовый тип с полями `code` (строка §6), `message`, `fix`,
`candidates`, `cause`; подтипы по кодам §6 и кодам SDK: `not_running`,
`protocol_mismatch`, `port_in_use`, `server_start_failed`,
`binary_not_found`, `download_failed` (§11.6). Неизвестный код
от сервера — базовый тип с этим кодом (новый сервер не ломает старый SDK).
Текст ошибки включает `fix` и кандидатов — его читает ИИ, чинящий тест.

### 11.5 Тесты

- Интеграция с раннерами: сервер на прогон (`start`, в конце `close`),
  приложение на тест (закрывается в teardown), при падении — снимок окна и
  текстовое дерево (§5) в артефакты теста.
- Каждый SDK проходит `sdk/conformance/cases.json` против
  `hangui serve --port 0 --fixture tests/fixtures/sample-app.json`.
- Версия SDK = версия hangui; `protocol` растёт только при несовместимом
  изменении §10.
- Каждый SDK проверяет §11.6 против локального HTTP-сервера с поддельным
  релизом (`HANGUI_DOWNLOAD_URL=http://127.0.0.1:…`, пустой `PATH`,
  временный `HANGUI_CACHE_DIR`): скачал, распаковал, запустил, сообщения в
  stderr; второй вызов — из кэша без сети; неверная сумма →
  `download_failed`, в кэше ничего; `HANGUI_DOWNLOAD=0` →
  `binary_not_found`; два процесса одновременно → одно скачивание. Таблицу
  сумм тест подменяет средствами языка, не переменной окружения.

### 11.6 Поиск и скачивание бинаря (D19)

Выполняется, когда SDK нужен свой сервер (`start()`, `launch(...)` верхнего
уровня, фикстуры раннеров, `ensure_binary()`). При подключении к уже
работающему hangui бинарь не ищется.

1. **Явно:** `binary` или `HANGUI_BIN` — путь как есть; файла нет →
   `binary_not_found` (явную настройку не подменять скачиванием).
2. **`PATH`:** `hangui` (`hangui.exe`) — берётся, если
   `hangui version --format json` даёт тот же `protocol`, что у SDK; иначе
   сообщение (п. 5) и дальше к п. 3.
3. **Кэш:** `<cache>/hangui/<версия SDK>/hangui[.exe]`; `<cache>` —
   `HANGUI_CACHE_DIR`, иначе `$XDG_CACHE_HOME` (`~/.cache`) /
   `%LOCALAPPDATA%` / `~/Library/Caches`.
4. **Скачивание:** `<base>/v<версия>/hangui-<версия>-<os>-<arch>.gz`;
   `<base>` — `HANGUI_DOWNLOAD_URL`, иначе релизы GitHub проекта; `os` ∈
   `linux`/`windows`/`macos`, `arch` ∈ `x86_64`/`arm64`. Один gzip-файл
   (не tar/zip — их нет в стандартной библиотеке Node и netstandard2.0).
   sha256 распакованного бинаря сверяется с таблицей, **вшитой в пакет SDK**
   при релизе (не со скачанным файлом сумм). Запись — во временный файл в
   том же каталоге, `chmod +x`, атомарное переименование; параллельные
   процессы ждут блокировку `<версия>/.lock` и берут готовый файл. Таймаут —
   `HANGUI_DOWNLOAD_TIMEOUT_MS` (по умолчанию 120 000); прокси — из
   `HTTPS_PROXY`/`NO_PROXY`.
5. **Сообщения — всегда в stderr** (не через логгер, который раннер может
   заглушить), с префиксом `hangui:`, на английском, по строке на шаг:
   ```
   hangui: binary not found (HANGUI_BIN, PATH, cache) — downloading 0.4.0 for linux-x86_64
   hangui: https://github.com/<owner>/hangui/releases/download/v0.4.0/hangui-0.4.0-linux-x86_64.gz
   hangui:   -> /home/u/.cache/hangui/0.4.0/hangui
   hangui: 6.2 MB, sha256 ok, ready in 3.1 s
   ```
   Найден в `PATH`/кэше — одна строка `hangui: using /…/hangui (0.4.0)`
   при первом запуске сервера в процессе; `PATH` с другим `protocol` —
   строка, какая версия найдена и какая скачивается.
6. **Ошибки:** `HANGUI_DOWNLOAD=0` (или `false`) и бинаря нет →
   `binary_not_found`, `fix` — адрес файла и `HANGUI_BIN`; сеть, HTTP-статус
   или сумма → `download_failed` с `cause`, недокачанный файл удаляется,
   битый бинарь в кэш не попадает; нет сборки для этой ОС/архитектуры →
   `binary_not_found` со списком доступных.
