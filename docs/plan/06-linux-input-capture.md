# Пачка 06 — Linux: ввод и захват X11/Wayland

Зависит от: 04 | Стенд: Ubuntu VM (GNOME/Wayland; X11-сеанс — если доступен на входе, иначе другая VM в 10)

## Цель
Ступень `via: input` и пиксели для визуального слоя: `click`, `type`, `key`,
`screenshot`, запасной путь для `invoke`/`set_value`.

## Прочитать
- [architecture.md](architecture.md) — «Лестница действий», «Захват пикселей».
- [platform-notes.md](platform-notes.md) — Linux, «Синтетический ввод».
- [contracts.md](contracts.md) §4, §5, §6.
- В lumepeer (только для справки): `crates/media/src/capture/linux_x11.rs`,
  `crates/media/src/capture/linux_wayland.rs`, `crates/media/src/capture/pipewire_stream.rs`,
  `docs/adr/0010-*.md`, `0160-*.md`, `0166-*.md`.

## Задачи
1. Определение сеанса (`XDG_SESSION_TYPE` → `WAYLAND_DISPLAY` → `DISPLAY`).
2. Захват окна X11/XWayland: `XGetImage` по id окна (`x11rb` 0.14; на
   Wayland-сеансе работает без портала — проверено в 00). Сопоставление
   окна и приложения — `_NET_WM_PID`.
3. Wayland: одна сессия портала RemoteDesktop + ScreenCast (`ashpd` 0.13),
   `persist_mode` + `restore_token` в `$XDG_STATE_HOME/hangui/`; кадр
   монитора из PipeWire; координаты указателя — в координатах того же
   потока. Нет портала → `unsupported` с советом (Cinnamon → X11).
4. Ввод: X11-сеанс — XTest; Wayland-сеанс — портал (XTest в XWayland тоже
   уходит в портал — проверено в 00). Клавиши по evdev-коду; `type` через
   keysym раскладки, недоступные символы — ошибка с перечнем.
5. Связь окна и монитора на Wayland: где окно на кадре ScreenCast — по
   геометрии XWayland-окна (проверить, совпадает ли) или по поиску снимка
   окна (`XGetImage`) в кадре монитора; результат и способ — в platform-notes.
6. Запасной путь для узлов с `bounds`: `invoke` → клик по центру
   (`via: input`), `--no-fallback` запрещает. `doctor`: тип сеанса,
   XTest/портал, сохранён ли токен.

## Не делать
- `/dev/uinput`, ydotool, libei (`reis`) — пока портал работает.
- OCR и распознавание — пачка 07.

## Готово, когда
- `hangui screenshot --app kesl-gui -o k.png` без диалогов (XWayland-окно).
- Wayland: первый `hangui click --at` показывает диалог портала, второй
  запуск — нет (токен); клик по кнопке «7» калькулятора по её экранным
  координатам вводит `7`; `type` в поле вводит `Hello, мир 123`; `key ctrl+a`.
- Вывод проверок в PROGRESS.

## Стоп-и-спроси
- Портал старее 1.17 и диалог появляется каждый раз.
