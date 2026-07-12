#include "snapview.hpp"

#include "mappings.hpp"

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

QString default_profile_name(int index)
{
    if (index == 0)
        return QCoreApplication::translate("snapview", "Default");

    return QCoreApplication::translate("snapview", "Profile %1").arg(index + 1);
}

} // namespace

bool snapview_key::is_empty() const
{
    return keycode.isEmpty() && guid.isEmpty();
}

struct snapview::point_settings final
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
        key1(b, QStringLiteral("snap-view-key-%1").arg(point_id(axis_, alt_, index_))),
        key2(b, QStringLiteral("snap-view-key-2-%1").arg(point_id(axis_, alt_, index_))),
        present(b, QStringLiteral("present-%1").arg(point_id(axis_, alt_, index_)), false),
        enabled(b, QStringLiteral("enabled-%1").arg(point_id(axis_, alt_, index_)), false),
        x(b, QStringLiteral("x-%1").arg(point_id(axis_, alt_, index_)), 0.),
        y(b, QStringLiteral("y-%1").arg(point_id(axis_, alt_, index_)), 0.)
    {}
};

snapview& snapview::instance()
{
    static snapview self;
    return self;
}

snapview::snapview(QObject* parent) :
    QObject(parent),
    control_b(options::make_bundle(QStringLiteral("opentrack-snap-view"))),
    selected_profile_value(control_b, QStringLiteral("selected-profile"), 0),
    profile_count_value(control_b, QStringLiteral("profile-count"), 1),
    b(options::make_bundle(profile_bundle_name(0)))
{
    if (QCoreApplication* app = QCoreApplication::instance())
        connect(app, &QCoreApplication::aboutToQuit,
                this, &snapview::shutdown,
                Qt::DirectConnection);

    QMutexLocker lock(&mtx);
    normalize_profile_state_unlocked();
    set_profile_bundle_unlocked();
    build_storage();
    migrate_legacy_profile0_unlocked();
    shortcuts = std::make_unique<Shortcuts>();
    reload_shortcuts_unlocked();
}

snapview::~snapview()
{
    shutdown();
}

void snapview::shutdown()
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

QString snapview::profile_bundle_name(int index)
{
    return QStringLiteral("opentrack-snap-view-profile-%1").arg(std::max(0, index));
}

QString snapview::profile_mapping_bundle_name(int index, const QString& base_name)
{
    // Profile 0 is deliberately the existing mapping configuration so current
    // opentrack profiles keep their mapping curves unchanged.
    if (index <= 0)
        return base_name;

    return QStringLiteral("snap-view-profile-%1-%2").arg(index).arg(base_name);
}

void snapview::normalize_profile_state_unlocked()
{
    int count = std::max(1, int(profile_count_value()));
    if (count != int(profile_count_value()))
        profile_count_value = count;

    int selected = std::clamp(int(selected_profile_value()), 0, count - 1);
    if (selected != int(selected_profile_value()))
        selected_profile_value = selected;
}

void snapview::set_profile_bundle_unlocked()
{
    b = options::make_bundle(profile_bundle_name(selected_profile_value()));
}

QString snapview::profile_name_unlocked(int index) const
{
    if (index < 0 || index >= int(profile_count_value()))
        return {};

    options::value<QString> name(control_b,
                                 QStringLiteral("profile-%1-name").arg(index),
                                 default_profile_name(index));
    return name();
}

int snapview::profile_count() const
{
    QMutexLocker lock(&mtx);
    return std::max(1, int(profile_count_value()));
}

int snapview::selected_profile() const
{
    QMutexLocker lock(&mtx);
    return std::clamp(int(selected_profile_value()), 0, std::max(1, int(profile_count_value())) - 1);
}

QString snapview::profile_name(int index) const
{
    QMutexLocker lock(&mtx);
    return profile_name_unlocked(index);
}

QStringList snapview::profile_names() const
{
    QMutexLocker lock(&mtx);

    QStringList names;
    const int count = std::max(1, int(profile_count_value()));
    names.reserve(count);

    for (int i = 0; i < count; i++)
        names.push_back(profile_name_unlocked(i));

    return names;
}

void snapview::set_mappings(Mappings* new_mappings)
{
    {
        QMutexLocker lock(&mtx);
        mappings = new_mappings;
        rebind_mappings_unlocked();
        sync_points_from_mappings_unlocked();
    }

    emit changed();
}

