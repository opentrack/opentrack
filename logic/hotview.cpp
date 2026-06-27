#include "hotview.hpp"

#include <QCoreApplication>
#include <QMutexLocker>

#include <algorithm>
#include <map>
#include <utility>

namespace
{

QString axis_id(Axis axis)
{
    switch (axis)
    {
    case TX:    return QStringLiteral("x");
    case TY:    return QStringLiteral("y");
    case TZ:    return QStringLiteral("z");
    case Yaw:   return QStringLiteral("yaw");
    case Pitch: return QStringLiteral("pitch");
    case Roll:  return QStringLiteral("roll");
    default:    return QStringLiteral("unknown");
    }
}

Axis clamp_axis(int value)
{
    if (value < Axis_MIN || value > Axis_MAX)
        return Yaw;

    return static_cast<Axis>(value);
}

QString point_id(Axis axis, bool alt, int index)
{
    return QStringLiteral("%1-%2-%3")
        .arg(axis_id(axis), alt ? QStringLiteral("alt") : QStringLiteral("main"))
        .arg(index + 1);
}

} // namespace

bool hotview_key::is_empty() const
{
    return keycode.isEmpty() && guid.isEmpty();
}

struct hotview::point_settings final
{
    Axis axis = Yaw;
    bool alt = false;
    int index = 0;

    key_opts key1;
    key_opts key2;
    options::value<bool> present;
    options::value<bool> enabled;
    options::value<double> x;
    options::value<double> y;

    bool active1 = false;
    bool active2 = false;

    point_settings(options::bundle b, Axis axis_, bool alt_, int index_) :
        axis(axis_),
        alt(alt_),
        index(index_),
        // Keep the original option name for the first hotview shortcut so existing profiles keep working.
        key1(b, QStringLiteral("Snap View-%1").arg(point_id(axis_, alt_, index_))),
        key2(b, QStringLiteral("Snap View-2-%1").arg(point_id(axis_, alt_, index_))),
        present(b, QStringLiteral("present-%1").arg(point_id(axis_, alt_, index_)), false),
        enabled(b, QStringLiteral("enabled-%1").arg(point_id(axis_, alt_, index_)), false),
        x(b, QStringLiteral("x-%1").arg(point_id(axis_, alt_, index_)), 0.),
        y(b, QStringLiteral("y-%1").arg(point_id(axis_, alt_, index_)), 0.)
    {}
};

hotview& hotview::instance()
{
    static hotview self;
    return self;
}

hotview::hotview(QObject* parent) :
    QObject(parent),
    b(options::make_bundle(QStringLiteral("opentrack-Snap View")))
{
    if (QCoreApplication* app = QCoreApplication::instance())
        connect(app, &QCoreApplication::aboutToQuit,
                this, &hotview::shutdown,
                Qt::DirectConnection);

    QMutexLocker lock(&mtx);
    build_storage();
    shortcuts = std::make_unique<Shortcuts>();
    reload_shortcuts_unlocked();
}

hotview::~hotview()
{
    shutdown();
}

void hotview::shutdown()
{
    std::unique_ptr<Shortcuts> old_shortcuts;

    {
        QMutexLocker lock(&mtx);
        shutting_down = true;
        set_all_inactive_unlocked();
        old_shortcuts = std::move(shortcuts);
    }

    old_shortcuts.reset();
}

void hotview::build_storage()
{
    storage.clear();
    storage.reserve(Axis_COUNT * 2 * max_points_per_curve);

    for (int axis = Axis_MIN; axis <= Axis_MAX; axis++)
    {
        for (bool alt : { false, true })
        {
            for (int index = 0; index < max_points_per_curve; index++)
                storage.emplace_back(std::make_unique<point_settings>(b, static_cast<Axis>(axis), alt, index));
        }
    }
}

int hotview::storage_index(Axis axis, bool alt, int point_index) const
{
    const int a = int(clamp_axis(int(axis))) - Axis_MIN;
    return (a * 2 + (alt ? 1 : 0)) * max_points_per_curve + point_index;
}

hotview::point_settings* hotview::find_unlocked(Axis axis, bool alt, int point_index)
{
    if (point_index < 0 || point_index >= max_points_per_curve)
        return nullptr;

    const int idx = storage_index(axis, alt, point_index);
    if (idx < 0 || idx >= int(storage.size()))
        return nullptr;

    return storage[std::size_t(idx)].get();
}

