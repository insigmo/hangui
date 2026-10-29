# Пачка 00 — разведка стенда Linux и kesl-gui

Дата: 2026-10-29. Все команды выполнены по SSH `beta@192.168.40.128` от
пользователя GUI-сеанса (uid 1000). Где нужен сеанс:
`DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/1000/bus`.

## Итог в трёх строках

1. **kesl-gui не отдаёт дерево через AT-SPI.** Его собственный Qt 5.9.5 собран
   без моста AT-SPI; никакой флаг или переменная окружения это не включают.
2. **AT-SPI на стенде работает полностью** для приложений с мостом:
   `gnome-calculator` (GTK4) — дерево, нажатие кнопки и запись текста через
   API работают под Wayland.
3. **Синтетический ввод на этом GNOME/Wayland идёт только через портал
   RemoteDesktop** — даже XTest в XWayland (XWayland запущен с
   `-enable-ei-portal`) вызывает диалог согласия.

## Окружение (задача 2)

| Что | Значение | Команда |
|---|---|---|
| ОС | Ubuntu 26.04.1 LTS, ядро 7.0.0-42 | `/etc/os-release`, `uname -r` |
| Сеанс | `Type=wayland`, `XDG_CURRENT_DESKTOP=ubuntu:GNOME`, GNOME Shell 50.1 | `loginctl show-session 1`, `gnome-shell --version` |
| XWayland | 24.1.10, `-rootless … -enable-ei-portal`, `DISPLAY=:0`, `XAUTHORITY=/run/user/1000/.mutter-Xwaylandauth.*` | `ps`, `systemctl --user show-environment` |
| Сеанс Xorg на входе | не проверялся с экрана; в 26.04 GNOME без X11-сеанса — **[проверить при пачке 05]** | — |
| AT-SPI | at-spi2-core 2.60.4, `at-spi-bus-launcher` и `at-spi2-registryd` запущены; адрес шины `unix:path=/run/user/1000/at-spi/bus` | `busctl --user call org.a11y.Bus /org/a11y/bus org.a11y.Bus GetAddress` |
| `org.a11y.Status` | было `IsEnabled=false`, `ScreenReaderEnabled=false`; **оставлено `IsEnabled=true`** | `busctl --user get/set-property … IsEnabled` |
| gsettings | `toolkit-accessibility=false` | `gsettings get org.gnome.desktop.interface toolkit-accessibility` |

## kesl-gui (задача 3)

- Процесс: `/opt/kaspersky/kesl/libexec/kesl-gui --tray-only`, пользователь
  `beta` (не root). Пользователь в группах `kesluser`, `kesladmin`.
  Запускается из `/etc/xdg/autostart/kaspersky-kesl.desktop` через
  shell-скрипт `kesl-gui-launcher` (он делает `unset QT_QPA_PLATFORM` и
  задаёт `FONTCONFIG_FILE`).
- Версия KESL 12.5.0.1569. Тулкит — **собственный Qt 5.9.5** в
  `/opt/kaspersky/kesl/lib64/gui/` (Widgets, Quick, QuickControls2, Qml,
  DBus), интерфейс на QML. Бинарь с `DT_RPATH=/opt/kaspersky/kesl/lib64/gui/`
  — `LD_LIBRARY_PATH` его не переопределяет.
- Платформа Qt — `xcb` (бандл `libqxcb.so` + `libQt5XcbQpa.so.5`), т.е.
  kesl-gui — **X11-клиент в XWayland**. Окно видно через X11:
  `0x800004 "Kaspersky Endpoint Security 12.5 for Linux" ("kesl-gui")
  1000x580 +173+126`, `_NET_WM_PID=9320`.
- Окружение процесса: `QT_ACCESSIBILITY=1` (задаёт Ubuntu глобально),
  `GTK_MODULES=gail:atk-bridge`, `QT_IM_MODULE=ibus`; `NO_AT_BRIDGE` нет.

## Шина доступности (задачи 4–5)

Список приложений на шине (`Atspi.get_desktop(0)`): gnome-shell,
ibus-extension-gtk3, update-notifier, evolution-alarm-notify,
lumepeer-desktop, vmware-user, xdg-desktop-portal-gtk, mutter-x11-frames,
gjs, ptyxis. **kesl-gui нет.**

