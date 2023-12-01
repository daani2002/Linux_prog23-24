#include "mainwindow.h"
#include "nethandler.h"
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_pNetHandler = new NetHandler(this);

    // felhasználónév bekérése
    getUserName();

    // Csatlakozás állapotának kijelzése
    connect(m_pNetHandler, SIGNAL(signalConnectionStatus(int)),
            this, SLOT(slotConnectionStatus(int)));
    // Szöveg beküldése enterrel - szöveg megjelenítése
    connect(ui->lineEdit, &QLineEdit::returnPressed,
            this, &MainWindow::returnPressed);
    // Szöveg érkezett - megjelenítés
    connect(m_pNetHandler, &NetHandler::packageReceived,
            this, &MainWindow::packageReceived);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// A kapcsolat allapotanak kijelzese es a menuelemek allitasa.
// Attol fuggoen, hogy mi a halozati kapcsolat allapota kulonbozo szoveget irunk ki,
// illetve kulonbozo menupontokat engedelyezunk.
void MainWindow::slotConnectionStatus(int status)
{
    // A halozati kapcsolat allapotanak kiirasa a status bar-ra.
    switch(status)
    {
    case NetHandler::Error:
        statusBar()->showMessage("Error");
        break;
    case NetHandler::Connecting:
        statusBar()->showMessage("Connecting...");
        break;
    case NetHandler::Connected:
        statusBar()->showMessage("Connected");
        break;
    case NetHandler::Disconnected:
        statusBar()->showMessage("Disconnected");
        break;
    }
    if((status == NetHandler::Error) || (status == NetHandler::Disconnected))
    {
        ui->actionConnect->setEnabled(true);

    }
    else if((status == NetHandler::Connecting) || (status == NetHandler::Connected))
    {
        ui->actionConnect->setEnabled(false);

    }
}

void MainWindow::returnPressed()
{
    // Kiolvassuk a beírt sort
    QString text = ui->lineEdit->text();
    // Töröljük a beírt szöveget
    ui->lineEdit->clear();
    // Szöveg megjelenítése
    ui->textEdit->append(text);
    // Szöveg elküldése
    m_pNetHandler->packageSend(text);
}

void MainWindow::packageReceived(QString str)
{
    ui->textEdit->append(str);
}

void MainWindow::on_actionConnect_triggered()
{
    // Bekerjuk a masik gep cimet. Ha ez nem nulla hosszusagu, akkor kapcsolodunk hozza.
    bool ok;
    QString addr = QInputDialog::getText(this,
      "Server address", "Please enter the server host:",
      QLineEdit::Normal, "localhost", &ok);
    if(ok && !addr.isEmpty())
        m_pNetHandler->Connect(addr);
}

void MainWindow::on_actionStop_triggered()
{
    m_pNetHandler->slotDisconnected();
}

void MainWindow::getUserName()
{
    // Bekerjuk a választott felhasználó nevet
    bool ok;
    QString userName = QInputDialog::getText(this,
      "Username", "Please enter your username",
      QLineEdit::Normal, "userame", &ok);
    if(ok && !userName.isEmpty())
        ui->listWidget->addItem(userName);
}





