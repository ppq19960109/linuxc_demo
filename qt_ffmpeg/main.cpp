#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    if(argc<2)
        return -1;
    qDebug() <<  "argv:" << argv[1] << endl;
    playThread::rtsp_ip=argv[1];
    qDebug() <<  "rtsp_ip:" << playThread::rtsp_ip.toLatin1().data() << endl;

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
