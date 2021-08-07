#include "mainwindow.h"
#include <QApplication>


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QCoreApplication::setOrganizationName("INR");
	QCoreApplication::setApplicationName("ControlServer");
    QCoreApplication::setApplicationVersion("1.4 beta");
	QApplication::setStyle(QStyleFactory::create("windowsvista"));
//	QFontDatabase::addApplicationFont(":/consola.ttf");
//	QFontDatabase::addApplicationFont(":/calibri.ttf");
    MainWindow w;
	w.show();

	return a.exec();
}
