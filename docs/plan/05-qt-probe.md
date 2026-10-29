# Пачка 05 — Зонд Qt: из PoC в продукт, kesl-gui

Зависит от: 01 (02 желательно — для проверки слияния) | Стенд: Ubuntu VM (kesl-gui) + WSL Debian для сборки

## Цель
`hangui launch -- kesl-gui` и дальше обычные `tree`/`find`/`do` дают карту и
управление kesl-gui, как для приложения с AT-SPI. PoC уже доказал путь —
теперь это часть продукта.

## Прочитать
- [architecture.md](architecture.md) — «Зонды».
- [contracts.md](contracts.md) §1, §4, §5, §6, §8, §9.
- `docs/research/qt-probe-poc.md`, `probes/qt5/hangui_qt5_probe.cpp`, `probes/qt5/build.sh`.
- `crates/hangui-core/src/` — трейт `Provider`.

## Задачи
1. Зонд `probes/qt5`: привести к протоколу §9 (`hello`, `tree`, `act`,
   `key`, `type`); JSON писать вручную в `QString` (без `QJson*`), чтобы
   одна сборка могла подойти к нескольким 5.x; `act`: `invoke` — press
   `QAccessible`, иначе сигнал `clicked()` через `QMetaObject::invokeMethod`,
   иначе пара `QMouseEvent`; `set_value` — свойство `text`/`value`; `key`/
   `type` — `QKeyEvent` в окно с фокусом. Перед действием — `enabled`/
   `visible`, иначе `disabled`.
2. Деревья `QWidget` (`QApplication::topLevelWidgets`) рядом с Quick —
   проверить на любом Qt Widgets-приложении, собранном против того же Qt
   5.9 (маленький тестовый бинарь в `probes/qt5/testapp/`, собранный тем же
   `build.sh`-окружением, с библиотеками kesl на VM).
3. Крейт `crates/hangui-probe`: определение тулкита и версии цели (по
   бинарю и его `DT_NEEDED`/`RPATH`: путь к `libQt5Core.so.5` и строка
   версии в нём), выбор сборки зонда, `hangui launch` (Linux:
   `LD_PRELOAD`, запуск без shell-обёртки, ожидание `hello`), клиент
   протокола, `ProbeProvider: Provider` (роли `QAccessible::Role` → §2,
   `class`, `source: "probe"`, `bounds` = геометрия окна + `rect`).
4. Сборка зондов: `probes/qt5/build.sh` + скрипт загрузки заголовков
   нужной версии без root (как в PoC: `.deb` из архива Ubuntu, `dpkg -x`);
   набор версий на первом шаге: 5.9.5 (kesl) и 5.15 (системный Qt5 на
   Ubuntu). Проверить гипотезу «одна сборка на 5.x без `QJson`»: собрать
   против 5.9, запустить в приложении на 5.15 — результат в PROGRESS.
5. Слияние в `hangui-core`: `apps` показывает приложения и с a11y, и с
   зондом; если у приложения есть оба — по умолчанию зонд, когда дерево
   a11y пустое.
6. `doctor kesl-gui`: «AT-SPI: нет моста (Qt 5.9.5 без atspi); зонд:
   доступен через `hangui launch`».

## Не делать
- `attach` к работающему процессу (ptrace) — после того как `launch` готов.
- Qt 6 — отдельная пачка, когда будет цель на Qt 6.
- Обход проверки целостности приложения, если оно откажется грузить зонд —
  это стоп-и-спроси.

## Готово, когда
- На VM: `hangui launch -- /opt/kaspersky/kesl/libexec/kesl-gui`, затем
  `hangui tree 'application[name="kesl-gui"]'` печатает карту из PoC в
  формате §5; `hangui do invoke '*[name="View"]'` переводит на «Protection
  Center» (проверено новым `tree`); `hangui do invoke '*[name="Reports"]'`
  при выключенной кнопке → exit 9 `disabled`.
- Тестовое Widgets-приложение: `tree`, `invoke`, `type` в `QLineEdit`.
- `cargo test` покрывает клиент протокола на записанных ответах зонда.

## Стоп-и-спроси
- Приложение (не kesl) отказывается стартовать с зондом или падает.
- Нужны права root для `launch`.
