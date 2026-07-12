#pragma once

#include <QWidget>
#include <Qt>

class QEvent;
class QResizeEvent;
class QShowEvent;
class QComboBox;
class QPushButton;
class QTableWidget;
class QTableWidgetItem;

class snapview_table final : public QWidget
{
    Q_OBJECT

public:
    explicit snapview_table(QWidget* parent = nullptr);

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
    void profile_changed(int index);
    void add_profile();

private:
    void set_headers();
    void adjust_columns();
    void apply_sort();
    bool is_sortable_column(int column) const;
    int header_width_for_column(int column) const;
    int content_width_for_column(int column) const;
    void update_header_checkbox();
    void toggle_all_enabled();
    void rebuild_profile_selector();

    QComboBox* profile_combo = nullptr;
    QPushButton* add_profile_button = nullptr;
    QTableWidget* table = nullptr;
    bool updating = false;
    int sort_column = 1;
    Qt::SortOrder sort_order = Qt::AscendingOrder;
};
