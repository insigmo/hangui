# Пачка 12 — Windows: действия, ввод, захват, зонд Qt

Зависит от: 11, 05, 07 | Стенд: Windows 11

## Цель
Паритет пачек 04–07 на Windows: действия, ввод, захват + визуальный слой, зонд Qt.

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
4. Захват окна: Windows.Graphics.Capture (или `PrintWindow`) → `screenshot`
   и `tree --source vision` (крейт `hangui-vision` из 07 без изменений).
   Сравнить `ocrs` с `Windows.Media.Ocr` на тех же окнах (доля подписей,
   время): не хуже — на Windows OCR ОС, модели не скачиваются
   (architecture.md «Поставка»).
5. Зонд Qt на Windows: `hangui launch` создаёт процесс приостановленным,
   загружает `hangui_qt5_probe.dll` (`LoadLibraryW` через удалённый поток),
   продолжает; протокол §9 по named pipe. Проверить на Qt-приложении без
   UIA-данных (или со скудными) — выбрать цель на машине владельца.
6. Сценарий `scripts/e2e-windows.ps1`: Блокнот — ввод текста, меню
   «Правка», `ctrl+a`; Параметры — переход в раздел и чтение заголовка;
   одно окно — через `--source vision`.

## Не делать
- `uiAccess`-подпись и установщик — пачка 15.

## Готово, когда
- `scripts/e2e-windows.ps1` проходит 3 раза подряд; `hangui mcp` из 06
  работает на Windows без изменений в `hangui-mcp` (проверка D2).

## Стоп-и-спроси
- Нужен uiAccess или служба для повседневных сценариев.
