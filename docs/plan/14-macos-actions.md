# Пачка 14 — macOS: действия, ввод, захват

Зависит от: 13, 07 | Стенд: macOS

## Цель
Паритет пачек 04, 06, 07 на macOS: действия, ввод, захват + визуальный слой.

## Прочитать
- [platform-notes.md](platform-notes.md) — macOS.
- [contracts.md](contracts.md) §4, §6.
- В lumepeer для справки: `docs/adr/0108-a-macos-host-presses-keys-by-position.md`,
  `crates/media/src/capture/macos_keys.rs`.

## Задачи
1. Действия §4: `AXPress`, `AXValue` (set), `AXFocused` (set),
   `AXShowMenu`, `AXRaise`, раскрытие через `AXDisclosing`/`AXExpanded`,
   выбор через `AXSelected`/`AXSelectedRows`.
2. CGEvent: мышь (точки, верхний левый угол главного дисплея), клавиши по
   позиции (evdev → `kVK_*`), `type` через Unicode-строку события только для
   текста, не для аккордов.
3. Запасной путь `via: "input"`; `--no-fallback`.
4. Захват окна: ScreenCaptureKit (разрешение Screen Recording — в `doctor`)
   → `screenshot` и `tree --source vision`. Сравнить `ocrs` с Vision
   (`VNRecognizeTextRequest`): не хуже — на macOS OCR ОС, модели не
   скачиваются (architecture.md «Поставка»).
5. Зонд — только для приложений без hardened runtime
   (`DYLD_INSERT_LIBRARIES`); остальным — ступени a11y и vision. `doctor`
   говорит, какой случай.
6. Сценарий `scripts/e2e-macos.sh`: TextEdit — ввод текста, меню Format;
   Системные настройки — открыть раздел, прочитать заголовок; одно окно —
   через `--source vision`.

## Не делать
- Обход TCC, AppleScript/`osascript` как механизм действий.

## Готово, когда
- `scripts/e2e-macos.sh` проходит 3 раза подряд; MCP из 08 работает без
  изменений в `hangui-mcp`.

## Стоп-и-спроси
- Действие требует Automation-разрешения (Apple Events) — решение за владельцем.
