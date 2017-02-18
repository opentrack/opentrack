#include "process-detector-fancy-table.hpp"

#include <QHeaderView>

void FancyTable::resizeEvent(QResizeEvent* e)
{
    QTableView::resizeEvent(e);
    int w = width();
    setColumnWidth(2, 32);
    w -= 40;
    setColumnWidth(0, w / 2);
    setColumnWidth(1, w / 2);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
}

FancyTable::FancyTable(QWidget* parent) : QTableWidget(parent) {}
