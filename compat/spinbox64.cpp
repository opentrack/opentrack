#include "spinbox64.hpp"
#include <climits>
#include <QLineEdit>
#include <QKeyEvent>
#include <QStyleOptionSpinBox>

// written by AI, public domain

spinbox64::spinbox64(QWidget* parent) : QAbstractSpinBox(parent)
{
    // This single connection handles both "Enter" and "Focus Out"
    connect(lineEdit(), &QLineEdit::editingFinished, this, [this]() {
        bool ok;
        qlonglong val = lineEdit()->text().toLongLong(&ok);
        if (ok)
            setValue(val);
        else
            lineEdit()->setText(QString::number(_value));
    });
}

void spinbox64::setValue(qlonglong val)
{
    qlonglong clipped = qBound(_min, val, _max);

    if (_value != clipped)
    {
        _value = clipped;
        update(); // CRITICAL: Updates arrow button enabled/disabled state
        emit valueChanged(_value);
    }

    // Only update text if it's actually different to prevent cursor jumping
    if (lineEdit()->text().toLongLong() != _value || lineEdit()->text().isEmpty())
    {
        lineEdit()->setText(QString::number(_value));
    }
}

void spinbox64::stepBy(int steps)
{
    qlonglong delta = (qlonglong)steps * _step;

    if (delta > 0)
    {
        if (_value > LLONG_MAX - delta)
            setValue(_max);
        else
            setValue(qBound(_min, _value + delta, _max));
    }
    else if (delta < 0)
    {
        if (_value < LLONG_MIN - delta)
            setValue(_min);
        else
            setValue(qBound(_min, _value + delta, _max));
    }
}

QAbstractSpinBox::StepEnabled spinbox64::stepEnabled() const
{
    StepEnabled flags = StepNone;
    if (_value < _max) flags |= StepUpEnabled;
    if (_value > _min) flags |= StepDownEnabled;
    return flags;
}

QValidator::State spinbox64::validate(QString& input, int& pos) const
{
    (void)pos;
    if (input.isEmpty() || input == "-")
        return QValidator::Intermediate;

    bool ok;
    input.toLongLong(&ok);
    return ok ? QValidator::Acceptable : QValidator::Invalid;
}

void spinbox64::clear()
{
    // Standard behavior for clear() in spinboxes is resetting to 0 (if in range) or min
    setValue(qBound(_min, qlonglong{0}, _max));
}

void spinbox64::focusOutEvent(QFocusEvent* event)
{
    // Base class triggers editingFinished signal, handled by ctor connection
    QAbstractSpinBox::focusOutEvent(event);
}

void spinbox64::keyPressEvent(QKeyEvent* event)
{
    // Let the base class handle standard navigation (Tab, Up/Down arrows, etc.)
    QAbstractSpinBox::keyPressEvent(event);

    // Manually handle the Enter/Return keys to ensure typed text
    // is processed even if the widget doesn't lose focus.
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        bool ok;
        qlonglong val = lineEdit()->text().toLongLong(&ok);
        if (ok)
        {
            setValue(val);
        }
        else
        {
            // Revert the text if it was invalid
            lineEdit()->setText(QString::number(_value));
        }
    }
}

void spinbox64::fixup(QString& input) const
{
    // Called when validate() returns Invalid but the user has finished editing.
    // We try to "repair" the input.
    bool ok;
    input.toLongLong(&ok);
    if (!ok)
    {
        // If it's garbage or empty, revert to the last known good value
        input = QString::number(_value);
    }
}

void spinbox64::initStyleOption(QStyleOptionSpinBox* option) const
{
    if (!option)
        return;

    // First call the base class to set up standard properties (frame, etc.)
    QAbstractSpinBox::initStyleOption(option);

    // Synchronize the style's knowledge of which arrows are enabled
    option->stepEnabled = stepEnabled();

    // You can also customize display text specifically for the style here
    // option->text = lineEdit()->text();
}
