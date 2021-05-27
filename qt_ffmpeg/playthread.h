#ifndef PLAYTHREAD_H
#define PLAYTHREAD_H

#include <QObject>
#include <QThread>
#include <QImage>
class playThread : public QThread
{
    Q_OBJECT

public:
    playThread();
    void stop();
protected:
    virtual void run();
    bool runing;
signals:
    void sig_GetOneFrame(QImage img);
};

#endif // PLAYTHREAD_H
