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

    static mem<TrackLogger> make() { return std::make_shared<TrackLogger>(); }

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
public:
    TrackLoggerCSV(const QString &filename)  :  TrackLogger(),
        first_col(true)
    {
        out.open(filename.toStdString());
        if (!out.is_open())
            throw std::ios_base::failure("unable to open file");
    }

    static mem<TrackLogger> make(const main_settings &s) { return std::static_pointer_cast<TrackLogger>(std::make_shared<TrackLoggerCSV>(s.tracklogging_filename)); }

    virtual void write(const char *s);
    virtual void write(const double *p, int n);
    virtual void next_line();
};

