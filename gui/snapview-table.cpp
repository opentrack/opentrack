#include "snapview-table.hpp"

#include "snapview-keybinding-dialog.hpp"
#include "logic/snapview.hpp"

#include <QAbstractItemView>
#include <QComboBox>
#include <QEvent>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QStringList>
#include <QStyle>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>
#include <QVector>

#include <algorithm>
#include <array>
#include <initializer_list>

namespace
{

constexpr int column_enabled   = 0;
constexpr int column_point     = 1;
constexpr int column_curve     = 2;
constexpr int column_input     = 3;
constexpr int column_output    = 4;
constexpr int column_shortcut1 = 5;
constexpr int column_bind1     = 6;
constexpr int column_clear1    = 7;
constexpr int column_shortcut2 = 8;
constexpr int column_bind2     = 9;
constexpr int column_clear2    = 10;
constexpr int column_count     = 11;

constexpr int role_sort        = Qt::UserRole + 10;
constexpr int role_axis        = Qt::UserRole + 20;
constexpr int role_alt         = Qt::UserRole + 21;
constexpr int role_index       = Qt::UserRole + 22;

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

QString point_sort_key(const snapview_point& point)
{
    return QStringLiteral("%1-%2-%3")
        .arg(snapview::point_name(point.axis, point.index), 32, QLatin1Char('0'))
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

int compact_text_width(QTableWidget* table, const QString& text)
{
    const QFontMetrics metrics(table->font());
    return metrics.horizontalAdvance(text) + 12;
}

bool is_compact_content_column(int column)
{
    return column == column_point || column == column_curve ||
           column == column_input || column == column_output;
}

bool is_shortcut_column(int column)
{
    return column == column_shortcut1 || column == column_shortcut2;
}

bool is_button_column(int column)
{
    return column == column_bind1 || column == column_clear1 ||
           column == column_bind2 || column == column_clear2;
}

} // namespace

snapview_table::snapview_table(QWidget* parent) :
    QWidget(parent)
{
    setObjectName(QStringLiteral("snapview_table"));

    auto* layout = new QVBoxLayout(this);

    auto* profile_layout = new QHBoxLayout;
    auto* profile_label = new QLabel(tr("Profile"), this);
    profile_combo = new QComboBox(this);
    add_profile_button = new QPushButton(tr("Add"), this);

    profile_layout->addWidget(profile_label);
    profile_layout->addWidget(profile_combo, 1);
    profile_layout->addWidget(add_profile_button);
    layout->addLayout(profile_layout);

    connect(profile_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &snapview_table::profile_changed);

    connect(add_profile_button, &QPushButton::clicked,
            this, &snapview_table::add_profile);

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
            this, &snapview_table::item_changed);

    connect(table->horizontalHeader(), &QHeaderView::sectionClicked,
            this, &snapview_table::header_clicked);

    connect(&snapview::instance(), &snapview::changed,
            this, &snapview_table::rebuild);

    rebuild();
}

void snapview_table::set_headers()
{
    table->setHorizontalHeaderLabels({
        QString(),
        tr("Point"),
        tr("Curve"),
        tr("Input"),
        tr("Output"),
        tr("Shortcut"),
        tr("Bind"),
        tr("Clear"),
        tr("Shortcut"),
        tr("Bind"),
        tr("Clear"),
    });

    if (QTableWidgetItem* enabled = table->horizontalHeaderItem(column_enabled))
    {
        enabled->setToolTip(tr("Enabled"));
        enabled->setCheckState(Qt::Unchecked);
    }
}

void snapview_table::reload()
{
    snapview::instance().reload();
}

void snapview_table::save()
{
    snapview::instance().save();
}

void snapview_table::rebuild()
{
    updating = true;
    table->setRowCount(0);
    set_headers();
    rebuild_profile_selector();

    const QVector<snapview_point> points = snapview::instance().points();
    table->setRowCount(points.size());

    for (int row = 0; row < points.size(); row++)
    {
        const snapview_point& point = points[row];
        const bool key1_assigned = !point.key1.is_empty();
        const bool key2_assigned = !point.key2.is_empty();
        const bool key_assigned = key1_assigned || key2_assigned;
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

        table->setItem(row, column_point, readonly_item(snapview::point_name(point.axis, point.index), point_sort_key(point)));
        table->setItem(row, column_curve, readonly_item(point.alt ? tr("Alt") : tr("Main"), curve_sort_key(point.alt)));
        table->setItem(row, column_input, readonly_item(QString::number(point.x, 'f', 1), QStringLiteral("%1").arg(point.x, 20, 'f', 6, QLatin1Char('0'))));
        table->setItem(row, column_output, readonly_item(QString::number(point.y, 'f', 1), QStringLiteral("%1").arg(point.y, 20, 'f', 6, QLatin1Char('0'))));

        table->setItem(row, column_shortcut1, readonly_item(key1_assigned ? snapview::key_to_string(point.key1) : tr("None")));
        table->setItem(row, column_shortcut2, readonly_item(key2_assigned ? snapview::key_to_string(point.key2) : tr("None")));

        auto add_button_pair = [this, point, row](int key_index, int bind_column, int clear_column)
        {
            auto* bind = new QPushButton(tr("Bind"), table);
            bind->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
            connect(bind, &QPushButton::clicked,
                    this,
                    [this, point, key_index]
                    {
                        snapview_key key;

                        if (prompt_snapview_key_binding(this, key))
                            snapview::instance().update_key(point.axis, point.alt, point.index, key_index, key);
                    });
            table->setCellWidget(row, bind_column, bind);

            auto* clear = new QPushButton(tr("Clear"), table);
            clear->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
            connect(clear, &QPushButton::clicked,
                    this,
                    [point, key_index]
                    {
                        snapview::instance().clear_key(point.axis, point.alt, point.index, key_index);
                    });
            table->setCellWidget(row, clear_column, clear);
        };

        add_button_pair(0, column_bind1, column_clear1);
        add_button_pair(1, column_bind2, column_clear2);
    }

    updating = false;
    update_header_checkbox();
    apply_sort();
    adjust_columns();

    QTimer::singleShot(0, this, &snapview_table::adjust_columns);
}

