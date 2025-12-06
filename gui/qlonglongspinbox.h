#pragma once

#include <QtWidgets/QAbstractSpinBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidget>

class QLongLongSpinBoxPrivate;
class Q_WIDGETS_EXPORT QLongLongSpinBox : public QAbstractSpinBox
{
    Q_OBJECT

    Q_PROPERTY(qlonglong minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(qlonglong maximum READ maximum WRITE setMaximum)

    Q_PROPERTY(qlonglong value READ value WRITE setValue NOTIFY valueChanged USER true)

    qlonglong m_minimum;
    qlonglong m_maximum;
    qlonglong m_value;

public:
    explicit QLongLongSpinBox(QWidget *parent = nullptr)
        : QAbstractSpinBox(parent),
          m_minimum(std::numeric_limits<qlonglong>::min()),
          m_maximum(std::numeric_limits<qlonglong>::max()),
          m_value(m_minimum)
    {
        setRange(m_minimum, m_maximum);
        connect(lineEdit(), &QLineEdit::textEdited,
                this,        &QLongLongSpinBox::onEditFinished);
    }

    ~QLongLongSpinBox() {};

    qlonglong value() const { return m_value; };

    qlonglong minimum() const { return m_minimum; };

    void setMinimum(qlonglong min) { m_minimum = min; }

    qlonglong maximum() const { return m_maximum; };

    void setMaximum(qlonglong max) { m_maximum = max; }

    void setRange(qlonglong min, qlonglong max)
    {
        setMinimum(min);
        setMaximum(max);
    }

    virtual void stepBy(int steps)
    {
        auto new_value = m_value;
        if (steps < 0 && new_value + steps > new_value)
        {
            new_value = std::numeric_limits<qlonglong>::min();
        }
        else if (steps > 0 && new_value + steps < new_value)
        {
            new_value = std::numeric_limits<qlonglong>::max();
        }
        else
        {
            new_value += steps;
        }

        lineEdit()->setText(textFromValue(new_value));
        setValue(new_value);
    }

protected:
    // bool event(QEvent *event);
    virtual QValidator::State validate(QString& input, int& pos) const
    {
        bool ok;
        qlonglong val = input.toLongLong(&ok);
        if (!ok)
            return QValidator::Invalid;

        if (val < m_minimum || val > m_maximum)
            return QValidator::Invalid;

        return QValidator::Acceptable;
    }

    virtual qlonglong valueFromText(const QString& text) const { return text.toLongLong(); }

    virtual QString textFromValue(qlonglong val) const { return QString::number(val); }
    // virtual void fixup(QString &str) const;

    virtual QAbstractSpinBox::StepEnabled stepEnabled() const { return StepUpEnabled | StepDownEnabled; }

public Q_SLOTS:
    void setValue(qlonglong val)
    {
        if (m_value != val)
        {
            lineEdit()->setText(textFromValue(val));
            m_value = val;
        }
    }

    void onEditFinished()
    {
        QString input = lineEdit()->text();
        int pos = 0;
        if (QValidator::Acceptable == validate(input, pos))
            setValue(valueFromText(input));
        else
            lineEdit()->setText(textFromValue(m_value));
    }

Q_SIGNALS:
    void valueChanged(qlonglong v);

private:
    Q_DISABLE_COPY(QLongLongSpinBox)

    Q_DECLARE_PRIVATE(QLongLongSpinBox)
};