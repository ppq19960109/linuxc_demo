#include <QDebug>
#include "playthread.h"
#include "dec_video.h"

playThread::playThread()
{

}

void playThread::stop()
{
    qDebug() <<  "playThread stop";
    runing=false;
}

void playThread::run()
{

    int mWidth=1,mHeight=1;
    qDebug() <<  "playThread start";
    runing=true;

    dec_open("rtsp://192.168.1.16:8554/live",nullptr,&mWidth,&mHeight);
    uchar* rgbBuffer=new uchar[mWidth*mHeight*3];
    qDebug() <<  "w:"<<mWidth << "h:"<< mHeight;
    while(runing)
    {
        qDebug() <<  "playThread run";
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

