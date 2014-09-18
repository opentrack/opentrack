#include "facetracknoir.h"
#include <QApplication>
#include <memory>

int main(int argc, char** argv)
{
    QApplication::setAttribute(Qt::AA_X11InitThreads, true);
    QApplication app(argc, argv);
    auto w = std::make_shared<FaceTrackNoIR>();

    w->show();
    app.exec();

    return 0;
}
