# Пачка 02 — Linux: чтение дерева AT-SPI

Зависит от: 00, 01 | Стенд: Ubuntu VM, kesl-gui

## Цель
`hangui apps`, `hangui tree`, `hangui inspect` на реальном Linux показывают
карту kesl-gui, совпадающую с эталоном пачки 00.

## Прочитать
- `docs/research/linux-probe.md`, `docs/research/kesl-gui-tree.txt` (из 00).
- [platform-notes.md](platform-notes.md) — Linux.
- [contracts.md](contracts.md) §1, §2, §6, §8.
- `crates/hangui-core/src/lib.rs` (трейт `Provider`).

## Задачи
1. Крейт `crates/hangui-linux` (`atspi` 0.30, `zbus`, `tokio` current-thread
   во внутреннем потоке — D6). Подключение к шине доступности.
2. `apps()`: дети корня реестра (`org.a11y.atspi.Registry`), имя + pid.
3. `children()`/`node()`: роль (таблица AT-SPI → §2 + тест покрытия), имя,
   описание, состояния (StateSet → §1), `accessible_id`, атрибуты (`class`
   и т.п.), интерфейсы → список доступных действий, `Text`/`Value` для
   `value`, `Component.GetExtents(Screen)` для `bounds` (правило Wayland из §1).
4. Производительность: дерево kesl-gui глубиной 10 — замерить время;
   если > 2 с, читать свойства параллельно (join нескольких вызовов) —
   замер до и после в PROGRESS.
5. `hangui a11y status|enable` (свойство `org.a11y.Status.IsEnabled`) и
   первые проверки `doctor`: шина доступна, `IsEnabled`, тип сеанса,
   пользователь процесса vs пользователь GUI.
6. Ошибки: нет сессионной шины (запуск по SSH без переменных) →
   `permission_denied` с `fix` про `DBUS_SESSION_BUS_ADDRESS`; приложение
   закрылось во время обхода → узел пропускается, обход продолжается.

## Не делать
- Действия, ввод, события, hit-test — это 03/04/05.
- Кэш дерева между запусками CLI.

## Готово, когда
- На VM: `hangui apps` содержит kesl-gui; `hangui tree
  'application[name="kesl-gui"]' --depth 6` совпадает по структуре с
  эталоном 00 (diff приложен в PROGRESS); `--format json` валиден по §1.
- `hangui doctor` по SSH без переменных объясняет, что задать.
- Замер времени обхода записан.

## Стоп-и-спроси
- kesl-gui виден только частично (например, одно окно без детей) и ни один
  способ из 00 не помогает.