const hotview::point_settings* hotview::find_unlocked(Axis axis, bool alt, int point_index) const
{
    return const_cast<hotview*>(this)->find_unlocked(axis, alt, point_index);
}

hotview_key hotview::read_key(const key_opts& key)
{
    return {
        key.keycode(),
        key.guid(),
        key.button(),
    };
}

void hotview::assign_key(key_opts& dst, const hotview_key& src)
{
    dst.keycode = src.keycode;
    dst.guid = src.guid;
    dst.button = src.button;
}

QString hotview::key_id(const hotview_key& key)
{
    if (!key.keycode.isEmpty())
        return QStringLiteral("kbd:%1").arg(key.keycode);

    if (!key.guid.isEmpty())
        return QStringLiteral("joy:%1:%2").arg(key.guid).arg(key.button);

    return {};
}

bool hotview::key_is_valid(const key_opts& key)
{
    return !key.keycode().isEmpty() || !key.guid().isEmpty();
}

bool hotview::key_is_valid(const hotview_key& key)
{
    return !key.keycode.isEmpty() || !key.guid.isEmpty();
}

QVector<hotview_point> hotview::points_unlocked() const
{
    QVector<hotview_point> ret;

    for (const point_ptr& ptr : storage)
    {
        const point_settings& p = *ptr;

        if (!p.present())
            continue;

        const bool has_key = key_is_valid(p.key1) || key_is_valid(p.key2);

        ret.push_back({
            p.axis,
            p.alt,
            p.index,
            p.x(),
            p.y(),
            p.present(),
            p.enabled() && has_key,
            read_key(p.key1),
            read_key(p.key2),
        });
    }

    std::sort(ret.begin(), ret.end(), [](const hotview_point& a, const hotview_point& b) {
        if (a.axis != b.axis)
            return int(a.axis) < int(b.axis);
        if (a.alt != b.alt)
            return int(a.alt) < int(b.alt);
        return a.index < b.index;
    });

    return ret;
}

QVector<hotview_point> hotview::points() const
{
    QMutexLocker lock(&mtx);
    return points_unlocked();
}

void hotview::register_curve(Axis axis, bool alt, const QVector<QPointF>& points)
{
    {
        QMutexLocker lock(&mtx);

        const int n = std::min<int>(int(points.size()), max_points_per_curve);

        for (int i = 0; i < max_points_per_curve; i++)
        {
            point_settings* p = find_unlocked(axis, alt, i);
            if (!p)
                continue;

            if (i < n)
            {
                p->present = true;
                p->x = points[i].x();
                p->y = points[i].y();
            }
            else
            {
                p->present = false;
                p->active1 = false;
                p->active2 = false;
            }
        }

        b->save();
        reload_shortcuts_unlocked();
    }

    emit changed();
}

void hotview::update_key(Axis axis, bool alt, int index, int key_index, const hotview_key& key)
{
    {
        QMutexLocker lock(&mtx);

        if (point_settings* p = find_unlocked(axis, alt, index))
        {
            assign_key(key_index == 0 ? p->key1 : p->key2, key);
            p->enabled = key_is_valid(p->key1) || key_is_valid(p->key2);
            p->active1 = false;
            p->active2 = false;
            b->save();
            reload_shortcuts_unlocked();
        }
    }

    emit changed();
}

void hotview::clear_key(Axis axis, bool alt, int index, int key_index)
{
    {
        QMutexLocker lock(&mtx);

        if (point_settings* p = find_unlocked(axis, alt, index))
        {
            assign_key(key_index == 0 ? p->key1 : p->key2, {});

            if (!key_is_valid(p->key1) && !key_is_valid(p->key2))
                p->enabled = false;

            p->active1 = false;
            p->active2 = false;
            b->save();
            reload_shortcuts_unlocked();
        }
    }

    emit changed();
}

void hotview::set_enabled(Axis axis, bool alt, int index, bool enabled)
{
    {
        QMutexLocker lock(&mtx);

        if (point_settings* p = find_unlocked(axis, alt, index))
        {
            p->enabled = enabled && (key_is_valid(p->key1) || key_is_valid(p->key2));
            p->active1 = false;
            p->active2 = false;
            b->save();
            reload_shortcuts_unlocked();
        }
    }

    emit changed();
}

