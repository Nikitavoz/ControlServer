#include "mainwindow.h"
#include <QApplication>


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QCoreApplication::setOrganizationName("INR");
	QCoreApplication::setApplicationName("ControlServer");
    QCoreApplication::setApplicationVersion("1.g");
    MainWindow w;
	w.show();

	return a.exec();
}
