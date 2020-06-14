#pragma once

#include <QString>
#include <QException>

class ProtonException : public QException
{
public:
    ProtonException(const QString& message)
        : message(message) {}

    virtual ~ProtonException()
        {}

    void raise() const { throw *this; }
    ProtonException *clone() const { return new ProtonException(*this); }

    QString getMessage() const {
        return message;
    }
private:
    QString message;
};