void hotview::reload()
{
    {
        QMutexLocker lock(&mtx);
        b->reload();
        set_all_inactive_unlocked();
        reload_shortcuts_unlocked();
    }

    emit changed();
}

void hotview::save()
{
    QMutexLocker lock(&mtx);
    b->save();
}

void hotview::set_all_inactive_unlocked()
{
    for (point_ptr& ptr : storage)
    {
        ptr->active1 = false;
        ptr->active2 = false;
    }
}

void hotview::reload_shortcuts_unlocked()
{
    if (shutting_down)
        return;

    if (!shortcuts)
        shortcuts = std::make_unique<Shortcuts>();

    Shortcuts::t_keys keys;
    std::map<QString, key_opts*> grouped;

    auto maybe_add = [&grouped](key_opts& key)
    {
        const hotview_key k = read_key(key);
        const QString id = key_id(k);

        if (!id.isEmpty() && grouped.find(id) == grouped.end())
            grouped.emplace(id, &key);
    };

    for (point_ptr& ptr : storage)
    {
        point_settings& p = *ptr;

        if (!p.present() || !p.enabled())
            continue;

        maybe_add(p.key1);
        maybe_add(p.key2);
    }

    keys.reserve(grouped.size());

    for (const auto& [id, key] : grouped)
    {
        keys.emplace_back(*key,
                          [this, id](bool held) { set_active_group(id, held); },
                          false);
    }

    shortcuts->reload(keys);
}

void hotview::set_active_group(const QString& id, bool held)
{
    QMutexLocker lock(&mtx);

    if (shutting_down)
        return;

    for (point_ptr& ptr : storage)
    {
        point_settings& p = *ptr;

        if (!p.present() || !p.enabled())
        {
            p.active1 = false;
            p.active2 = false;
            continue;
        }

        if (key_id(read_key(p.key1)) == id)
            p.active1 = held;

        if (key_id(read_key(p.key2)) == id)
            p.active2 = held;
    }
}

Pose hotview::apply(Pose value) const
{
    QMutexLocker lock(&mtx);

    for (const point_ptr& ptr : storage)
    {
        const point_settings& p = *ptr;

        if (!p.present() || !p.enabled() || (!p.active1 && !p.active2))
            continue;

        const int axis = int(p.axis);
        if (axis < Axis_MIN || axis > Axis_MAX)
            continue;

        double out = p.y();
        if (p.alt)
            out = -out;

        value(axis) = out;
    }

    return value;
}

QString hotview::axis_name(Axis axis)
{
    switch (axis)
    {
    case TX:    return QStringLiteral("X");
    case TY:    return QStringLiteral("Y");
    case TZ:    return QStringLiteral("Z");
    case Yaw:   return QStringLiteral("Yaw");
    case Pitch: return QStringLiteral("Pitch");
    case Roll:  return QStringLiteral("Roll");
    default:    return QStringLiteral("?");
    }
}

QString hotview::point_name(Axis axis, int index)
{
    return QStringLiteral("%1%2").arg(axis_name(axis)).arg(index + 1);
}

QString hotview::curve_name(bool alt)
{
    return alt
        ? QCoreApplication::translate("hotview_table", "Alt")
        : QCoreApplication::translate("hotview_table", "Main");
}

QString hotview::key_to_string(const hotview_key& key)
{
    if (!key.guid.isEmpty())
    {
        const int modifier_mask = int(Qt::KeyboardModifierMask);
        const int button = key.button & ~modifier_mask;
        const int modifiers = key.button & modifier_mask;

        QString prefix;

        if (modifiers & int(Qt::ControlModifier))
            prefix += QStringLiteral("Control+");
        if (modifiers & int(Qt::AltModifier))
            prefix += QStringLiteral("Alt+");
        if (modifiers & int(Qt::ShiftModifier))
            prefix += QStringLiteral("Shift+");

        const QString format = key.guid == QStringLiteral("mouse")
            ? QCoreApplication::translate("options_dialog", "Mouse %1")
            : key.guid.startsWith(QStringLiteral("GI!"))
                ? QCoreApplication::translate("options_dialog", "Gamepad button %1")
                : QCoreApplication::translate("options_dialog", "Joy button %1");

        return prefix + format.arg(QString::number(button));
    }

    if (!key.keycode.isEmpty())
        return key.keycode;

    return QCoreApplication::translate("hotview_table", "None");
}
