#include "tracklogger.hpp"
#include "tracker.h"
 

void TrackLoggerCSV::write(const char *s)
{
    out << s;
}


void TrackLoggerCSV::write(const double *p, int n)
{
    if (!first_col)
        out.put(';');
    first_col = false;
    for (int i = 0; i < n-1; ++i)
    {
        out << p[i];
        out.put(';');
    }
    out << p[n-1];
}

void TrackLoggerCSV::next_line()
{
    out << std::endl;
    first_col = true;
}