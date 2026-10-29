# Пачка 17 — SDK для Python (`hangui`, pytest)

Зависит от: 16 | Стенд: любая ОС на фикстуре; Ubuntu VM для приёмки | Параллельно с 18, 19, 20

## Цель
ИИ пишет на Python тест, который потом сам по себе запускается `pytest`:
фикстура поднимает (или находит) hangui, тест запускает приложение, жмёт,
проверяет, закрывает.

## Прочитать
- [README.md](README.md) — D14–D19.
- [contracts.md](contracts.md) §3, §4, §6, §10, §11.
- `sdk/conformance/cases.json`.

## Задачи
1. `sdk/python/` — пакет `hangui` (PyPI), Python ≥3.9, `pyproject.toml`
   (hatchling), **без зависимостей во время выполнения** (`urllib.request`,
   `json`, `subprocess`). Полные аннотации типов, `py.typed`.
2. Клиент по §11: `hangui.is_running(url=None)`, `hangui.connect(...)`,
   `hangui.start(port=None, fixture=None, binary=None, args=())` →
   `Hangui` (контекстный менеджер; `close()` останавливает только свой сервер).
   Порядок настроек и порт по умолчанию — §11.1. Верхнеуровневый
   `hangui.launch(command, ...)` → `App`, владеющий своим сервером (§11.2).
   Поиск и скачивание бинаря — §11.6 (`hangui.ensure_binary()`): stdlib
   (`urllib.request`, `gzip`, `hashlib`, блокировка — `fcntl`/`msvcrt`),
   таблица sha256 — модуль `hangui/_checksums.py`, генерируется из
   `dist/checksums.json` при релизе; сообщения — `print(..., file=sys.stderr)`.
   `python -m hangui fetch` — то же из консоли (для образов CI).
3. Методы — по §11.3 в snake_case: `apps()`, `launch(...)` → `App`
   (`app.close()`, `app.locator(...)`), `tree`, `find`, `inspect`, `wait`,
   `screenshot(path)`, `doctor`, `selector_for`; `Locator` с `invoke`,
   `click`, `type`, `press`, `set_value`, `toggle`, `focus`, `select`,
   `expand`, `collapse`, `wait`, `wait_gone`, `exists`, `inspect`, `text`.
4. Ошибки — по §11.4: `HanguiError(code, message, fix, candidates)` и
   подклассы по коду (`NotFound`, `Ambiguous`, `Disabled`, `Timeout`, …),
   плюс `NotRunning`, `ProtocolMismatch`. Сообщение включает `fix`.
5. Плагин pytest (entry point `pytest11`, модуль `hangui.pytest_plugin`):
   фикстура `hangui` (scope `session`: `start()` / остановка), фикстура
   `hangui_app` (фабрика: `hangui_app("kesl-gui", probe="auto")` —
   запуск в тесте, закрытие в teardown), опции `--hangui-port`,
   `--hangui-url`, `--hangui-bin`, `--hangui-fixture`. При падении теста —
   снимок экрана и текстовое дерево приложения в `tmp_path` и в отчёт.
6. Тесты пакета: `sdk/conformance/cases.json` против
   `hangui serve --port 0 --fixture …`; юнит-тесты разбора настроек;
   пример `sdk/python/examples/test_sample_app.py` на фикстуре.
7. CI-задача: Python 3.9 и 3.13 на ubuntu/windows/macos.

## Не делать
- async-API (`asyncio`) — после первого релиза, если попросят.
- Запуск hangui через CLI вместо HTTP (кроме `hangui serve` в `start()`).
- Обёртки над unittest/robotframework — достаточно обычного клиента.

## Готово, когда
- `pytest sdk/python` зелёный на фикстуре, conformance пройден полностью.
- На стенде: тест из трёх строк (`hangui_app("kesl-gui")` →
  `locator('button[name="View"]').invoke()` → `wait`) проходит под
  `pytest` без ИИ; второй запуск при уже работающем `hangui serve` его
  использует и не останавливает.
- На чистой VM без hangui: `pip install` пакета, затем
  `hangui.launch("gnome-calculator")` в `python -c` — в stderr строки §11.6
  о скачивании, приложение запущено; второй запуск — без сети, из кэша.
- Тесты §11.5 на скачивание проходят.
- `mypy --strict` и `ruff` чисты.

## Стоп-и-спроси
- Имя `hangui` на PyPI занято — предложить другое до публикации (21).