void snapview::rebind_mappings_unlocked()
{
    if (!mappings)
        return;

    const int profile = std::clamp(int(selected_profile_value()), 0, std::max(1, int(profile_count_value())) - 1);

    for (int axis_index = Axis_MIN; axis_index <= Axis_MAX; axis_index++)
    {
        Map& axis = (*mappings)(axis_index);

        axis.spline_main.set_bundle(options::make_bundle(profile_mapping_bundle_name(profile, axis.name)),
                                    axis.opts.prefix(),
                                    axis.opts.axis());

        axis.spline_alt.set_bundle(options::make_bundle(profile_mapping_bundle_name(profile, axis.alt_name)),
                                   axis.opts.prefix(),
                                   axis.opts.axis());
    }
}


bool snapview::sync_curve_unlocked(Axis axis, bool alt, const QList<QPointF>& points)
{
    bool changed = false;
    const int n = std::min<int>(int(points.size()), max_points_per_curve);

    for (int i = 0; i < max_points_per_curve; i++)
    {
        point_settings* p = find_unlocked(axis, alt, i);
        if (!p)
            continue;

        if (i < n)
        {
            const double x = points[i].x();
            const double y = points[i].y();

            if (!p->present())
            {
                p->present = true;
                changed = true;
            }

            if (p->x() != x)
            {
                p->x = x;
                changed = true;
            }

            if (p->y() != y)
            {
                p->y = y;
                changed = true;
            }
        }
        else
        {
            if (p->present())
            {
                p->present = false;
                changed = true;
            }

            if (p->active1 || p->active2)
            {
                p->active1 = false;
                p->active2 = false;
                changed = true;
            }
        }
    }

    return changed;
}

void snapview::sync_points_from_mappings_unlocked()
{
    if (!mappings)
        return;

    bool changed = false;

    for (int axis_index = Axis_MIN; axis_index <= Axis_MAX; axis_index++)
    {
        Map& axis = (*mappings)(axis_index);
        changed |= sync_curve_unlocked(static_cast<Axis>(axis_index), false, axis.spline_main.get_points());
        changed |= sync_curve_unlocked(static_cast<Axis>(axis_index), true,  axis.spline_alt.get_points());
    }

    if (changed)
    {
        b->save();
        reload_shortcuts_unlocked();
    }
}

void snapview::migrate_legacy_profile0_unlocked()
{
    if (int(selected_profile_value()) != 0)
        return;

    options::value<bool> migrated(control_b, QStringLiteral("legacy-profile-0-migrated"), false);
    if (migrated())
        return;

    struct legacy_point_settings final
    {
        key_opts key1;
        key_opts key2;
        options::value<bool> present;
        options::value<bool> enabled;
        options::value<double> x;
        options::value<double> y;

        legacy_point_settings(options::bundle b, Axis axis, bool alt, int index) :
            key1(b, QStringLiteral("Snap View-%1").arg(point_id(axis, alt, index))),
            key2(b, QStringLiteral("Snap View-2-%1").arg(point_id(axis, alt, index))),
            present(b, QStringLiteral("present-%1").arg(point_id(axis, alt, index)), false),
            enabled(b, QStringLiteral("enabled-%1").arg(point_id(axis, alt, index)), false),
            x(b, QStringLiteral("x-%1").arg(point_id(axis, alt, index)), 0.),
            y(b, QStringLiteral("y-%1").arg(point_id(axis, alt, index)), 0.)
        {}
    };

    options::bundle legacy_b = options::make_bundle(QStringLiteral("opentrack-Snap View"));
    bool changed = false;

    for (int axis = Axis_MIN; axis <= Axis_MAX; axis++)
    {
        for (bool alt : { false, true })
        {
            for (int index = 0; index < max_points_per_curve; index++)
            {
                point_settings* dst = find_unlocked(static_cast<Axis>(axis), alt, index);
                if (!dst)
                    continue;

                legacy_point_settings src(legacy_b, static_cast<Axis>(axis), alt, index);
                const bool src_has_key1 = key_is_valid(src.key1);
                const bool src_has_key2 = key_is_valid(src.key2);
                const bool src_has_key = src_has_key1 || src_has_key2;

                if (!src.present() && !src_has_key)
                    continue;

                if (!dst->present() && src.present())
                {
                    dst->present = true;
                    dst->x = src.x();
                    dst->y = src.y();
                    changed = true;
                }

                if (!key_is_valid(dst->key1) && src_has_key1)
                {
                    assign_key(dst->key1, read_key(src.key1));
                    changed = true;
                }

                if (!key_is_valid(dst->key2) && src_has_key2)
                {
                    assign_key(dst->key2, read_key(src.key2));
                    changed = true;
                }

                if ((src_has_key1 || src_has_key2) && bool(dst->enabled()) != bool(src.enabled()))
                {
                    dst->enabled = src.enabled();
                    changed = true;
                }
            }
        }
    }

    migrated = true;
    control_b->save();

    if (changed)
        b->save();
}

