# Пачка 02 — Linux: чтение дерева AT-SPI

Зависит от: 01 | Стенд: Ubuntu VM (gnome-calculator, «Параметры», «Файлы»)

## Цель
`hangui apps`, `hangui tree`, `hangui inspect` на реальном Linux показывают
карту приложений с AT-SPI. Эталон — gnome-calculator (дамп из пачки 00).
kesl-gui здесь — пример «дерева нет» (его карта — пачка 05).

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
   описание, состояния (StateSet → §1), `accessible_id`, атрибуты,
   интерфейсы → действия, `Text`/`Value` для `value`, `bounds` из
   `GetExtents(Screen)` — на Wayland это `0,0` (проверено в 00), поэтому
   там `bounds` не выводится (contracts §1); оконные координаты — в
   отдельное поле `window_bounds` (ввести в contracts §1).
4. Производительность: дерево «Параметров» GNOME глубиной 10 — замерить
   время; если > 2 с, читать свойства параллельно — замер до и после в PROGRESS.
5. `hangui a11y status|enable` (`org.a11y.Status.IsEnabled`) и первые
   проверки `doctor`: шина доступна, `IsEnabled`, тип сеанса, пользователь
   процесса vs пользователь GUI, X11-окна без узла в AT-SPI («kesl-gui:
   окно есть, дерева нет» — сопоставление по pid из `_NET_WM_PID`).
6. Ошибки: нет сессионной шины (SSH без переменных) → `permission_denied`
   с `fix` про `DBUS_SESSION_BUS_ADDRESS`; приложение закрылось во время
   обхода → узел пропускается, обход продолжается.

## Не делать
- Действия, ввод, события, hit-test — это 03/04/06.
- Кэш дерева между запусками CLI.

## Готово, когда
- На VM: `hangui tree 'application[name="gnome-calculator"]'` содержит
  `button "7" (invoke)` и `edit #source_view`; структура совпадает с дампом
  00 (diff в PROGRESS); `--format json` валиден по §1.
- `hangui doctor` перечисляет kesl-gui как «окно без дерева доступности» и
  по SSH без переменных объясняет, что задать.
- Замер времени обхода записан.

## Стоп-и-спроси
- Дерево стандартного GNOME-приложения неполное по сравнению с дампом 00.
