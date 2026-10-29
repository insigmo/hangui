# Прогресс

Статусы: ⬜ не начата · 🟡 в работе (ставится и коммитится до первой правки)
· ✅ готово · ⛔ заблокирована (причина ниже).

| №  | Пачка                                         | Статус |
|----|-----------------------------------------------|--------|
| 00 | Разведка стенда Linux и kesl-gui              | ✅     |
| 01 | Скелет workspace, модель, селекторы, CLI      | ⬜     |
| 02 | Linux: чтение дерева AT-SPI                   | ⬜     |
| 03 | Linux: поиск, hit-test, фокус                 | ⬜     |
| 04 | Linux: семантические действия                 | ⬜     |
| 05 | Зонд Qt: из PoC в продукт, kesl-gui           | ⬜     |
| 06 | Linux: ввод и захват                          | ⬜     |
| 07 | Визуальный слой                               | ⬜     |
| 08 | MCP-сервер                                    | ⬜     |
| 09 | TUI-инспектор                                 | ⬜     |
| 10 | CDP и матрица Linux                           | ⬜     |
| 11 | Windows: чтение                               | ⬜     |
| 12 | Windows: действия, ввод, захват, зонд         | ⬜     |
| 13 | macOS: чтение                                 | ⬜     |
| 14 | macOS: действия, ввод, захват                 | ⬜     |
| 15 | Релиз                                         | ⬜     |
| 16 | Локальный HTTP API (`hangui serve`)           | ⬜     |
| 17 | SDK Python                                    | ⬜     |
| 18 | SDK Go                                        | ⬜     |
| 19 | SDK TypeScript/JavaScript                     | ⬜     |
| 20 | SDK C#                                        | ⬜     |
| 21 | Руководство для ИИ, публикация SDK            | ⬜     |

## Стенды

- Ubuntu VM (VMware), kesl-gui — `C:\Users\beta_win\Documents\Virtual Machines\Ubuntu 64-bit\Ubuntu 64-bit.vmx`,
  сеть VMnet8 (NAT), адрес 192.168.40.128 (DHCP, может смениться: `vmrun list` + `arp -a | grep 192.168.40.`).
  `ssh beta@192.168.40.128` по ключу `~/.ssh/id_ed25519`. Ubuntu 26.04.1, GNOME 50 на Wayland.
  sudo по паролю агент не использует — команды с root запускает владелец.
- Сборка Linux-кода и зондов — WSL Debian 13 (g++ 14, без root); заголовки Qt —
  `.deb` из архивов, распакованные в `~/hangui-deps/` (см. `docs/research/qt-probe-poc.md`).

## Журнал

<!-- Каждая пачка: ≤10 строк. Что сделано, что проверено на стенде (команда),
что не получилось, изменения contracts.md. -->

**00 (2026-10-29)** — `docs/research/linux-probe.md`, `kesl-gui-tree.txt`.
- kesl-gui (Qt 5.9.5 свой, xcb/XWayland, от `beta`) **не виден в AT-SPI**: мост не вкомпилирован; `IsEnabled` и `QT_LINUX_ACCESSIBILITY_ALWAYS_ON=1` не помогают.
- AT-SPI работает: gnome-calculator (GTK4/Wayland) — `click` и `set_text_contents` через API работают, `grab_focus` нет; экранные координаты `0,0`.
- XTest в XWayland уходит в портал RemoteDesktop (диалог) — ввод на этом стенде только через портал.
- platform-notes обновлены. Стенд: `IsEnabled=true`.

**Решение владельца (2026-10-29):** «нужно, чтобы управлялся любой GUI». Принято: лестница источников
(architecture.md) — a11y → зонд/CDP → пиксели+OCR. Доказательство зонда — `docs/research/qt-probe-poc.md`:
карта kesl-gui и нажатие «View» через `LD_PRELOAD`-зонд без портала. План перенумерован (добавлены 05, 07).
Стенд сейчас: kesl-gui запущен с зондом (юнит `hangui-kesl-gui-probe`), на странице «Protection Center».

**Решение владельца (2026-10-29):** SDK для тестов, которые пишет ИИ и которые потом идут без ИИ —
Python, Go, TS/JS, C#. Принято: D14–D17, пачки 16–21; `hangui serve` — локальный HTTP API на порту 7357
(contracts.md §10), общий контракт SDK — §11; §6 (`port_in_use`, `forbidden`) и §7 (`selector_for`)
записаны заранее, пачка 16 сверяет их с реализацией.

**Решение владельца (2026-10-29):** бинарь — без зависимостей во время выполнения, минимального размера, умеет
всё на всех ОС; сборка может качать что угодно. Принято: D18 + architecture.md «Поставка» (Linux — только glibc
2.17, не musl: musl не подгрузит libpipewire; Windows `+crt-static`; зонды вшиты сжатыми; модели OCR скачиваются
при первом `vision`; бюджет ≤ 10 МБ, CI-гейт +5%). SDK при `launch`/`start` сам находит или скачивает hangui
и пишет об этом в stderr: D19, contracts.md §11.6 (+ `hangui version`, `hangui fetch ocr` в §5). Затронуты
пачки 01, 06, 07, 12, 14, 15, 17–21.

**Решение владельца (2026-10-29):** SDK — в отдельных репозиториях (`hangui-python`, `-go`, `-js`, `-dotnet`),
не в этом. Принято: D20. hangui пишется как продукт для пакетов: HTTP API §10 — публичный контракт, набор
совместимости — `conformance/` и релизный `hangui-conformance-<версия>.tar.gz` (§11.5), `checksums.json` — в
релизе; SDK собираются против релиза, не исходников. Затронуты architecture.md «Модули», contracts.md §11,
пачки 15–21; статус 17–20 ведётся здесь.
