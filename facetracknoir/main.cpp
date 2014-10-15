#include "facetracknoir.h"
#include <QApplication>
#include <QStyleFactory>
#include <QStringList>
#include <memory>

int main(int argc, char** argv)
{
    // workaround QTBUG-38598
    QCoreApplication::addLibraryPath(".");
    
    // qt5 designer-made controls look like shit on 'doze -sh 20140921
#ifdef _WIN32
    {
        const QStringList preferred { "fusion", "windowsvista", "jazzbands'-marijuana", "macintosh", "windowsxp" };
        for (const auto& style_name : preferred)
        {
            QStyle* s = QStyleFactory::create(style_name);
            if (s)
            {
                QApplication::setStyle(s);
                break;
            }
        }
    }
#endif
    
    QApplication::setAttribute(Qt::AA_X11InitThreads, true);
    QApplication app(argc, argv);
    
    auto w = std::make_shared<FaceTrackNoIR>();

    w->show();
    app.exec();

    return 0;
}
