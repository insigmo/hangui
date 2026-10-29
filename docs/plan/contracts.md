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
hangui a11y (status | enable)               только Linux
hangui mcp                                  MCP-сервер по stdio
hangui tui                                  интерактивный инспектор
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
