#include <QDebug>
#include "playthread.h"
#include "dec_video.h"
#include <sys/stat.h>
playThread::playThread()
{

}

void playThread::stop()
{
    qDebug() <<  "playThread stop";
    runing=false;
}
QString playThread::rtsp_ip="123";

void playThread::run()
{

    int mWidth=1,mHeight=1;
    qDebug() <<  "playThread start";
    runing=true;

//    rtsp_ip="rtsp://192.168.0.108:8554/h264Live";
    QByteArray rtsp_ip_byte=rtsp_ip.toUtf8();
    char* rip=rtsp_ip_byte.data();
    qDebug() <<  "ffmpeg rtsp_ip:" << rip << endl;

    sleep(2);
    dec_open(rip,NULL,&mWidth,&mHeight);
    uchar* rgbBuffer=new uchar[mWidth*mHeight*3];
    qDebug() <<  "w:"<<mWidth << "h:"<< mHeight;


    while(runing)
    {
//        qDebug() <<  "playThread run";
        //        sleep(1);

        if(dec_run(rgbBuffer)<0)
            break;
        QImage tmpImg((uchar *)rgbBuffer,mWidth,mHeight,QImage::Format_RGB888);
        QImage image = tmpImg.copy(); //把图像复制一份 传递给界面显示
        emit sig_GetOneFrame(image);  //发送信号，将次imamge发送到要显示的控件，用paintEvent绘制出来

    }
    qDebug() <<  "playThread exit\n";
    delete[] rgbBuffer;
    dec_close();
}

