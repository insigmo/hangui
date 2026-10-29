# Пачка 19 — SDK для TypeScript и JavaScript

Зависит от: 16 | Стенд: любая ОС на фикстуре; Ubuntu VM для приёмки | Параллельно с 17, 18, 20

## Цель
Один npm-пакет для TS и JS: тест на Vitest, Jest, `node:test` или
Playwright Test поднимает или находит hangui в `globalSetup`/`beforeAll` и
работает с приложением по HTTP API.

## Прочитать
- [README.md](README.md) — D14–D19.
- [contracts.md](contracts.md) §3, §4, §6, §10, §11.
- `sdk/conformance/cases.json`.

## Задачи
1. `sdk/js/` — пакет `hangui` (npm), исходник на TypeScript, сборка в ESM
   и CJS + `.d.ts` (`tsup`), Node ≥18, **без зависимостей во время
   выполнения** (встроенный `fetch`, `child_process`). JS-пользователь
   получает тот же API, TS — типы узлов, ролей, кодов ошибок из §1–§6.
2. Клиент по §11: `isRunning(opts?)`, `connect(opts?)`, `start(opts?)` →
   `Hangui`; `close()` останавливает только свой сервер; поддержка
   `await using` (`Symbol.asyncDispose`). Настройки и порт — §11.1.
   Верхнеуровневый `launch(command, opts?)` → `App`, владеющий своим
   сервером (§11.2).
   Поиск и скачивание бинаря — §11.6 (`ensureBinary(opts?)`): встроенные
   `fetch`, `zlib.gunzip`, `crypto`; встроенный `fetch` в Node 18–22 не
   читает `HTTPS_PROXY` — при заданном прокси идти через `node:https` +
   `CONNECT`, без зависимостей. Таблица sha256 — `src/checksums.ts` из
   `dist/checksums.json` при релизе; сообщения — `process.stderr`.
   `npx hangui fetch` — то же из консоли (поле `bin` пакета).
3. Методы — по §11.3 в camelCase, все `async`: `apps`, `launch` → `App`
   (`close`, `locator`), `tree`, `find`, `inspect`, `wait`, `screenshot`,
   `doctor`, `selectorFor`; `Locator` с `invoke`, `click`, `type`, `press`,
   `setValue`, `toggle`, `focus`, `select`, `expand`, `collapse`, `wait`,
   `waitGone`, `exists`, `inspect`, `text`.
4. Ошибки — по §11.4: `HanguiError` с `code`, `fix`, `candidates`;
   `NotRunningError`, `ProtocolMismatchError`; объединение строковых
   литералов кодов в типах.
5. Интеграции без зависимостей от фреймворков (подпути экспорта):
   `hangui/global-setup` — функция для `globalSetup` Vitest/Jest/Playwright
   (сервер на весь прогон, URL передаётся воркерам через `HANGUI_URL`);
   `hangui/test` — `useHangui()` и `useApp(cmd, opts)` для
   `beforeAll`/`afterAll` любого раннера. При падении — снимок и дерево в
   каталог отчётов.
6. Тесты: conformance на `node:test` против
   `hangui serve --port 0 --fixture …`; примеры `sdk/js/examples/`
   (`vitest`, `jest`, `node:test`, обычный `.mjs` без TS).
7. CI-задача: Node 18/22 на ubuntu/windows/macos, `tsc --noEmit`, eslint.

## Не делать
- Браузерную сборку (API отклоняет запросы с `Origin`, §10.2).
- Отдельные пакеты под каждый раннер.

## Готово, когда
- `npm test` в `sdk/js` зелёный на фикстуре, conformance пройден; пакет
  импортируется и из ESM, и из CJS (тест на оба).
- На стенде: тест Vitest с `globalSetup` запускает kesl-gui, нажимает
  «View», ждёт результат, закрывает — без ИИ.
- На чистой VM без hangui: `npm i` пакета и `node -e` с `launch(...)` —
  скачивание с сообщениями §11.6, второй запуск из кэша; тесты §11.5 на
  скачивание проходят.

## Стоп-и-спроси
- Имя `hangui` на npm занято — предложить scope (`@<owner>/hangui`) до
  публикации (21).
