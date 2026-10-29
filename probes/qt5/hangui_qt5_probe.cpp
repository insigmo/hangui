// hangui in-process probe for Qt 5 applications.
//
// Loaded with LD_PRELOAD into an application that ships its own Qt (for
// example one built without the AT-SPI bridge). It registers the startup hook
// that Qt 5 exposes for tools such as GammaRay (qtHookData[QHooks::Startup]),
// then serves the application's object tree over a Unix socket:
//   $XDG_RUNTIME_DIR/hangui/qt-<pid>.sock, one JSON request per line:
//   {"cmd":"tree"}                    -> windows, Quick items, QAccessible data
//   {"cmd":"click","id":"q55d0..."}   -> QAccessible press, else a synthetic
//                                        mouse click delivered inside the process
//
// Only public Qt 5 API is used, so the probe builds against any Qt 5 headers
// and links against the target application's own Qt libraries.

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QHash>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QPointer>
#include <QtCore/QTimer>
#include <QtGui/QAccessible>
#include <QtGui/QGuiApplication>
#include <QtGui/QMouseEvent>
#include <QtGui/QWindow>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>
#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickWindow>

#include <cstdio>

// Defined in QtCore (qhooks_p.h); a global variable, so not name-mangled.
extern quintptr qtHookData[];

