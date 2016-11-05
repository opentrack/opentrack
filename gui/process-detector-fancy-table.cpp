#include "process-detector-fancy-table.hpp"

void FancyTable::resizeEvent(QResizeEvent* e)
{
    QTableView::resizeEvent(e);
    int w = width();
    setColumnWidth(2, 32);
    w -= 40;
    setColumnWidth(0, w / 2);
    setColumnWidth(1, w / 2);
}

FancyTable::FancyTable(QWidget* parent) : QTableWidget(parent) {}
