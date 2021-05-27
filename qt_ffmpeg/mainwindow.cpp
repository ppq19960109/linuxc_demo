#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(&play_thread, &playThread::sig_GetOneFrame,
            this, &MainWindow::displayRGB);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::displayRGB(QImage img)
{
    qDebug() <<  "displayRGB";
    ui->image_label->setPixmap(QPixmap::fromImage(img));
}

void MainWindow::on_playButton_clicked()
{
//    QImage* rgb_image = new QImage();

//    ui->image_label->setPixmap(QPixmap::fromImage(*rgb_image));
//    delete rgb_image;
}

void MainWindow::on_playButton_clicked(bool checked)
{
    qDebug() <<  "on_playButton_clicked bool:" << checked;
    if(checked)
    {
       ui->playButton->setText("PAUSE");
       play_thread.start();
    }
    else
    {
        ui->playButton->setText("PLAY");
        play_thread.stop();
    }
}
