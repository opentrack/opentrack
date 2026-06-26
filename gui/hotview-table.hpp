#pragma once

#include <QWidget>
#include <Qt>

class QEvent;
class QResizeEvent;
class QShowEvent;
class QTableWidget;
class QTableWidgetItem;

class hotview_table final : public QWidget
{
    Q_OBJECT

public:
    explicit hotview_table(QWidget* parent = nullptr);

public slots:
    void reload();
    void save();
    void rebuild();

protected:
    void changeEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

private slots:
    void item_changed(QTableWidgetItem* item);
    void header_clicked(int column);

private:
    void set_headers();
    void adjust_columns();
    void apply_sort();
    bool is_sortable_column(int column) const;
    int header_width_for_column(int column) const;
    int content_width_for_column(int column) const;
    void update_header_checkbox();
    void toggle_all_enabled();

    QTableWidget* table = nullptr;
    bool updating = false;
    int sort_column = 1;
    Qt::SortOrder sort_order = Qt::AscendingOrder;
};
