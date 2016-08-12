#pragma once

#include <QMutex>

class MyMutex {
private:
    QMutex inner;

public:
    QMutex* operator->() { return &inner; }
    QMutex* operator->() const { return &const_cast<MyMutex*>(this)->inner; }

    MyMutex operator=(const MyMutex& datum)
    {
        auto mode =
                datum->isRecursive()
                ? QMutex::Recursive
                : QMutex::NonRecursive;

        return MyMutex(mode);
    }

    MyMutex(const MyMutex& datum)
    {
        *this = datum;
    }

    MyMutex(QMutex::RecursionMode mode = QMutex::NonRecursive) :
        inner(mode)
    {
    }

    QMutex* operator&() const
    {
        return const_cast<QMutex*>(&inner);
    }
};
