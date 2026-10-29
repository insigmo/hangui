# Платформенные заметки

Известные ловушки — чтобы пачки их не открывали заново. Пометки:
**[lumepeer]** — проверено в проекте lumepeer (ссылка на ADR);
**[док]** — по документации, на стенде hangui не проверено;
**[проверить]** — предположение, пачка обязана подтвердить или опровергнуть
и исправить этот файл.

## Linux

### Доступ к AT-SPI
- AT-SPI2 — отдельная D-Bus шина. Адрес берётся вызовом `GetAddress` у
  `org.a11y.Bus` на сессионной шине. Крейт `atspi` делает это сам. **[док]**
- Процесс должен работать **от того же пользователя и в той же сессии**, что
  и приложение. По SSH сессионной шины нет: нужен
  `DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/$UID/bus` (и `DISPLAY` /
  `WAYLAND_DISPLAY` для ввода). Lumepeer упирался в то же на Secret Service
  по SSH. **[lumepeer: docs/platform-support.md]**
- Если GUI запущен от root (`sudo kesl-gui`), его объекты на шине доступности
  root-сеанса — пользовательский процесс их не увидит. **[проверить]**

### Включение доступности в тулкитах
- Флаг `org.a11y.Status.IsEnabled` (свойство на сессионной шине) — сигнал
  «есть ассистивная технология». `hangui a11y enable` выставляет его.
  Многие приложения читают его **только при старте** — после включения
  приложение нужно перезапустить. **[док]**
- GTK3: мост `at-spi2-atk` грузится всегда, если не задан `NO_AT_BRIDGE=1`.
  GTK4: встроенный AT-SPI бэкенд. **[док]**
- Qt5/Qt6: регистрируются, если `IsEnabled=true`, либо при
  `QT_LINUX_ACCESSIBILITY_ALWAYS_ON=1` (у старых Qt5 — `QT_ACCESSIBILITY=1`).
  **[док]** kesl-gui предположительно на Qt **[проверить в 00]**.
- Chromium/Electron: дерево веб-контента появляется с
  `--force-renderer-accessibility`; без флага — только рамка окна. **[проверить в 08]**
- Java Swing: нужен `java-atk-wrapper` — вне плана.

### Координаты
- X11: `Component.GetExtents(Screen)` даёт экранные координаты. **[док]**
- Wayland: у клиента нет глобальных координат; GTK/Qt под Wayland отдают
  координаты относительно окна или нули. Hit-test `--at X,Y` и клик по
  координатам на Wayland ненадёжны — опираться на семантические действия.
  **[проверить в 03]** Для `bounds` правило из contracts.md §1: не
  подставлять оконные координаты как экранные.

### Синтетический ввод
- Тип сеанса: `XDG_SESSION_TYPE`, затем `WAYLAND_DISPLAY`, затем `DISPLAY`;
  неизвестный считать Wayland. **[lumepeer: crates/media/src/capture/mod.rs `detect_session_type`]**
- X11: XTest через `x11rb`; keycode X11 = evdev + 8. **[lumepeer: ADR 0003, linux_x11.rs]**
- Wayland: портал `org.freedesktop.portal.RemoteDesktop` через `ashpd`
  (`notify_pointer_motion_absolute`, `notify_keyboard_keycode`,
  `notify_pointer_button`, `notify_pointer_axis`). Первое подключение
  показывает диалог согласия. **[lumepeer: ADR 0010]**
- Запомнить согласие: `persist_mode` + `restore_token` в `SelectDevices`;
  работает с xdg-desktop-portal ≥ 1.17, GNOME-бэкенд ≥ 45, KDE ≥ 6.0;
  токен одноразовый — сохранять новый после каждого `Start`. Ловушка GNOME:
  если запомненный монитор исчез, сеанс восстанавливается без потока.
  **[lumepeer: ADR 0160]**
- Cinnamon на Wayland не имеет бэкенда RemoteDesktop/ScreenCast → ошибка
  `unsupported`, совет «войти в сеанс Cinnamon (X11)». **[lumepeer: ADR 0166]**
- Абсолютные координаты в портале — в координатах потока ScreenCast, т.е.
  для ввода по координатам на Wayland нужен ещё и выбор монитора. **[lumepeer: ADR 0010]**
- `/dev/uinput` (ydotool) требует root или udev-правила — вне плана.

## Windows

- UIA-клиент: `CoInitializeEx(COINIT_MULTITHREADED)`, `CUIAutomation8`.
  Обход — `TreeWalker` (ControlView по умолчанию, RawView для `--all`).
  Пакетное чтение свойств — `CacheRequest` (иначе каждый вызов —
  межпроцессный). **[док]**
- Обработчики событий UIA нельзя добавлять/снимать из потока обработчика;
  держать отдельный поток. **[док]**
- **UIPI:** процесс со средним уровнем целостности не управляет окнами
  повышенных процессов (диспетчер задач, regedit, установщики, VMware).
  Нужен запуск от администратора или `uiAccess=true` (подписанный бинарь в
  Program Files). Ошибка — `permission_denied` с этим советом.
  **[lumepeer: ADR 0169]**
- Защищённый рабочий стол (UAC, Ctrl+Alt+Del) недоступен обычному процессу. **[lumepeer: ADR 0056]**
- DPI: процесс объявляет Per-Monitor-V2 (`SetProcessDpiAwarenessContext`),
  иначе `BoundingRectangle` и `SendInput` живут в разных пикселях. **[док]**
- Клавиши — по позиции (scan-коды), аккорды не печатать как символы.
  **[lumepeer: ADR 0065, `capture::windows::physical_key`]**
- Старые Win32/VB-приложения: часть данных только через
  `LegacyIAccessible` (MSAA) — использовать как запасной источник `name`/`value`.

## macOS

- Разрешение **Accessibility** (TCC) выдаётся бинарю, который делает вызовы;
  при запуске из Terminal/iTerm — самому терминалу. Проверка
  `AXIsProcessTrusted()`, запрос — `AXIsProcessTrustedWithOptions` с
  `kAXTrustedCheckOptionPrompt`. **[lumepeer: macos_permissions.rs]**
- Ввод через CGEvent требует того же разрешения
  (`CGPreflightPostEventAccess`). **[lumepeer: macos_permissions.rs]**
- Клавиши — по позиции: таблица evdev → `kVK_*`; `CGEventKeyboardSetUnicodeString`
  не несёт key code, аккорды ломаются. **[lumepeer: ADR 0108]**
- `AXUIElementSetMessagingTimeout` — иначе зависшее приложение вешает обход. **[док]**
- Chromium/Electron: выставить `AXManualAccessibility` (Electron) или
  `AXEnhancedUserInterface` на элементе приложения, иначе дерева нет. **[проверить в 11]**
- Координаты AX — точки, начало в левом верхнем углу главного дисплея. **[док]**
- `AXObserver` (события) требует CFRunLoop в своём потоке. **[док]**
