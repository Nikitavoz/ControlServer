#include "mainwindow.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	QCoreApplication::setOrganizationName("INR");
	QCoreApplication::setApplicationName("ControlServer");
    QCoreApplication::setApplicationVersion("1.j.4");

    qInstallMessageHandler([](QtMsgType, const QMessageLogContext &, const QString &msg) {
        QFile el(QCoreApplication::applicationName() + ".errorlog");
        el.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
        el.write((msg.endsWith("\n") ? msg : msg + "\n").toUtf8());
        el.close();
    });

    MainWindow w;
	w.show();
	return a.exec();
}
