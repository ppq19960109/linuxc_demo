#ifndef PLAYTHREAD_H
#define PLAYTHREAD_H

#include <QObject>
#include <QThread>
#include <QImage>
#include <QString>
class playThread : public QThread
{
    Q_OBJECT

public:
    static QString rtsp_ip;
    playThread();
    void stop();
protected:
    virtual void run();
    bool runing;
signals:
    void sig_GetOneFrame(QImage img);
};

#endif // PLAYTHREAD_H
