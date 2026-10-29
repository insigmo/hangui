# Пачка 05 — Linux: синтетический ввод X11/Wayland

Зависит от: 04 | Стенд: Ubuntu VM — сеанс X11 и сеанс Wayland

## Цель
`click`, `type`, `key` и запасной путь для `invoke`/`set_value`, когда у
элемента нет семантического действия (D1).

## Прочитать
- [platform-notes.md](platform-notes.md) — Linux, «Синтетический ввод».
- [contracts.md](contracts.md) §4, §6.
- В lumepeer (только для справки, не копировать целиком):
  `crates/media/src/capture/linux_x11.rs` (XTest-инжектор),
  `crates/media/src/capture/linux_wayland.rs` (портал),
  `docs/adr/0010-*.md`, `0160-*.md`, `0166-*.md`.

## Задачи
1. Определение сеанса (`XDG_SESSION_TYPE` → `WAYLAND_DISPLAY` → `DISPLAY`).
2. X11: XTest (`x11rb` 0.14) — движение, кнопки, колесо, клавиши по evdev-коду
   (+8). Таблица имён клавиш для `key` (`ctrl`, `alt`, `shift`, `super`,
   `enter`, `tab`, `esc`, `f1`–`f12`, стрелки, буквы/цифры по позиции).
3. `type`: по символам через раскладку X11 (keysym → keycode), символы вне
   раскладки — ошибка с перечнем, не молчаливый пропуск.
4. Wayland: `ashpd` 0.13 RemoteDesktop — сессия, `SelectDevices` с
   `persist_mode` и `restore_token` (хранить в `$XDG_STATE_HOME/hangui/`),
   `notify_*`. Нет портала → `unsupported` с советом (Cinnamon → X11).
5. Запасной путь: `invoke` без действия → клик по центру `bounds` (только
   если `bounds` экранные); `--no-fallback` запрещает; `via: "input"`.
6. `doctor`: тип сеанса, доступность XTest / портала, сохранён ли токен.

## Не делать
- `/dev/uinput`, ydotool, libei (`reis`) — пока портал работает.
- Захват экрана/скриншоты.

## Готово, когда
- X11: `hangui click --at` по кнопке kesl-gui нажимает её; `type` в поле
  ввода (любого GTK-приложения) вводит `Hello, мир 123`; `key ctrl+a`.
- Wayland (GNOME): первый запуск показывает диалог, второй — нет
  (восстановление по токену); `key` и `type` работают.
- Вывод обеих проверок в PROGRESS.

## Стоп-и-спроси
- Портал на стенде старее 1.17 и диалог появляется каждый раз — решить,
  приемлемо ли это.
