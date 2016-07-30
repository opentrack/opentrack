#pragma once
#include "main-settings.hpp"
#include "opentrack-compat/options.hpp"

#include <fstream>
#include <QString>
#include <QMessageBox>
#include <QWidget>

class OPENTRACK_LOGIC_EXPORT TrackLogger
{
public:
    TrackLogger()
    {
    }

    virtual ~TrackLogger();

    virtual void write(const char *)
    {
    }

    virtual void write(const double *, int n)
    {
    }

    virtual void next_line()
    {
    }

    void write_pose(const double *p)
    {
        write(p, 6);
    }
};


class OPENTRACK_LOGIC_EXPORT TrackLoggerCSV : public TrackLogger
{
    std::ofstream out;
    bool first_col;
    inline void handle_first_col_sep();
public:
    TrackLoggerCSV(const QString &filename) : first_col(true)
    {
        out.open(filename.toStdString());
    }

    bool is_open() const { return out.is_open(); }
    void write(const char *s) override;
    void write(const double *p, int n) override;
    void next_line() override;
};