void snapview::copy_current_mappings_to_profile_unlocked(int profile_index)
{
    if (!mappings || profile_index <= 0)
        return;

    for (int axis_index = Axis_MIN; axis_index <= Axis_MAX; axis_index++)
    {
        Map& axis = (*mappings)(axis_index);

        auto copy_points = [&](const spline& src, const QString& base_name)
        {
            options::bundle dst_bundle = options::make_bundle(profile_mapping_bundle_name(profile_index, base_name));
            options::value<QList<QPointF>> dst_points(dst_bundle, QStringLiteral("points"), {});
            dst_points = src.get_points();
            dst_bundle->save();
        };

        copy_points(axis.spline_main, axis.name);
        copy_points(axis.spline_alt, axis.alt_name);
    }
}

void snapview::copy_point_settings_to_profile_unlocked(int from_profile, int to_profile)
{
    if (from_profile < 0 || to_profile < 0 || from_profile == to_profile)
        return;

    options::bundle src_bundle = options::make_bundle(profile_bundle_name(from_profile));
    options::bundle dst_bundle = options::make_bundle(profile_bundle_name(to_profile));

    for (int axis = Axis_MIN; axis <= Axis_MAX; axis++)
    {
        for (bool alt : { false, true })
        {
            for (int index = 0; index < max_points_per_curve; index++)
            {
                point_settings src(src_bundle, static_cast<Axis>(axis), alt, index);
                point_settings dst(dst_bundle, static_cast<Axis>(axis), alt, index);

                assign_key(dst.key1, read_key(src.key1));
                assign_key(dst.key2, read_key(src.key2));
                dst.present = src.present();
                dst.enabled = src.enabled();
                dst.x = src.x();
                dst.y = src.y();
            }
        }
    }

    dst_bundle->save();
}

void snapview::set_selected_profile(int index)
{
    {
        QMutexLocker lock(&mtx);
        normalize_profile_state_unlocked();

        const int count = std::max(1, int(profile_count_value()));
        const int selected = std::clamp(index, 0, count - 1);

        if (selected == int(selected_profile_value()))
            return;

        if (shortcuts)
        {
            Shortcuts::t_keys no_keys;
            shortcuts->reload(no_keys);
        }

        set_all_inactive_unlocked();
        selected_profile_value = selected;
        set_profile_bundle_unlocked();
        build_storage();
        migrate_legacy_profile0_unlocked();
        rebind_mappings_unlocked();
        sync_points_from_mappings_unlocked();
        control_b->save();
        reload_shortcuts_unlocked();
    }

    emit changed();
}

int snapview::add_profile()
{
    int index = 0;

    {
        QMutexLocker lock(&mtx);
        normalize_profile_state_unlocked();

        const int old_selected = int(selected_profile_value());
        index = std::max(1, int(profile_count_value()));

        copy_current_mappings_to_profile_unlocked(index);
        copy_point_settings_to_profile_unlocked(old_selected, index);

        options::value<QString> name(control_b,
                                     QStringLiteral("profile-%1-name").arg(index),
                                     default_profile_name(index));
        name = default_profile_name(index);

        if (shortcuts)
        {
            Shortcuts::t_keys no_keys;
            shortcuts->reload(no_keys);
        }

        set_all_inactive_unlocked();
        profile_count_value = index + 1;
        selected_profile_value = index;
        set_profile_bundle_unlocked();
        build_storage();
        migrate_legacy_profile0_unlocked();
        rebind_mappings_unlocked();
        sync_points_from_mappings_unlocked();
        control_b->save();
        reload_shortcuts_unlocked();
    }

    emit changed();
    return index;
}

void snapview::build_storage()
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

int snapview::storage_index(Axis axis, bool alt, int point_index) const
{
    const int a = int(clamp_axis(int(axis))) - Axis_MIN;
    return (a * 2 + (alt ? 1 : 0)) * max_points_per_curve + point_index;
}

