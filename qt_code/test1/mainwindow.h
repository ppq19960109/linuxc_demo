#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextBrowser>
#include <QGraphicsView>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QTextBrowser *textBrowser;
    QAction *openAction;

    QGraphicsView *graphicsView;
    QGraphicsScene *graphicsScene;
public slots:
 void openActionTriggered();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
