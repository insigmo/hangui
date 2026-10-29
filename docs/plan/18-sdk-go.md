# Пачка 18 — SDK для Go

Зависит от: 16 | Стенд: любая ОС на фикстуре; Ubuntu VM для приёмки | Параллельно с 17, 19, 20

## Цель
ИИ пишет на Go тест (`go test`), который поднимает или находит hangui через
`TestMain`/`t.Cleanup` и работает с приложением по HTTP API.

## Прочитать
- [README.md](README.md) — D14–D19.
- [contracts.md](contracts.md) §3, §4, §6, §10, §11.
- `sdk/conformance/cases.json`.

## Задачи
1. `sdk/go/` — модуль `<путь репозитория>/sdk/go` (путь уточнить у
   владельца по `git remote`), пакет `hangui`, Go ≥1.22, **только
   стандартная библиотека** (`net/http`, `encoding/json`, `os/exec`).
2. Клиент по §11: `hangui.IsRunning(ctx, url)`, `hangui.Connect(ctx, opts)`,
   `hangui.Start(ctx, opts)` → `*Client`; `Close()` останавливает только
   свой сервер. `Options{Port, URL, Token, Binary, Fixture, Args}`; пустые
   поля берутся из окружения и §11.1. Верхнеуровневый
   `hangui.Launch(ctx, cmd, ...)` → `*App`, владеющий своим сервером (§11.2).
   Поиск и скачивание бинаря — §11.6 (`hangui.EnsureBinary(ctx, opts)`):
   stdlib (`net/http`, `compress/gzip`, `crypto/sha256`), таблица sha256 —
   `checksums_gen.go` из `dist/checksums.json` при релизе; сообщения —
   `os.Stderr`. `go run <модуль>/cmd/hangui-fetch` — то же из консоли.
3. Методы — по §11.3 в CamelCase, первым аргументом `context.Context`:
   `Apps`, `Launch` → `*App` (`Close`, `Locator`), `Tree`, `Find`,
   `Inspect`, `Wait`, `Screenshot`, `Doctor`, `SelectorFor`; `Locator` с
   `Invoke`, `Click`, `Type`, `Press`, `SetValue`, `Toggle`, `Focus`,
   `Select`, `Expand`, `Collapse`, `Wait`, `WaitGone`, `Exists`, `Inspect`,
   `Text`. Опции действий — функциональные (`hangui.Timeout(d)`,
   `hangui.NoFallback()`).
4. Ошибки — по §11.4: `*hangui.Error{Code, Message, Fix, Candidates}`,
   константы кодов, `errors.Is(err, hangui.ErrNotFound)` и т.д.;
   `ErrNotRunning`, `ErrProtocolMismatch`.
5. Подпакет `hanguitest`: `hanguitest.Start(t)` (сервер на тест/пакет,
   остановка через `t.Cleanup`), `hanguitest.Launch(t, c, cmd, ...)`
   (закрытие через `t.Cleanup`), при `t.Failed()` — снимок и дерево в
   `t.TempDir()` с путём в `t.Log`. Флаги/окружение — §11.1.
6. Тесты: conformance против `hangui serve --port 0 --fixture …`; пример
   `sdk/go/example_test.go` (Example-функция для godoc) и
   `sdk/go/examples/sample_app_test.go`.
7. CI-задача: `go vet`, `go test ./...` на ubuntu/windows/macos.

## Не делать
- Сторонние зависимости (testify и т.п.) в модуле SDK.
- Генерацию клиента из OpenAPI: API маленький, ручной клиент проще.

## Готово, когда
- `go test ./...` в `sdk/go` зелёный на фикстуре, conformance пройден.
- На стенде: тест с `hanguitest.Start` + `Launch("kesl-gui")` + `Invoke`
  проходит под `go test` без ИИ; при уже работающем `hangui serve`
  используется он.
- На чистой VM без hangui: `go test` примера скачивает бинарь с
  сообщениями §11.6 в stderr и проходит; второй прогон — из кэша.
- Тесты §11.5 на скачивание проходят.
- `go vet` и `staticcheck` чисты.

## Стоп-и-спроси
- Модуль в подкаталоге монорепозитория неудобен для `go get` (теги
  `sdk/go/vX.Y.Z`) — предложить отдельный репозиторий до публикации (21).
