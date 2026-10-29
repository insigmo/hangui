# Пачка 08 — Матрица оболочек и тулкитов Linux

Зависит от: 05 | Стенд: VM с несколькими DE (GNOME, KDE Plasma, XFCE, Cinnamon, MATE)

## Цель
Подтвердить «почти все графические оболочки» таблицей, а не словами.

## Прочитать
- [platform-notes.md](platform-notes.md) — Linux.
- `scripts/e2e-kesl.sh` (из 04) как образец сценария.

## Задачи
1. Сценарий `scripts/matrix.sh`: для приложения — `apps`, `tree --depth 4`,
   `invoke` безопасной кнопки, `type` в поле, `key`; результат — строка таблицы.
2. Приложения-эталоны: GTK3 (gedit/mousepad), GTK4 (gnome-text-editor),
   Qt5/Qt6 (kate или kwrite), Electron (VS Code), Firefox, kesl-gui.
3. Оболочки: GNOME (X11, Wayland), KDE (X11, Wayland), XFCE, Cinnamon
   (X11, Wayland), MATE. Что нельзя поставить на стенд — пометить
   «не проверено», не «работает».
4. Каждый провал — в issues или в PROGRESS с причиной; простые фиксы
   (включение доступности, флаги Electron) — в `doctor` и platform-notes.

## Не делать
- Чинить приложения третьих сторон; поддержку Java.

## Готово, когда
- `docs/linux-matrix.md`: таблица оболочка × тулкит × (чтение / действие /
  ввод) с версиями и датой; README репозитория ссылается на неё.

## Стоп-и-спроси
- Какой-то DE требует ввода не через портал/XTest (свой протокол).
