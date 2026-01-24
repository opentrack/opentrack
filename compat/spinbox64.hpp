#pragma once
#include <limits>
#include <QAbstractSpinBox>
#include "export.hpp"

// written by AI, public domain

class OTR_COMPAT_EXPORT spinbox64 : public QAbstractSpinBox
{
    Q_OBJECT

    using limits = std::numeric_limits<qlonglong>;
    qlonglong _min = limits::min(), _max = limits::max(), _value = 0, _step = 1;

    Q_PROPERTY(qlonglong minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(qlonglong maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(qlonglong value   READ value   WRITE setValue NOTIFY valueChanged USER true)
    Q_PROPERTY(qlonglong step    READ stepValue WRITE setStepValue)

public:
    explicit spinbox64(QWidget* parent = nullptr);
    QValidator::State validate(QString& input, int& pos) const override;
    void fixup(QString& input) const override;
    void stepBy(int steps) override;

    qlonglong value() const { return _value; }
    // minimum
    qlonglong minimum() const { return _min; }
    void setMinimum(qlonglong v) { _min = v; }
    // maximum
    qlonglong maximum() const { return _max; }
    void setMaximum(qlonglong v) { _max = v; }
    // step
    qlonglong stepValue() const { return _step; }
    void setStepValue(qlonglong v) { _step = v; }

public slots:
    void clear() override;
    void setValue(qlonglong v);

protected:
    void initStyleOption(QStyleOptionSpinBox* option) const override;
    StepEnabled stepEnabled() const override;

    void keyPressEvent(QKeyEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

signals:
    void valueChanged(qlonglong newValue);
};
