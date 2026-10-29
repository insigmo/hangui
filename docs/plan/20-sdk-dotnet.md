# Пачка 20 — SDK для C# (.NET)

Зависит от: 16 | Стенд: Windows-машина владельца на фикстуре; Ubuntu VM для приёмки | Параллельно с 17, 18, 19

Репозиторий: **`insigmo/hangui-dotnet`** — отдельный (D20), создаёт
владелец. Пути ниже — от его корня; этот план, contracts.md и PROGRESS —
в репозитории hangui.

## Цель
ИИ пишет тест на xUnit, NUnit или MSTest: фикстура класса/сборки поднимает
или находит hangui и работает с приложением по HTTP API.

## Прочитать
- [README.md](README.md) — D14–D20.
- [contracts.md](contracts.md) §3, §4, §6, §10, §11.
- Набор совместимости из релиза hangui (§11.5).

## Задачи
1. Корень репозитория — решение с пакетом `Hangui` (NuGet), цели `net8.0` и
   `netstandard2.0`, зависимости — только `System.Text.Json` (для
   netstandard2.0). Nullable включён, XML-документация в пакете.
2. Клиент по §11: `HanguiClient.IsRunningAsync(...)`,
   `HanguiClient.ConnectAsync(options)`, `HanguiClient.StartAsync(options)`
   → `HanguiClient : IAsyncDisposable` (останавливает только свой сервер).
   `HanguiOptions { Port, Url, Token, Binary, Fixture, Args }`; пустые поля
   — из окружения и §11.1. Верхнеуровневый `Hangui.LaunchAsync(command, ...)`
   → `HanguiApp`, владеющий своим сервером (§11.2).
   Поиск и скачивание бинаря — §11.6 (`Hangui.EnsureBinaryAsync()`):
   `HttpClient`, `GZipStream`, `SHA256` (всё есть в netstandard2.0);
   `chmod +x` на Linux/macOS — `File.SetUnixFileMode` на net8.0, на
   netstandard2.0 — `chmod` через `Process`. Таблица sha256 — генерируемый при
   сборке `Checksums.g.cs` из `checksums.json` релиза hangui той же версии; сообщения — `Console.Error`.
3. Методы — по §11.3 в PascalCase с суффиксом `Async` и
   `CancellationToken`: `AppsAsync`, `LaunchAsync` → `HanguiApp`
   (`CloseAsync`, `Locator`), `TreeAsync`, `FindAsync`, `InspectAsync`,
   `WaitAsync`, `ScreenshotAsync`, `DoctorAsync`, `SelectorForAsync`;
   `Locator` с `InvokeAsync`, `ClickAsync`, `TypeAsync`, `PressAsync`,
   `SetValueAsync`, `ToggleAsync`, `FocusAsync`, `SelectAsync`,
   `ExpandAsync`, `CollapseAsync`, `WaitAsync`, `WaitGoneAsync`,
   `ExistsAsync`, `InspectAsync`, `TextAsync`.
4. Ошибки — по §11.4: `HanguiException { Code, Fix, Candidates }`,
   `enum HanguiErrorCode`, подклассы `NotFoundException`,
   `TimeoutException`, …; `HanguiNotRunningException`,
   `HanguiProtocolMismatchException`.
5. Интеграции — без зависимостей ядра от фреймворков: пакет
   `Hangui.Xunit` (`HanguiFixture : IAsyncLifetime` для `IClassFixture`/
   `ICollectionFixture`, `AppFixture`); для NUnit и MSTest — примеры
   `[OneTimeSetUp]`/`[AssemblyInitialize]` в документации, без пакетов.
   При падении — снимок и дерево в каталог результатов теста.
6. Тесты: набор совместимости на xUnit против
   `hangui serve --port 0 --fixture …`; примеры `examples/` (xUnit, NUnit,
   MSTest).
7. CI: `dotnet test` на ubuntu/windows/macos; бинарь и набор совместимости —
   из релиза hangui версии из файла `HANGUI_VERSION` (§11.5).

## Не делать
- Синхронные дубли методов (`.Result` в тестах — на совести пользователя).
- Пакеты `Hangui.NUnit`/`Hangui.MSTest` — пока хватает примеров.

## Готово, когда
- `dotnet test` зелёный на фикстуре, conformance пройден, сборка
  без предупреждений (`TreatWarningsAsErrors`).
- На стенде: xUnit-тест с `HanguiFixture` запускает kesl-gui, нажимает
  «View», ждёт результат, закрывает — без ИИ.
- На чистой VM без hangui: `dotnet test` примера скачивает бинарь с
  сообщениями §11.6 в stderr и проходит; второй прогон — из кэша; тесты
  §11.5 на скачивание проходят.

## Стоп-и-спроси
- API или набору совместимости чего-то не хватает — не обходить в SDK, а
  записать задачу для репозитория hangui (README «Общие правила»).
- Префикс `Hangui` на NuGet занят или требует резервирования — до
  публикации (21).
