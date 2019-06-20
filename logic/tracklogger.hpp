#pragma once
#include "main-settings.hpp"
#include "options/options.hpp"
#include "compat/timer.hpp"

#include <fstream>
#include <QString>
#include <QDebug>

class OTR_LOGIC_EXPORT TrackLogger
{
    Timer t;

public:
    TrackLogger() = default;
    virtual ~TrackLogger();

    virtual void write(const char *) {}
    virtual void write(const double *, int) {}
    virtual void next_line() {}

    void write_pose(const double *p);

    void reset_dt();
    void write_dt();
};


class OTR_LOGIC_EXPORT TrackLoggerCSV : public TrackLogger
{
    std::ofstream out;
    bool first_col = true;
    inline void handle_first_col_sep();
public:
    explicit TrackLoggerCSV(const QString &filename)
    {
        out.open(filename.toStdString());
    }

    bool is_open() const { return out.is_open(); }
    void write(const char *s) override;
    void write(const double *p, int n) override;
    void next_line() override;

    TrackLoggerCSV(const TrackLoggerCSV&) = delete;
    TrackLoggerCSV& operator=(const TrackLoggerCSV&) = delete;
};