snapview::point_settings* snapview::find_unlocked(Axis axis, bool alt, int point_index)
{
    if (point_index < 0 || point_index >= max_points_per_curve)
        return nullptr;

    const int idx = storage_index(axis, alt, point_index);
    if (idx < 0 || idx >= int(storage.size()))
        return nullptr;

    return storage[std::size_t(idx)].get();
}

const snapview::point_settings* snapview::find_unlocked(Axis axis, bool alt, int point_index) const
{
    return const_cast<snapview*>(this)->find_unlocked(axis, alt, point_index);
}

snapview_key snapview::read_key(const key_opts& key)
{
    return {
        key.keycode(),
        key.guid(),
        key.button(),
    };
}

void snapview::assign_key(key_opts& dst, const snapview_key& src)
{
    dst.keycode = src.keycode;
    dst.guid = src.guid;
    dst.button = src.button;
}

QString snapview::key_id(const snapview_key& key)
{
    if (!key.keycode.isEmpty())
        return QStringLiteral("kbd:%1").arg(key.keycode);

    if (!key.guid.isEmpty())
        return QStringLiteral("joy:%1:%2").arg(key.guid).arg(key.button);

    return {};
}

bool snapview::key_is_valid(const key_opts& key)
{
    return !key.keycode().isEmpty() || !key.guid().isEmpty();
}

bool snapview::key_is_valid(const snapview_key& key)
{
    return !key.keycode.isEmpty() || !key.guid.isEmpty();
}

QVector<snapview_point> snapview::points_unlocked() const
{
    QVector<snapview_point> ret;

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

    std::sort(ret.begin(), ret.end(), [](const snapview_point& a, const snapview_point& b) {
        if (a.axis != b.axis)
            return int(a.axis) < int(b.axis);
        if (a.alt != b.alt)
            return int(a.alt) < int(b.alt);
        return a.index < b.index;
    });

    return ret;
}

QVector<snapview_point> snapview::points() const
{
    QMutexLocker lock(&mtx);
    const_cast<snapview*>(this)->sync_points_from_mappings_unlocked();
    return points_unlocked();
}

void snapview::register_curve(Axis axis, bool alt, const QList<QPointF>& points)
{
    bool curve_changed = false;

    {
        QMutexLocker lock(&mtx);
        curve_changed = sync_curve_unlocked(axis, alt, points);

        if (curve_changed)
        {
            b->save();
            reload_shortcuts_unlocked();
        }
    }

    if (curve_changed)
        emit changed();
}

void snapview::update_key(Axis axis, bool alt, int index, int key_index, const snapview_key& key)
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

void snapview::clear_key(Axis axis, bool alt, int index, int key_index)
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

void snapview::set_enabled(Axis axis, bool alt, int index, bool enabled)
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

void snapview::reload()
{
    {
        QMutexLocker lock(&mtx);
        control_b->reload();
        normalize_profile_state_unlocked();
        set_profile_bundle_unlocked();
        b->reload();
        build_storage();
        migrate_legacy_profile0_unlocked();
        rebind_mappings_unlocked();
        sync_points_from_mappings_unlocked();
        set_all_inactive_unlocked();
        reload_shortcuts_unlocked();
    }

    emit changed();
}

void snapview::save()
{
    QMutexLocker lock(&mtx);
    control_b->save();
    b->save();
}

void snapview::set_all_inactive_unlocked()
{
    for (point_ptr& ptr : storage)
    {
        ptr->active1 = false;
        ptr->active2 = false;
    }
}

void snapview::reload_shortcuts_unlocked()
{
    if (shutting_down)
        return;

    if (!shortcuts)
        shortcuts = std::make_unique<Shortcuts>();

    Shortcuts::t_keys keys;
    std::map<QString, key_opts*> grouped;

    auto maybe_add = [&grouped](key_opts& key)
    {
        const snapview_key k = read_key(key);
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

void snapview::set_active_group(const QString& id, bool held)
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

Pose snapview::apply(Pose value) const
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

QString snapview::axis_name(Axis axis)
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

QString snapview::point_name(Axis axis, int index)
{
    return QStringLiteral("%1%2").arg(axis_name(axis)).arg(index + 1);
}

QString snapview::curve_name(bool alt)
{
    return alt
        ? QCoreApplication::translate("snapview_table", "Alt")
        : QCoreApplication::translate("snapview_table", "Main");
}

QString snapview::key_to_string(const snapview_key& key)
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

    return QCoreApplication::translate("snapview_table", "None");
}