namespace {

// Indices from qhooks_p.h, stable across Qt 5.4+.
const int kHookDataSize = 1;
const int kStartupHook = 5;

using StartupCallback = void (*)();
StartupCallback g_previousStartup = nullptr;

QHash<QString, QPointer<QObject>> g_ids;

QString idOf(QObject *o)
{
    const QString id = QStringLiteral("q") + QString::number(quintptr(o), 16);
    g_ids.insert(id, o);
    return id;
}

QJsonObject rectJson(const QRectF &r)
{
    QJsonObject j;
    j["x"] = qRound(r.x());
    j["y"] = qRound(r.y());
    j["w"] = qRound(r.width());
    j["h"] = qRound(r.height());
    return j;
}

QJsonArray statesJson(const QAccessible::State &s)
{
    QJsonArray a;
    if (s.disabled) a.append("disabled");
    if (s.invisible) a.append("invisible");
    if (s.offscreen) a.append("offscreen");
    if (s.focusable) a.append("focusable");
    if (s.focused) a.append("focused");
    if (s.checkable) a.append("checkable");
    if (s.checked) a.append("checked");
    if (s.checkStateMixed) a.append("mixed");
    if (s.selectable) a.append("selectable");
    if (s.selected) a.append("selected");
    if (s.expandable) a.append("expandable");
    if (s.expanded) a.append("expanded");
    if (s.editable) a.append("editable");
    if (s.readOnly) a.append("read_only");
    if (s.modal) a.append("modal");
    if (s.pressed) a.append("pressed");
    return a;
}

void addAccessible(QObject *o, QJsonObject &j)
{
    QAccessibleInterface *a = QAccessible::queryAccessibleInterface(o);
    if (!a || !a->isValid())
        return;
    QJsonObject acc;
    acc["role"] = int(a->role());
    acc["name"] = a->text(QAccessible::Name);
    const QString desc = a->text(QAccessible::Description);
    if (!desc.isEmpty()) acc["description"] = desc;
    const QString value = a->text(QAccessible::Value);
    if (!value.isEmpty()) acc["value"] = value;
    acc["states"] = statesJson(a->state());
    acc["rect"] = rectJson(a->rect());
    if (QAccessibleActionInterface *act = a->actionInterface())
        acc["actions"] = QJsonArray::fromStringList(act->actionNames());
    j["acc"] = acc;
}

void addProperties(QObject *o, QJsonObject &j)
{
    static const char *const names[] = {"text", "title", "checked", "placeholderText",
                                        "currentIndex", "value", "source"};
    QJsonObject props;
    for (const char *name : names) {
        const QVariant v = o->property(name);
        if (v.isValid() && v.canConvert<QString>() && !v.toString().isEmpty())
            props[name] = v.toString();
    }
    if (!props.isEmpty())
        j["props"] = props;
}

QJsonObject itemJson(QQuickItem *it, int depth)
{
    QJsonObject j;
    j["id"] = idOf(it);
    j["class"] = it->metaObject()->className();
    if (!it->objectName().isEmpty()) j["objectName"] = it->objectName();
    j["visible"] = it->isVisible();
    j["enabled"] = it->isEnabled();
    const QPointF topLeft = it->mapToScene(QPointF(0, 0));
    j["scene"] = rectJson(QRectF(topLeft, QSizeF(it->width(), it->height())));
    addProperties(it, j);
    addAccessible(it, j);
    if (depth < 128) {
        QJsonArray children;
        for (QQuickItem *c : it->childItems())
            children.append(itemJson(c, depth + 1));
        if (!children.isEmpty())
            j["children"] = children;
    }
    return j;
}

QJsonObject tree()
{
    QJsonArray windows;
    for (QWindow *w : QGuiApplication::topLevelWindows()) {
        QJsonObject wj;
        wj["id"] = idOf(w);
        wj["class"] = w->metaObject()->className();
        wj["title"] = w->title();
        wj["visible"] = w->isVisible();
        wj["geometry"] = rectJson(w->geometry());
        if (QQuickWindow *qw = qobject_cast<QQuickWindow *>(w))
            wj["root"] = itemJson(qw->contentItem(), 0);
        windows.append(wj);
    }
    QJsonObject r;
    r["pid"] = qint64(QCoreApplication::applicationPid());
    r["app"] = QCoreApplication::applicationName();
    r["qt"] = QString::fromLatin1(qVersion());
    r["accessible_active"] = QAccessible::isActive();
    r["windows"] = windows;
    return r;
}

// Replies are filled through an out-parameter: a Qt 5.15 header makes
// returning a named QJsonObject call the move constructor, which Qt 5.9 lacks.
void error(QJsonObject &r, const QString &code, const QString &message)
{
    r["error"] = code;
    r["message"] = message;
}

void click(const QString &id, QJsonObject &r)
{
    QObject *o = g_ids.value(id);
    if (!o) {
        error(r, "stale", "no live object with id " + id + "; request a new tree");
        return;
    }
    QAccessibleInterface *a = QAccessible::queryAccessibleInterface(o);
    if (a && a->actionInterface()
        && a->actionInterface()->actionNames().contains(QAccessibleActionInterface::pressAction())) {
        a->actionInterface()->doAction(QAccessibleActionInterface::pressAction());
        r["ok"] = true;
        r["via"] = "qaccessible";
        return;
    }
    QQuickItem *it = qobject_cast<QQuickItem *>(o);
    if (!it || !it->window()) {
        error(r, "unsupported", "object has no press action and is not a Quick item in a window");
        return;
    }
    QQuickWindow *w = it->window();
    const QPointF p = it->mapToScene(QPointF(it->width() / 2, it->height() / 2));
    const QPointF g = w->mapToGlobal(p.toPoint());
    // postEvent, not sendEvent: Qt 5.9 has sendEvent only as an inline.
    QCoreApplication::postEvent(w, new QMouseEvent(QEvent::MouseButtonPress, p, p, g,
                                                   Qt::LeftButton, Qt::LeftButton, Qt::NoModifier));
    QCoreApplication::postEvent(w, new QMouseEvent(QEvent::MouseButtonRelease, p, p, g,
                                                   Qt::LeftButton, Qt::NoButton, Qt::NoModifier));
    r["ok"] = true;
    r["via"] = "synthetic_event";
}

void handle(const QByteArray &line, QJsonObject &r)
{
    const QJsonObject req = QJsonDocument::fromJson(line).object();
    const QString cmd = req.value("cmd").toString();
    if (cmd == "tree")
        r = tree();
    else if (cmd == "click")
        click(req.value("id").toString(), r);
    else
        error(r, "usage", "unknown cmd: " + cmd);
}

void startServer()
{
    // Quick items publish accessible names and roles only once accessibility
    // is active; without the AT-SPI bridge nobody else turns it on.
    QAccessible::setActive(true);

    QString dir = QString::fromLocal8Bit(qgetenv("XDG_RUNTIME_DIR"));
    if (dir.isEmpty())
        dir = QDir::tempPath();
    dir += "/hangui";
    QDir().mkpath(dir);
    const QString path = dir + "/qt-" + QString::number(QCoreApplication::applicationPid()) + ".sock";
    QLocalServer::removeServer(path);

    QLocalServer *server = new QLocalServer(QCoreApplication::instance());
    server->setSocketOptions(QLocalServer::UserAccessOption);
    if (!server->listen(path)) {
        std::fprintf(stderr, "hangui-probe: listen %s failed: %s\n", qPrintable(path),
                     qPrintable(server->errorString()));
        return;
    }
    QObject::connect(server, &QLocalServer::newConnection, server, [server]() {
        while (QLocalSocket *s = server->nextPendingConnection()) {
            QObject::connect(s, &QLocalSocket::disconnected, s, &QObject::deleteLater);
            QObject::connect(s, &QLocalSocket::readyRead, s, [s]() {
                while (s->canReadLine()) {
                    QJsonObject reply;
                    handle(s->readLine(), reply);
                    s->write(QJsonDocument(reply).toJson(QJsonDocument::Compact));
                    s->write("\n");
                }
            });
        }
    });
    std::fprintf(stderr, "hangui-probe: listening on %s (Qt %s)\n", qPrintable(path), qVersion());
}

void onStartup()
{
    if (g_previousStartup)
        g_previousStartup();
    // The hook runs inside QCoreApplication's constructor; finish the setup
    // from the event loop, once QGuiApplication is fully initialised.
    QTimer::singleShot(0, QCoreApplication::instance(), &startServer);
}

__attribute__((constructor)) void install()
{
    if (qtHookData[kHookDataSize] <= quintptr(kStartupHook))
        return;
    g_previousStartup = reinterpret_cast<StartupCallback>(qtHookData[kStartupHook]);
    qtHookData[kStartupHook] = reinterpret_cast<quintptr>(&onStartup);
}

} // namespace
