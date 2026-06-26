#include "hotview-table.hpp"

#include "hotview-keybinding-dialog.hpp"
#include "logic/hotview.hpp"

#include <QAbstractItemView>
#include <QEvent>
#include <QFontMetrics>
#include <QHeaderView>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

namespace
{

constexpr int column_enabled  = 0;
constexpr int column_point    = 1;
constexpr int column_curve    = 2;
constexpr int column_input    = 3;
constexpr int column_output   = 4;
constexpr int column_shortcut = 5;
constexpr int column_bind     = 6;
constexpr int column_clear    = 7;
constexpr int column_count    = 8;

constexpr int role_sort       = Qt::UserRole + 10;
constexpr int role_axis       = Qt::UserRole + 20;
constexpr int role_alt        = Qt::UserRole + 21;
constexpr int role_index      = Qt::UserRole + 22;

class sortable_item final : public QTableWidgetItem
{
public:
    explicit sortable_item(const QString& text = {}) :
        QTableWidgetItem(text)
    {}

    bool operator<(const QTableWidgetItem& other) const override
    {
        const QVariant a = data(role_sort);
        const QVariant b = other.data(role_sort);

        if (a.isValid() || b.isValid())
            return a.toString() < b.toString();

        return QTableWidgetItem::operator<(other);
    }
};

QTableWidgetItem* readonly_item(const QString& text, const QString& sort_key = {})
{
    auto* item = new sortable_item(text);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setData(role_sort, sort_key.isEmpty() ? text : sort_key);
    return item;
}

QString enabled_sort_key(bool key_assigned, bool enabled)
{
    if (!key_assigned)
        return QStringLiteral("0");

    return enabled ? QStringLiteral("2") : QStringLiteral("1");
}

QString point_sort_key(const hotview_point& point)
{
    return QStringLiteral("%1-%2-%3")
        .arg(hotview::point_name(point.axis, point.index), 32, QLatin1Char('0'))
        .arg(point.alt ? 1 : 0)
        .arg(point.index, 8, 10, QLatin1Char('0'));
}

QString curve_sort_key(bool alt)
{
    return alt ? QStringLiteral("1") : QStringLiteral("0");
}

int text_width(QTableWidget* table, const QString& text)
{
    const QFontMetrics metrics(table->font());
    return metrics.horizontalAdvance(text) + 24;
}

} // namespace

hotview_table::hotview_table(QWidget* parent) :
    QWidget(parent)
{
    setObjectName(QStringLiteral("hotview_table"));

    auto* layout = new QVBoxLayout(this);

    table = new QTableWidget(this);
    table->setColumnCount(column_count);
    set_headers();

    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    table->horizontalHeader()->setStretchLastSection(false);
    table->horizontalHeader()->setMinimumSectionSize(1);
    table->horizontalHeader()->setSectionsClickable(true);
    table->horizontalHeader()->setSortIndicatorShown(true);
    table->horizontalHeader()->setSortIndicator(sort_column, sort_order);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSortingEnabled(false);

    layout->addWidget(table);

    connect(table, &QTableWidget::itemChanged,
            this, &hotview_table::item_changed);

    connect(table->horizontalHeader(), &QHeaderView::sectionClicked,
            this, &hotview_table::header_clicked);

    connect(&hotview::instance(), &hotview::changed,
            this, &hotview_table::rebuild);

    rebuild();
}

void hotview_table::set_headers()
{
    table->setHorizontalHeaderLabels({
        tr("Enabled"),
        tr("Point"),
        tr("Curve"),
        tr("Input"),
        tr("Output"),
        tr("Shortcut"),
        tr("Bind"),
        tr("Clear"),
    });
}

void hotview_table::reload()
{
    hotview::instance().reload();
}

void hotview_table::save()
{
    hotview::instance().save();
}