Что пробовал, чтобы он появился:

1. `IsEnabled=true` при работающем процессе — нет.
2. Перезапуск с открытым главным окном и
   `QT_LINUX_ACCESSIBILITY_ALWAYS_ON=1` (`systemd-run --user
   --unit=hangui-kesl-gui --setenv=QT_LINUX_ACCESSIBILITY_ALWAYS_ON=1
   kesl-gui-launcher`; переменная видна в `/proc/9320/environ`) — нет.

Почему — статический разбор бандла:

| Проверка | Результат |
|---|---|
| `strings` (8 бит) по всем `.so` в `lib64/gui`: `org.a11y` / `QSpi` / `AT-SPI` | 0 (единственное «совпадение» в Widgets — `QSpinBox`) |
| то же в UTF-16 (`strings -el`) по qxcb, XcbQpa, Gui, DBus | 0 |
| `QT_LINUX_ACCESSIBILITY_ALWAYS_ON` в qxcb/XcbQpa/Gui | 0 |
| Контроль метода: системный `libatk-bridge-2.0.so.0` | 93 совпадения `org.a11y` |
| Экспортированные `QAccessible*` в Gui / Widgets / Quick | 224 / 34 / 0 |

Вывод: API `QAccessible` в Qt есть, но мост, который публикует его в AT-SPI
(в Qt5 он вкомпилирован в xcb-плагин), при сборке этого Qt не включён.
Включить его снаружи нельзя.

## Дерево kesl-gui (задача 6)

Дерева нет — см. выше. В [kesl-gui-tree.txt](kesl-gui-tree.txt) — то, что
доступно: окна X11.

## Координаты и действия на Wayland (задача 7) — на gnome-calculator (GTK4)

- `Component.get_extents(SCREEN)` — **всегда `0,0`** для всех узлов;
  `get_extents(WINDOW)` — правильные координаты внутри окна (кнопка «7»:
  `win=24,428`, 56×40). Экранных координат на Wayland нет.
- Кнопки: роль `button`, имя `"7"`, единственное действие `click`; внутри —
  `label "7"`. `accessible_id` есть у части узлов (`MathWindow`,
  `source_view`, `_buttons`, `headerbar`); у кнопок — пустой.
- `Action.do_action(click)` на кнопке «7» → `True`, текст поля
  `source_view` стал `'7'`. **Работает.**
- `EditableText.set_text_contents("12+30")` → `True`, поле `'12+30'`.
  **Работает.**
- `Component.grab_focus` на `source_view` → `GError atspi_error (1)`.
  **Не работает** на GTK4.
- Окно приложения само имеет действия (`win.close`, `window.minimize`,
  `default.activate` …) — GTK4 отдаёт `GAction` как AT-SPI-действия.

## Синтетический ввод в XWayland

- `XTestQueryExtension` → есть, v2.2. `XTestFakeMotionEvent(673,416)` →
  указатель **не сдвинулся** (`XQueryPointer` до и после: `982,33`).
- В ту же секунду `xdg-desktop-portal-gnome` открыл окно «Remote Desktop»
  («Allow Remote Interaction», кнопка «Share») — найдено через AT-SPI. Т.е.
  XWayland 24.1 с `-enable-ei-portal` пропускает XTest только после согласия
  через портал RemoteDesktop. Отдельного X11-обхода на GNOME/Wayland нет.

## Что оставлено на стенде

- `org.a11y.Status IsEnabled=true` (нужно для следующих пачек).
- kesl-gui работает как пользовательский юнит `hangui-kesl-gui.service` с
  главным окном (было: `--tray-only` из автозапуска). Вернуть как было:
  `systemctl --user stop hangui-kesl-gui` и перелогиниться.
- `gnome-calculator` как юнит `hangui-calc.service`.
- `/tmp/hangui-dump.py` — проба-дампер на python3-gi (не продукт).
- Диалог «Remote Desktop» от пробы XTest — закрыть вручную.

## Что это значит для плана

Пачки 02–05 сформулированы вокруг дерева kesl-gui — его нет. Варианты —
в разделе «Решение владельца» PROGRESS.md.
