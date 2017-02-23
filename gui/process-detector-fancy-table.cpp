#include "process-detector-fancy-table.hpp"

#include <QHeaderView>

void FancyTable::resizeEvent(QResizeEvent* e)
{
    QTableView::resizeEvent(e);
    int w = width();
    setContentsMargins(QMargins(0, 0, 0, 0));
    w -= 64;
    setColumnWidth(2, 32);
    setColumnWidth(0, w / 2);
    setColumnWidth(1, w / 2);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
}

FancyTable::FancyTable(QWidget* parent) : QTableWidget(parent) {}
