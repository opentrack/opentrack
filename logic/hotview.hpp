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
#include <QVector>

#include <array>
#include <memory>
#include <vector>

struct OTR_LOGIC_EXPORT hotview_key final
{
    QString keycode;
    QString guid;
    int button = -1;

    bool is_empty() const;
};

struct OTR_LOGIC_EXPORT hotview_point final
{
    Axis axis = Yaw;
    bool alt = false;
    int index = 0;              // zero-based index inside a curve
    double x = 0;
    double y = 0;
    bool present = false;
    bool enabled = true;
    hotview_key key;
};

class OTR_LOGIC_EXPORT hotview final : public QObject
{
    Q_OBJECT

public:
    static constexpr int max_points_per_curve = 64;

    static hotview& instance();
    ~hotview() override;

    void shutdown();

    QVector<hotview_point> points() const;
    void register_curve(Axis axis, bool alt, const QVector<QPointF>& points);

    void update_key(Axis axis, bool alt, int index, const hotview_key& key);
    void clear_key(Axis axis, bool alt, int index);
    void set_enabled(Axis axis, bool alt, int index, bool enabled);

    void reload();
    void save();

    Pose apply(Pose value) const;

    static QString axis_name(Axis axis);
    static QString point_name(Axis axis, int index);
    static QString curve_name(bool alt);
    static QString key_to_string(const hotview_key& key);

signals:
    void changed();

private:
    explicit hotview(QObject* parent = nullptr);

    struct point_settings;
    using point_ptr = std::unique_ptr<point_settings>;

    options::bundle b;
    mutable QMutex mtx;
    std::vector<point_ptr> storage;
    std::unique_ptr<Shortcuts> shortcuts;
    bool shutting_down = false;

    void build_storage();
    int storage_index(Axis axis, bool alt, int point_index) const;
    point_settings* find_unlocked(Axis axis, bool alt, int point_index);
    const point_settings* find_unlocked(Axis axis, bool alt, int point_index) const;

    QVector<hotview_point> points_unlocked() const;
    void reload_shortcuts_unlocked();
    void set_active_group(const QString& key_id, bool held);
    void set_all_inactive_unlocked();

    static hotview_key read_key(const key_opts& key);
    static void assign_key(key_opts& dst, const hotview_key& src);
    static QString key_id(const hotview_key& key);
    static bool key_is_valid(const key_opts& key);
};