void snapview_table::rebuild_profile_selector()
{
    if (!profile_combo)
        return;

    const QSignalBlocker blocker(profile_combo);

    profile_combo->clear();

    const QStringList names = snapview::instance().profile_names();
    for (const QString& name : names)
        profile_combo->addItem(name);

    profile_combo->setCurrentIndex(snapview::instance().selected_profile());
}

void snapview_table::profile_changed(int index)
{
    if (updating || index < 0)
        return;

    snapview::instance().set_selected_profile(index);
}

void snapview_table::add_profile()
{
    snapview::instance().add_profile();
}

void snapview_table::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);

    if (event && event->type() == QEvent::LanguageChange)
    {
        rebuild();
        QTimer::singleShot(0, this, &snapview_table::adjust_columns);
    }
}

void snapview_table::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    adjust_columns();
}

void snapview_table::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    adjust_columns();
    QTimer::singleShot(0, this, &snapview_table::adjust_columns);
}

bool snapview_table::is_sortable_column(int column) const
{
    return column == column_point || column == column_curve;
}

void snapview_table::header_clicked(int column)
{
    if (column == column_enabled)
    {
        toggle_all_enabled();
        return;
    }

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

void snapview_table::apply_sort()
{
    if (!table || !is_sortable_column(sort_column))
        return;

    table->horizontalHeader()->setSortIndicator(sort_column, sort_order);
    table->sortItems(sort_column, sort_order);
}

int snapview_table::header_width_for_column(int column) const
{
    if (!table)
        return 1;

    if (column == column_enabled)
    {
        const int indicator = table->style()->pixelMetric(QStyle::PM_IndicatorWidth, nullptr, table);
        return indicator + 8;
    }

    if (const QTableWidgetItem* item = table->horizontalHeaderItem(column))
        return is_compact_content_column(column) ?
            compact_text_width(table, item->text()) :
            text_width(table, item->text());

    return 1;
}

int snapview_table::content_width_for_column(int column) const
{
    if (!table)
        return 1;

    int width = header_width_for_column(column);

    for (int row = 0; row < table->rowCount(); row++)
    {
        if (QWidget* widget = table->cellWidget(row, column))
            width = std::max(width, widget->sizeHint().width() + 8);

        if (const QTableWidgetItem* item = table->item(row, column))
        {
            if (column == column_enabled)
            {
                const int indicator = table->style()->pixelMetric(QStyle::PM_IndicatorWidth, nullptr, table);
                width = std::max(width, indicator + 8);
            }
            else
            {
                const int item_width = is_compact_content_column(column) ?
                    compact_text_width(table, item->text()) :
                    text_width(table, item->text());
                width = std::max(width, item_width);
            }
        }
    }

    return std::max(1, width);
}

void snapview_table::adjust_columns()
{
    if (!table || table->columnCount() != column_count)
        return;

    const int total_width = table->viewport()->width();
    if (total_width <= 0)
        return;

    std::array<int, column_count> widths {};
    std::array<int, column_count> minimums {};

    for (int column = 0; column < column_count; column++)
    {
        widths[std::size_t(column)] = content_width_for_column(column);
        minimums[std::size_t(column)] = 1;
    }

    const int maximum_shortcut_width = std::max(1, int(total_width * 0.20));
    widths[std::size_t(column_shortcut1)] = std::min(widths[std::size_t(column_shortcut1)], maximum_shortcut_width);
    widths[std::size_t(column_shortcut2)] = std::min(widths[std::size_t(column_shortcut2)], maximum_shortcut_width);

    // Hard minima. Button columns are deliberately allowed to shrink below
    // the translated header text; otherwise Russian "Назначить"/"Очистить"
    // makes the table wider than the Options dialog.
    minimums[std::size_t(column_enabled)] = std::max(1, header_width_for_column(column_enabled));
    minimums[std::size_t(column_point)] = std::min(widths[std::size_t(column_point)], 34);
    minimums[std::size_t(column_curve)] = std::min(widths[std::size_t(column_curve)], 30);
    minimums[std::size_t(column_input)] = std::min(widths[std::size_t(column_input)], 30);
    minimums[std::size_t(column_output)] = std::min(widths[std::size_t(column_output)], 30);
    minimums[std::size_t(column_shortcut1)] = std::min(widths[std::size_t(column_shortcut1)], 48);
    minimums[std::size_t(column_shortcut2)] = std::min(widths[std::size_t(column_shortcut2)], 48);
    minimums[std::size_t(column_bind1)] = 28;
    minimums[std::size_t(column_clear1)] = 28;
    minimums[std::size_t(column_bind2)] = 28;
    minimums[std::size_t(column_clear2)] = 28;

    auto sum_widths = [&widths]
    {
        int sum = 0;
        for (int width : widths)
            sum += width;
        return sum;
    };

    auto shrink_columns = [&widths, &minimums](std::initializer_list<int> columns, int& overflow)
    {
        bool changed = true;

        while (overflow > 0 && changed)
        {
            changed = false;

            for (int column : columns)
            {
                if (overflow <= 0)
                    break;

                int& width = widths[std::size_t(column)];
                const int minimum = std::max(1, minimums[std::size_t(column)]);

                if (width <= minimum)
                    continue;

                width--;
                overflow--;
                changed = true;
            }
        }
    };

    int overflow = sum_widths() - total_width;

    if (overflow > 0)
    {
        // First shrink the least information-dense columns.
        shrink_columns({ column_bind1, column_clear1, column_bind2, column_clear2 }, overflow);

        // Then shrink potentially long shortcut names. The 20% upper bound is
        // already applied above; here we only reduce them further if the window
        // is too narrow.
        shrink_columns({ column_shortcut1, column_shortcut2 }, overflow);

        // Last resort: shrink the short descriptive columns. Their contents are
        // elided by QTableWidget if the Options dialog is very narrow.
        shrink_columns({ column_point, column_curve, column_input, column_output }, overflow);
    }

    int free_width = total_width - sum_widths();

    if (free_width > 0)
    {
        // Keep content columns compact and use surplus width on interactive
        // columns. This also ensures that all columns fill the table width.
        const std::array<int, 6> expandable = {
            column_shortcut1, column_bind1, column_clear1,
            column_shortcut2, column_bind2, column_clear2,
        };

        bool changed = true;

        while (free_width > 0 && changed)
        {
            changed = false;

            for (int column : expandable)
            {
                if (free_width <= 0)
                    break;

                if (is_shortcut_column(column) && widths[std::size_t(column)] >= maximum_shortcut_width)
                    continue;

                widths[std::size_t(column)]++;
                free_width--;
                changed = true;
            }
        }

        // If both shortcut columns reached the 20% cap, put the remaining few
        // pixels into button columns so no blank strip remains on the right.
        const std::array<int, 4> buttons = { column_bind1, column_clear1, column_bind2, column_clear2 };
        int button = 0;

        while (free_width > 0)
        {
            widths[std::size_t(buttons[std::size_t(button % int(buttons.size()))])]++;
            button++;
            free_width--;
        }
    }

    for (int column = 0; column < column_count; column++)
        table->setColumnWidth(column, std::max(1, widths[std::size_t(column)]));
}

void snapview_table::update_header_checkbox()
{
    if (!table)
        return;

    int assigned = 0;
    int checked = 0;

    for (int row = 0; row < table->rowCount(); row++)
    {
        const QTableWidgetItem* item = table->item(row, column_enabled);
        if (!item || !item->flags().testFlag(Qt::ItemIsEnabled))
            continue;

        assigned++;
        if (item->checkState() == Qt::Checked)
            checked++;
    }

    if (QTableWidgetItem* header = table->horizontalHeaderItem(column_enabled))
    {
        if (assigned == 0 || checked == 0)
            header->setCheckState(Qt::Unchecked);
        else if (checked == assigned)
            header->setCheckState(Qt::Checked);
        else
            header->setCheckState(Qt::PartiallyChecked);
    }
}

void snapview_table::toggle_all_enabled()
{
    if (!table)
        return;

    bool any_checked = false;
    QVector<QTableWidgetItem*> assigned_items;

    for (int row = 0; row < table->rowCount(); row++)
    {
        QTableWidgetItem* item = table->item(row, column_enabled);
        if (!item || !item->flags().testFlag(Qt::ItemIsEnabled))
            continue;

        assigned_items.push_back(item);
        if (item->checkState() == Qt::Checked)
            any_checked = true;
    }

    const bool new_enabled = !any_checked;

    for (QTableWidgetItem* item : assigned_items)
    {
        const Axis axis = static_cast<Axis>(item->data(role_axis).toInt());
        const bool alt = item->data(role_alt).toBool();
        const int index = item->data(role_index).toInt();
        snapview::instance().set_enabled(axis, alt, index, new_enabled);
    }
}

void snapview_table::item_changed(QTableWidgetItem* item)
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
    update_header_checkbox();
    snapview::instance().set_enabled(axis, alt, index, enabled);
}
