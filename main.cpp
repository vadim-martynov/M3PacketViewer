#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator viewerT;
    QString qmPath = "m3packetviewer_";
    viewerT.load(qmPath + QLocale::system().name());
    a.installTranslator(&viewerT);

    MainWindow w(0, a.arguments());
    w.show();

    return a.exec();
}
