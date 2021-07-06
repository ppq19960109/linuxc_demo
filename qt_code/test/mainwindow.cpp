#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setGeometry(0,0,800,480);

    //    QPixmap pixmap(":images/face1.jpg");
    //    labelImage=new QLabel(this);
    //    labelImage->setGeometry(180, 150, 452, 132);
    //    labelImage->setPixmap(pixmap);
    //    labelImage->setScaledContents(true);

    //    labelString = new QLabel(this);
    //    labelString->setText("标签演示文本");
    //    labelString->setGeometry(300, 300, 100, 20);

    pushButton = new QPushButton(this);
    pushButton->setText("按钮音效测试");
    pushButton->setGeometry(340, 220, 120, 40);

    connect(pushButton, SIGNAL(clicked()),
            this, SLOT(pushButtonClicked()));

    form = new Form(this);
    form->setGeometry(100, 100, 55, 30);
//    form->setMaximumSize(55, 30);
//    form->setMinimumSize(55, 30);

}

void MainWindow::pushButtonClicked()
{
    qDebug() << "pushButtonClicked" <<endl;
    QSound::play(":/audio/c.mp3");
}

MainWindow::~MainWindow()
{
}

