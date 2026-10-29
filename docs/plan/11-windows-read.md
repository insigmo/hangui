# Пачка 11 — Windows: чтение дерева UIA, hit-test, фокус

Зависит от: 01, 03 | Стенд: Windows 11 (машина владельца)

## Цель
Паритет пачек 02–03 на Windows: `apps`, `tree`, `find`, `inspect`
(`--at`, `--focused`), `watch`.

## Прочитать
- [platform-notes.md](platform-notes.md) — Windows.
- [contracts.md](contracts.md) §1, §2, §3, §6, §8.
- `crates/hangui-core/src/` — трейт `Provider`; `crates/hangui-linux/src/`
  — только как образец структуры.

## Задачи
1. Крейт `crates/hangui-windows` (`windows` 0.62, фичи UIA/COM). COM MTA в
   выделенном потоке, `CUIAutomation8`, Per-Monitor-V2 DPI.
2. `apps()`: окна верхнего уровня по процессам (имя exe + заголовок + pid).
3. Обход `TreeWalker` (ControlView; RawView при `--all`) с `CacheRequest`
   на все свойства §1 и паттерны → действия.
4. Таблица ControlType → §2 с тестом покрытия; `LegacyIAccessible` как
   запасной источник `name`/`value`.
5. `ElementFromPoint`, `GetFocusedElement`; события фокуса и структуры в
   отдельном потоке.
6. `doctor`: уровень целостности процесса; предупреждение, если целевое
   окно повышено (UIPI) — `permission_denied` с советом.

## Не делать
- Действия, ввод, захват — пачка 12. MSAA-обход как основной путь.

## Готово, когда
- `hangui tree` для Блокнота, Проводника, Параметров (UWP/WinUI) и одного
  Qt- или Electron-приложения; `watch --hover` показывает кнопки Проводника.
- Диспетчер задач без прав админа даёт `permission_denied`, с правами —
  дерево. Выводы в PROGRESS.

## Стоп-и-спроси
- Обход большого окна (Проводник) дольше 2 с даже с CacheRequest.
