# Пачка 10 — Windows: действия UIA и SendInput

Зависит от: 09 | Стенд: Windows 11

## Цель
Паритет пачек 04–05 на Windows.

## Прочитать
- [platform-notes.md](platform-notes.md) — Windows.
- [contracts.md](contracts.md) §4, §6.
- В lumepeer для справки: `crates/media/src/capture/windows.rs`
  (`physical_key`, SendInput).

## Задачи
1. Действия §4 через паттерны: Invoke, Value/RangeValue, Toggle,
   ExpandCollapse, SelectionItem, ScrollItem, `SetFocus`;
   `LegacyIAccessible.DoDefaultAction` как запасной семантический путь.
2. `SendInput`: мышь в абсолютных координатах виртуального экрана, клавиши
   по scan-коду (таблица evdev → scan), `type` через `KEYEVENTF_UNICODE`.
3. Запасной путь `via: "input"` как на Linux; `--no-fallback`.
4. Сценарий `scripts/e2e-windows.ps1`: Блокнот — ввод текста, меню
   «Правка», `ctrl+a`; Параметры — переход в раздел и чтение заголовка.

## Не делать
- `uiAccess`-подпись и установщик — пачка 13.

## Готово, когда
- `scripts/e2e-windows.ps1` проходит 3 раза подряд; `hangui mcp` из 06
  работает на Windows без изменений в `hangui-mcp` (проверка D2).

## Стоп-и-спроси
- Нужен uiAccess или служба для повседневных сценариев.
