#include "facetracknoir.h"
#include <QApplication>
#include <memory>

#ifdef _WIN32
#   include <objbase.h>
#endif

int main(int argc, char** argv)
{
#ifdef _WIN32
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif
    
    QApplication::setAttribute(Qt::AA_X11InitThreads, true);
    QApplication app(argc, argv);
    auto w = std::make_shared<FaceTrackNoIR>();

    w->show();
    app.exec();

    return 0;
}
