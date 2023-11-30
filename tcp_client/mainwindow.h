#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class NetHandler;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:

public slots:
    void slotConnectionStatus(int);
    void returnPressed();
    void packageReceived(QString str);

private:
    Ui::MainWindow *ui;
    NetHandler* m_pNetHandler;

private slots:
    void on_actionConnect_triggered();

};
#endif // MAINWINDOW_H








