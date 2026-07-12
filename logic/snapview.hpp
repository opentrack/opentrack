#pragma once

#include "api/plugin-api.hpp"
#include "export.hpp"
#include "input/key-opts.hpp"
#include "input/shortcuts.h"
#include "options/options.hpp"

#include <QObject>
#include <QMutex>
#include <QPointF>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QList>

#include <memory>
#include <vector>

class Mappings;

struct OTR_LOGIC_EXPORT snapview_key final
{
    QString keycode;
    QString guid;
    int button = -1;

    bool is_empty() const;
};

struct OTR_LOGIC_EXPORT snapview_point final
{
    Axis axis = Yaw;
    bool alt = false;
    int index = 0;              // zero-based index inside a curve
    double x = 0;
    double y = 0;
    bool present = false;
    bool enabled = true;
    snapview_key key1;
    snapview_key key2;
};

class OTR_LOGIC_EXPORT snapview final : public QObject
{
    Q_OBJECT

public:
    static constexpr int max_points_per_curve = 64;

    static snapview& instance();
    ~snapview() override;

    void shutdown();

    void set_mappings(Mappings* mappings);

    int profile_count() const;
    int selected_profile() const;
    QString profile_name(int index) const;
    QStringList profile_names() const;
    void set_selected_profile(int index);
    int add_profile();

    QVector<snapview_point> points() const;
    void register_curve(Axis axis, bool alt, const QList<QPointF>& points);

    void update_key(Axis axis, bool alt, int index, int key_index, const snapview_key& key);
    void clear_key(Axis axis, bool alt, int index, int key_index);
    void set_enabled(Axis axis, bool alt, int index, bool enabled);

    void reload();
    void save();

    Pose apply(Pose value) const;

    static QString axis_name(Axis axis);
    static QString point_name(Axis axis, int index);
    static QString curve_name(bool alt);
    static QString key_to_string(const snapview_key& key);

signals:
    void changed();

private:
    explicit snapview(QObject* parent = nullptr);

    struct point_settings;
    using point_ptr = std::unique_ptr<point_settings>;

    options::bundle control_b;
    options::value<int> selected_profile_value;
    options::value<int> profile_count_value;
    options::bundle b;
    mutable QMutex mtx;
    std::vector<point_ptr> storage;
    std::unique_ptr<Shortcuts> shortcuts;
    Mappings* mappings = nullptr;
    bool shutting_down = false;

    void normalize_profile_state_unlocked();
    void set_profile_bundle_unlocked();
    void rebind_mappings_unlocked();
    void sync_points_from_mappings_unlocked();
    bool sync_curve_unlocked(Axis axis, bool alt, const QList<QPointF>& points);
    void migrate_legacy_profile0_unlocked();
    void copy_current_mappings_to_profile_unlocked(int profile_index);
    void copy_point_settings_to_profile_unlocked(int from_profile, int to_profile);
    void build_storage();
    int storage_index(Axis axis, bool alt, int point_index) const;
    point_settings* find_unlocked(Axis axis, bool alt, int point_index);
    const point_settings* find_unlocked(Axis axis, bool alt, int point_index) const;

    QVector<snapview_point> points_unlocked() const;
    void reload_shortcuts_unlocked();
    void set_active_group(const QString& key_id, bool held);
    void set_all_inactive_unlocked();

    QString profile_name_unlocked(int index) const;

    static QString profile_bundle_name(int index);
    static QString profile_mapping_bundle_name(int index, const QString& base_name);
    static snapview_key read_key(const key_opts& key);
    static void assign_key(key_opts& dst, const snapview_key& src);
    static QString key_id(const snapview_key& key);
    static bool key_is_valid(const key_opts& key);
    static bool key_is_valid(const snapview_key& key);
};
