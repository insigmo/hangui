# Зонд Qt внутри процесса — доказательство на kesl-gui

Дата: 2026-10-29. Стенд из пачки 00 (Ubuntu 26.04, GNOME 50/Wayland,
kesl-gui 12.5 со своим Qt 5.9.5 без моста AT-SPI). Код: `probes/qt5/`.

## Как устроен

- `hangui_qt5_probe.cpp` → `libhangui_qt5_probe.so`. Конструктор библиотеки
  ставит обработчик в `qtHookData[5]` (`QHooks::Startup`, тот же механизм,
  что у GammaRay). QtCore вызывает его из конструктора `QCoreApplication`;
  зонд откладывает запуск до цикла событий и:
  - вызывает `QAccessible::setActive(true)`;
  - слушает `$XDG_RUNTIME_DIR/hangui/qt-<pid>.sock` (`QLocalServer`, только
    владелец);
  - на `{"cmd":"tree"}` отдаёт окна (`QGuiApplication::topLevelWindows`),
    у `QQuickWindow` — дерево `QQuickItem` (класс, `objectName`, видимость,
    `enabled`, координаты в окне, свойства `text/title/checked/...`, данные
    `QAccessibleInterface`: роль, имя, состояния, действия);
  - на `{"cmd":"click","id":...}` — press через `QAccessible`, иначе пара
    `QMouseEvent` press/release в центр элемента, через `postEvent` окну.
- Сборка (`probes/qt5/build.sh`) в WSL Debian, g++ 14: заголовки Qt **5.9.5**
  из архива Ubuntu 18.04 (`qtbase5-dev_5.9.5+dfsg-0ubuntu1`,
  `qtdeclarative5-dev_5.9.5-0ubuntu1.1`), линковка с библиотеками самого
  kesl (`-Wl,--no-undefined`), `-static-libstdc++`, rpath на
  `/opt/kaspersky/kesl/lib64/gui`.
- Запуск: `systemd-run --user --setenv=LD_PRELOAD=…/libhangui_qt5_probe.so
  --setenv=FONTCONFIG_FILE=/opt/kaspersky/kesl/resource/fonts.conf
  /opt/kaspersky/kesl/libexec/kesl-gui` (не через shell-launcher, чтобы
  preload не грузился в `/bin/sh`). Журнал: `hangui-probe: listening on
  /run/user/1000/hangui/qt-13676.sock (Qt 5.9.5)`.

## Что получилось

Карта главного окна (сокращено; координаты — в окне):

```
WINDOW QQuickApplicationWindow_QML_39 'Kaspersky Endpoint Security 12.5 for Linux' {x:173,y:126,w:1000,h:580}
  QQuickStackView
    tab [NoConnection_QMLTYPE_59] focusable (SetFocus)
      "Help" [QQuickText] @16,551
      "The application is currently unavailable" [QQuickText] @362,110
      "View" [KLButton] @586,226 160x32           enabled
      "Scan" [KLInfoTab] @16,366 476x118
      "Update" [KLInfoTab] @508,366 476x118
      "Reports" [KLButton] @16,500 181x32         enabled:false
      "Backup" / "Settings" / "Temporary access" / "Support" [KLButton]
WINDOW KLConfirmationDialog… (скрыт), KLPopup (скрыт)
```

- `click` на «Reports» → `{"ok":true,"via":"synthetic_event"}`, но экран не
  изменился: кнопка `enabled:false` (приложение в состоянии «нет связи со
  службой»). **Вывод для продукта:** перед действием проверять `enabled`,
  иначе «ok» вводит в заблуждение.
- `click` на «View» → приложение перешло на страницу «Protection Center» со
  статусом «Application stopped»; следующий `tree` это показал.
- Всё на Wayland-сеансе, без портала и без диалогов.

Сырой ответ `tree` (после нажатия «View», страница «Protection Center») —
[kesl-gui-probe-tree.json](kesl-gui-probe-tree.json).

## Грабли по пути

1. Заголовки Qt 5.15 с библиотеками 5.9: линковка нашла 3 функции, которых
   нет в 5.9 (`qEnvironmentVariable`, внешний `QCoreApplication::sendEvent`,
   перемещающий конструктор `QJsonObject`) — заменены. После этого сборка
   прошла, но ответ был `{}`: inline-код `QJsonObject` из 5.15 (CBOR)
   несовместим с раскладкой 5.9. С заголовками 5.9.5 — работает. Отсюда
   правило: зонд собирается под точную минорную версию цели.
2. `qtHookData` — глобальная переменная C++, не манглится; объявляется
   `extern quintptr qtHookData[];`.
3. `QAccessible::isActive()` остаётся `false` (нет платформенного моста), но
   `queryAccessibleInterface` всё равно отдаёт роли и действия; имена у
   кастомных кнопок QML пустые — текст берётся из свойства `text`.
4. Координаты: `mapToScene` даёт координаты в окне; геометрия окна из Qt
   (`173,126`) совпала с `xwininfo` для XWayland.

## Не проверено

- Ввод текста и клавиш через зонд (`QKeyEvent`) — пачка 05.
- Виджеты (`QWidget`-деревья) — у kesl-gui только тray/QApplication; нужна
  другая цель — пачка 05.
- Qt 6, одна сборка на все 5.x.
