# Пачка 13 — macOS: чтение дерева AX, разрешения

Зависит от: 01, 03 | Стенд: macOS (VM или Mac; доступ — от владельца)

## Цель
Паритет пачек 02–03 на macOS.

## Прочитать
- [platform-notes.md](platform-notes.md) — macOS.
- [contracts.md](contracts.md) §1, §2, §3, §6, §8.
- В lumepeer для справки: `crates/media/src/capture/macos_permissions.rs`.

## Задачи
1. Крейт `crates/hangui-macos` (`objc2-application-services` 0.3 + тонкий
   `extern "C"` для недостающего). `AXUIElementSetMessagingTimeout`.
2. `apps()`: запущенные приложения (`NSWorkspace.runningApplications` с
   regular activation policy) → `AXUIElementCreateApplication(pid)`.
3. Атрибуты: `AXRole`/`AXSubrole` → §2 (тест покрытия), `AXTitle`,
   `AXDescription`, `AXValue`, `AXIdentifier`, `AXPosition`/`AXSize`,
   `AXEnabled`/`AXFocused`; `AXActionNames` → действия.
4. Пакетное чтение `AXUIElementCopyMultipleAttributeValues`.
5. Hit-test `AXUIElementCopyElementAtPosition`, фокус
   `AXFocusedUIElement`; события через `AXObserver` в потоке с CFRunLoop.
6. `doctor`: `AXIsProcessTrusted`; `hangui doctor --request` вызывает
   системный запрос разрешения. Без разрешения любая команда →
   `permission_denied` с путём в Системных настройках.
7. Chromium/Electron: выставление `AXManualAccessibility` /
   `AXEnhancedUserInterface` — проверить на одном Electron-приложении.

## Не делать
- Действия, ввод, захват — пачка 14. Подпись и нотаризация — 15.

## Готово, когда
- `tree` для Finder, Системных настроек, TextEdit и одного Electron-
  приложения; `watch --hover` в Finder. Выводы в PROGRESS.

## Стоп-и-спроси
- Нет доступа к Mac.
