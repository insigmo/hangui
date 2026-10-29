# Пачка 10 — Chromium/Electron через CDP и матрица Linux

Зависит от: 07 | Стенд: VM с несколькими DE (GNOME, KDE Plasma, XFCE, Cinnamon, MATE)

## Цель
Закрыть Electron/Chromium (их дерево a11y часто пустое) и подтвердить
«почти все графические оболочки» таблицей, а не словами.

## Прочитать
- [architecture.md](architecture.md) — «Лестница источников».
- [platform-notes.md](platform-notes.md) — Linux.
- [contracts.md](contracts.md) §1, §5.

## Задачи
1. Крейт `crates/hangui-cdp` (`chromiumoxide` 0.9): `hangui launch --cdp`
   добавляет `--remote-debugging-port=0` (порт из `DevToolsActivePort`),
   `CdpProvider`: `Accessibility.getFullAXTree` → узлы `source: "cdp"`,
   действия через `DOM`/`Input`. Проверить на VS Code и одном
   Electron-приложении; заодно — помогает ли `--force-renderer-accessibility`
   ступени a11y.
2. Сценарий `scripts/matrix.sh`: для приложения — `doctor <app>`, `tree
   --depth 4`, `invoke` безопасной кнопки, `type` в поле; результат —
   строка таблицы со ступенью, которая сработала.
3. Приложения: GTK3 (mousepad), GTK4 (gnome-text-editor), Qt5/Qt6
   (системный kwrite), Qt со своим Qt (kesl-gui), Electron (VS Code),
   Firefox.
4. Оболочки: GNOME (X11, Wayland), KDE (X11, Wayland), XFCE, Cinnamon
   (X11, Wayland), MATE. Что нельзя поставить — «не проверено», не «работает».
5. Провалы — с причиной в PROGRESS; простые фиксы — в `doctor` и platform-notes.

## Не делать
- Чинить приложения третьих сторон; поддержку Java.

## Готово, когда
- `docs/linux-matrix.md`: оболочка × приложение × (ступень, чтение,
  действие, ввод) с версиями и датой; README репозитория ссылается на неё.
- VS Code: `tree` показывает пункты меню и вкладки, `invoke` открывает панель.

## Стоп-и-спроси
- DE требует ввода не через портал/XTest (свой протокол).
