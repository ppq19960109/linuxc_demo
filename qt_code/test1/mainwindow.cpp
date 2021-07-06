#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setGeometry(0, 0, 800, 480);
    //    this->setWindowTitle("文本浏览器");
    //    textBrowser = new QTextBrowser(this);
    //    this->setCentralWidget(textBrowser);

    this->setWindowTitle("图像浏览器");
    graphicsView = new QGraphicsView(this);
//    this->setCentralWidget(graphicsView);
    graphicsView->setGeometry(0,40,640,400);
    graphicsScene = new QGraphicsScene(this);
    graphicsView->setScene(graphicsScene);


    openAction = new QAction("打开",this);
    ui->menubar->addAction(openAction);
    connect(openAction, SIGNAL(triggered()),
            this, SLOT(openActionTriggered()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openActionTriggered()
{
    QString fileName = QFileDialog::getOpenFileName(
                this,tr("打开文件"),"",
                tr("Files(*.txt *.cpp *.h *.html *.htm *.png *.jpg *.bmp)")
                );
    //    QFile myFile(fileName);
    //    if(!myFile.open(QIODevice::ReadOnly | QIODevice::Text))
    //        return;
    //    QTextStream in (&myFile);
    //    QString myText = in.readAll();
    //    if(fileName.endsWith("html") || fileName.endsWith("htm")){
    //        textBrowser->setHtml(myText);
    //    } else {
    //        textBrowser->setPlainText(myText);
    //    }

    QPixmap image(fileName);
    if(image.isNull())
        return;
    image = image.scaled(graphicsView->width(),
                         graphicsView->height(),
                         Qt::KeepAspectRatio,
                         Qt::FastTransformation
                         );
    graphicsScene->clear();
    graphicsScene->addPixmap(image);
    ui->statusbar->showMessage("文件名： " + fileName);
}

