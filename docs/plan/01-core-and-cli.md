# Пачка 01 — Скелет workspace, модель, селекторы, CLI на фикстурах

Зависит от: — | Стенд: любая ОС (Windows-машина владельца) | Можно параллельно с 00

## Цель
Ядро без ОС: модель узла, роли, селекторы, рендер text/json, коды ошибок,
трейт `Provider` и провайдер-фикстура. После пачки CLI полностью работает на
JSON-фикстуре — платформенным пачкам остаётся реализовать трейт.

## Прочитать
- [README.md](README.md) — решения D2, D6–D9, D18.
- [architecture.md](architecture.md) — «Поставка».
- [contracts.md](contracts.md) — целиком.

## Задачи
1. Workspace: `Cargo.toml` (resolver 3, edition 2024), `rust-toolchain.toml`
   (stable), крейты `crates/hangui-core`, `apps/hangui`. Пустые
   `hangui-linux/windows/macos` не создавать — их создаёт своя пачка.
2. `hangui-core`: `Node`, `Role`, `State`, `Bounds`, `Action`,
   `ActionOutcome`, `Error` (коды §6) с `serde`; трейт `Provider` (§8) —
   зафиксировать сигнатуры и обновить §8.
3. Селекторы: парсер (§3) с понятными ошибками (позиция + ожидание) и
   матчер поверх `Provider` (ленивый обход: не читать всё дерево ради
   `>` у корня).
4. Рендер: текстовое дерево (§5) и JSON (§1).
5. `FixtureProvider`: читает JSON-дерево; `perform` пишет действие в журнал
   и меняет состояние фикстуры (toggle → `checked`).
6. CLI (`clap` 4.6, derive): `apps`, `tree`, `find`, `inspect <selector>`,
   `do`, скрытый `--fixture`. Остальные подкоманды из §5 не объявлять до
   своих пачек.
7. Фикстура `tests/fixtures/sample-app.json` (окно, вкладки, кнопки, поле
   ввода, чекбокс, одна выключенная кнопка); вторая — карта kesl-gui,
   переведённая в §1 из `docs/research/kesl-gui-probe-tree.json`
   (`source: "probe"`).
8. Поля `source`/`confidence` и ошибка `disabled` (§1, §6) — с первого дня:
   `do invoke` по выключенному узлу фикстуры → exit 9.
9. Размер (D18): `[profile.release]` из architecture.md «Поставка»;
   зависимости с `default-features = false`; `hangui version` (§5, поле
   `protocol` = 1). Размер release-бинаря на Windows и в WSL (Linux) — в
   PROGRESS как точка отсчёта бюджета.

## Не делать
- Никакого кода AT-SPI/UIA/AX, никаких `cfg(target_os)`.
- Без async в ядре (D6), без собственного языка запросов сверх §3.

## Готово, когда
- `cargo test --workspace` покрывает: парсер (валидные/невалидные селекторы
  из §3), матчер на фикстуре (`>` vs `>>`, `:nth`, `~=`), `ambiguous`,
  `not_found`, рендер text и json (snapshot-тест строкой).
- `hangui --fixture tests/fixtures/sample-app.json tree` печатает дерево в
  формате §5; `do toggle '<selector>'` меняет `checked`; коды выхода — по §6.
- `hangui version --format json` печатает `version`, `protocol`, `os`, `arch`.
- fmt/clippy/test чисты.

## Стоп-и-спроси
- Грамматика §3 не позволяет выразить нужный случай — предложить правку
  контракта, не расширять молча.