void hotview_table::rebuild()
{
    updating = true;
    table->setRowCount(0);
    set_headers();

    const QVector<hotview_point> points = hotview::instance().points();
    table->setRowCount(points.size());

    for (int row = 0; row < points.size(); row++)
    {
        const hotview_point& point = points[row];
        const bool key_assigned = !point.key.is_empty();
        const bool checked = key_assigned && point.enabled;

        auto* enabled = new sortable_item;
        Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
        if (key_assigned)
            flags |= Qt::ItemIsEnabled;
        enabled->setFlags(flags);
        enabled->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
        enabled->setData(role_sort, enabled_sort_key(key_assigned, checked));
        enabled->setData(role_axis, int(point.axis));
        enabled->setData(role_alt, point.alt);
        enabled->setData(role_index, point.index);
        table->setItem(row, column_enabled, enabled);

        table->setItem(row, column_point, readonly_item(hotview::point_name(point.axis, point.index), point_sort_key(point)));
        table->setItem(row, column_curve, readonly_item(point.alt ? tr("Alt") : tr("Main"), curve_sort_key(point.alt)));
        table->setItem(row, column_input, readonly_item(QString::number(point.x, 'f', 1), QStringLiteral("%1").arg(point.x, 20, 'f', 6, QLatin1Char('0'))));
        table->setItem(row, column_output, readonly_item(QString::number(point.y, 'f', 1), QStringLiteral("%1").arg(point.y, 20, 'f', 6, QLatin1Char('0'))));
        table->setItem(row, column_shortcut, readonly_item(key_assigned ? hotview::key_to_string(point.key) : tr("None")));

        auto* bind = new QPushButton(tr("Bind"), table);
        connect(bind, &QPushButton::clicked,
                this,
                [this, point]
                {
                    hotview_key key;

                    if (prompt_hotview_key_binding(this, key))
                        hotview::instance().update_key(point.axis, point.alt, point.index, key);
                });
        table->setCellWidget(row, column_bind, bind);

        auto* clear = new QPushButton(tr("Clear"), table);
        connect(clear, &QPushButton::clicked,
                this,
                [point]
                {
                    hotview::instance().clear_key(point.axis, point.alt, point.index);
                });
        table->setCellWidget(row, column_clear, clear);
    }

    updating = false;
    apply_sort();
    adjust_columns();

    QTimer::singleShot(0, this, &hotview_table::adjust_columns);
}

void hotview_table::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);

    if (event && event->type() == QEvent::LanguageChange)
    {
        rebuild();
        QTimer::singleShot(0, this, &hotview_table::adjust_columns);
    }
}

void hotview_table::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    adjust_columns();
}

void hotview_table::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    adjust_columns();
    QTimer::singleShot(0, this, &hotview_table::adjust_columns);
}

bool hotview_table::is_sortable_column(int column) const
{
    return column == column_enabled || column == column_point || column == column_curve;
}

void hotview_table::header_clicked(int column)
{
    if (!is_sortable_column(column))
        return;

    if (sort_column == column)
    {
        sort_order = sort_order == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
    }
    else
    {
        sort_column = column;
        sort_order = Qt::AscendingOrder;
    }

    apply_sort();
    adjust_columns();
}

void hotview_table::apply_sort()
{
    if (!table || !is_sortable_column(sort_column))
        return;

    table->horizontalHeader()->setSortIndicator(sort_column, sort_order);
    table->sortItems(sort_column, sort_order);
}

void hotview_table::adjust_columns()
{
    if (!table || table->columnCount() != column_count)
        return;

    const int total_width = table->viewport()->width();
    if (total_width <= 0)
        return;

    const QString shortcut_header = table->horizontalHeaderItem(column_shortcut)
                                      ? table->horizontalHeaderItem(column_shortcut)->text()
                                      : tr("Shortcut");

    int shortcut_width = text_width(table, shortcut_header);

    for (int row = 0; row < table->rowCount(); row++)
    {
        if (auto* item = table->item(row, column_shortcut))
            shortcut_width = std::max(shortcut_width, text_width(table, item->text()));
    }

    const int minimum_shortcut_width = 60;
    const int maximum_shortcut_width = std::max(minimum_shortcut_width, int(total_width * 0.30));
    shortcut_width = std::clamp(shortcut_width, minimum_shortcut_width, maximum_shortcut_width);
    shortcut_width = std::min(shortcut_width, total_width);

    const int other_columns = column_count - 1;
    const int remaining_width = std::max(0, total_width - shortcut_width);
    const int base_width = other_columns > 0 ? remaining_width / other_columns : 0;
    const int extra_columns = other_columns > 0 ? remaining_width - base_width * other_columns : 0;

    int extra_index = 0;
    int used_width = 0;

    for (int column = 0; column < column_count; column++)
    {
        int width = 0;

        if (column == column_shortcut)
        {
            width = shortcut_width;
        }
        else
        {
            width = base_width + (extra_index < extra_columns ? 1 : 0);
            extra_index++;
        }

        table->setColumnWidth(column, width);
        used_width += width;
    }

    const int delta = total_width - used_width;
    if (delta != 0)
    {
        int last_equal_column = column_count - 1;
        if (last_equal_column == column_shortcut)
            last_equal_column--;

        if (last_equal_column >= 0)
            table->setColumnWidth(last_equal_column, std::max(1, table->columnWidth(last_equal_column) + delta));
    }
}

void hotview_table::item_changed(QTableWidgetItem* item)
{
    if (updating || !item || item->column() != column_enabled)
        return;

    const bool key_assigned = item->flags().testFlag(Qt::ItemIsEnabled);
    if (!key_assigned)
        return;

    const Axis axis = static_cast<Axis>(item->data(role_axis).toInt());
    const bool alt = item->data(role_alt).toBool();
    const int index = item->data(role_index).toInt();
    const bool enabled = item->checkState() == Qt::Checked;

    item->setData(role_sort, enabled_sort_key(true, enabled));
    hotview::instance().set_enabled(axis, alt, index, enabled);
}
