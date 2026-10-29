# Пачка 12 — macOS: действия AX и CGEvent

Зависит от: 11 | Стенд: macOS

## Цель
Паритет пачек 04–05 на macOS.

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
4. Сценарий `scripts/e2e-macos.sh`: TextEdit — ввод текста, меню Format;
   Системные настройки — открыть раздел, прочитать заголовок.

## Не делать
- Обход TCC, AppleScript/`osascript` как механизм действий.

## Готово, когда
- `scripts/e2e-macos.sh` проходит 3 раза подряд; MCP из 06 работает без
  изменений в `hangui-mcp`.

## Стоп-и-спроси
- Действие требует Automation-разрешения (Apple Events) — решение за владельцем.
