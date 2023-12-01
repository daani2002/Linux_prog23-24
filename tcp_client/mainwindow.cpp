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
    readUserName();

    // Csatlakozás állapotának kijelzése
    connect(m_pNetHandler, SIGNAL(signalConnectionStatus(int)),
            this, SLOT(slotConnectionStatus(int)));
    // Szöveg beküldése enterrel - szöveg megjelenítése
    connect(ui->lineEdit, &QLineEdit::returnPressed,
            this, &MainWindow::returnPressed);
    // Szöveg érkezett - megjelenítés
    connect(m_pNetHandler, &NetHandler::packageReceived,
            this, &MainWindow::packageReceived);
    // Felhasználólista bővítése
    connect(m_pNetHandler, &NetHandler::newUserItem,
            this, &MainWindow::newUserItem);
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
        // A köszöntő üzenetben közöljük a szerverrel a nevünket
        m_pNetHandler->sendMessage(NetHandler::ClientGreeting, m_pNetHandler->getUserName());
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
    //m_pNetHandler->packageSend(text);
    m_pNetHandler->sendMessage(NetHandler::PlainText, text);
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

void MainWindow::readUserName()
{
    // Bekerjuk a választott felhasználó nevet
    bool ok;
    QString userName = QInputDialog::getText(this,
      "Username", "Please enter your username",
      QLineEdit::Normal, "username", &ok);
    if(ok && !userName.isEmpty())
    {
        m_pNetHandler->setUserName(userName);
        ui->listWidget->addItem(userName);
    }
}

void MainWindow::newUserItem(QString username)
{
    ui->listWidget->addItem(username);
}




