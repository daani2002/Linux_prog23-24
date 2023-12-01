#include "mainwindow.h"
#include "server.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //Server *server = new Server();
    m_pServer = new Server(this);

    m_pServer->StartServer();
    //debug
    //connect(ui->pushButton, SIGNAL(clicked),
      //      m_pServer, SLOT(slotReadyRead));
    // Jelezzük a szerver elindulását
    connect(m_pServer, SIGNAL(serverStarted),
            this, SLOT(SetText));
    // Jelezzük az üzenet érkezését
    connect(m_pServer, &Server::textReceived,
            this, &MainWindow::textDisplay);

    ui->label->setText("Szerver teszt");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetText(){ui->textEdit->append("csomag érkezett");}

void MainWindow::textDisplay(QString str)
{
    ui->textEdit->append(str);//append("uezenet erkezett");
}
