#include "ui.h"
#include <QApplication>
#include <QCommandLineParser>
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

	QCommandLineParser p;
	p.setApplicationDescription("opentrack - Head tracking software for MS Windows, Linux, and Apple OSX");
	p.addHelpOption();
	QCommandLineOption autostartOption(QStringList() << "a" << "autostart", "Load <profile> and start tracking", "profile");
	p.addOption(autostartOption);
	p.process(app);

    MainWindow w;

	QString profile = p.value(autostartOption);
	if (! profile.isEmpty() ) 
	{
		w.open_and_run(profile);
	}
		
    w.show();
    app.exec();

    return 0;
}
