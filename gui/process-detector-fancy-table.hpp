#pragma once

#include <QObject>
#include <QWidget>
#include <QTableWidget>

class FancyTable final : public QTableWidget
{
    Q_OBJECT
public:
    void resizeEvent(QResizeEvent* e) override;
public:
    FancyTable(QWidget* parent = nullptr);
};
