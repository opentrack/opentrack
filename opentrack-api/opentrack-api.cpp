#include "opentrack-guts.h"
#include "opentrack.h"

opentrack opentrack_make_ctx(const char *dir)
{
    QDir d(dir);
    return new opentrack_ctx(d);
}

void opentrack_finalize_ctx(opentrack bye_bye)
{
    delete bye_bye;
}
