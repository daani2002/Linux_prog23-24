#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include <QMainWindow>


class Server;

class MainWindow : public QMainWindow, private Ui_MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void SetText();
    void packageDisplay(char*, int);

public:
    Ui::MainWindow *ui;
    Server* m_pServer;
};
#endif // MAINWINDOW_H
