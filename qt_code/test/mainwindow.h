#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QLabel>
#include <QPushButton>
#include <QString>

#include <QSound>
#include "form.h"
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QLabel *labelString;
    QLabel *labelImage;
    QPushButton *pushButton;

    Form* form;
private slots:
    void pushButtonClicked();

};
#endif // MAINWINDOW_H
