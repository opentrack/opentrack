#include "tracklogger.hpp"
#include "pipeline.hpp"

TrackLogger::~TrackLogger() {}

void TrackLogger::reset_dt()
{
    t.start();
}

void TrackLogger::write_dt()
{
    const double dt = t.elapsed_seconds();
    write(&dt, 1);
}

void TrackLoggerCSV::handle_first_col_sep()
{
    if (!first_col)
        out.put(',');
    first_col = false;
}

void TrackLoggerCSV::write(const char *s)
{
    handle_first_col_sep();
    out << s;
}


void TrackLoggerCSV::write(const double *p, int n)
{
    handle_first_col_sep();
    for (int i = 0; i < n-1; ++i)
    {
        out << p[i];
        out.put(',');
    }
    out << p[n-1];
}

void TrackLoggerCSV::next_line()
{
    out << std::endl;
    first_col = true;
}

