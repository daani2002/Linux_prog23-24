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
    // Személyes üezenet érkezett, ellenőrizni kell a küldőt
    connect(m_pNetHandler, &NetHandler::controlMessage,
            this, &MainWindow::slotControlMessage);
    // Felhasználólista bővítése
    connect(m_pNetHandler, &NetHandler::newUserItem,
            this, &MainWindow::newUserItem);
    // Cél felhasználó kiválasztása a listából kattintásra
    connect(ui->listWidget, &QListWidget::itemClicked,
            this, &MainWindow::setDestination);
    // Check box alapján csatornaüzenet beállítása
    connect(ui->checkBox, &QCheckBox::clicked,
           this, &MainWindow::setDestination);
    // Felhasználó tiltása a listából duplakattintással
    connect(ui->listWidget, & QListWidget::itemDoubleClicked,
            this, &MainWindow::blockUserItem);
    // Tiltott felhasználó feloldása
    connect(ui->listWidget_2, &QListWidget::itemDoubleClicked,
            this, &MainWindow::unblockUserItem);
    // Kilépő felhasználó törlése a listából
    connect(m_pNetHandler, &NetHandler::removeUserItem,
            this, &MainWindow::removeUserItem);

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
        // Töröljük a felhasználólistát, kivéve saját név
        ui->listWidget->clear();
        ui->listWidget->addItem(m_pNetHandler->getUserName() + " (te)");
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

    // Szöveg elküldése
    // Mindenkinek küldünk
    if(ui->checkBox->isChecked()){
        m_pNetHandler->sendMessage(NetHandler::ChannelMessage, text);
        // Szöveg megjelenítése
        ui->textEdit->append("<from you to everyone> " + text);
    }
    // Egyetlen személynek
    else if(m_pNetHandler->getDestinationName() != "notregistered"){
        m_pNetHandler->sendMessage(NetHandler::PlainText, text);
        // Szöveg megjelenítése
        ui->textEdit->append("<from you to " + m_pNetHandler->getDestinationName() + "> " + text);
    }
    // Hiba: nincs célszemély megjelölve
    else
        ui->textEdit->append("< Válassz címzettet! >");
}

void MainWindow::packageReceived(QString str)
{
    ui->textEdit->append(str);
}

// Ellenőrizzük, az üzenet feladója nincs-e tiltólistán (azaz a felhasználó listán van)
void MainWindow::slotControlMessage(QString Sender, QString msg)
{
    for(int i = 0; i < ui->listWidget->count(); i++)
        if(Sender == ui->listWidget->item(i)->text())
        {
            ui->textEdit->append("<from " + Sender + " to you> " + msg);
            break;
        }
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
    // ClientLeft üzenet küldése a szervernek (csak üzenetkód)
    // <3>
    m_pNetHandler->sendMessage(NetHandler::ClientLeft, "");
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
        ui->listWidget->addItem(userName + " (te)");
    }
}

// Új username felvétele a listába
void MainWindow::newUserItem(QString username)
{
    ui->listWidget->addItem(username);
}

// Kilépő user törlése a listWidget-ből
void MainWindow::removeUserItem(QString username)
{
    // findItems egy listát ad vissza
    QList<QListWidgetItem *> list = ui->listWidget->findItems(username, Qt::MatchContains);
    QListWidgetItem* item = list.first();

    int row = ui->listWidget->row(item);
    ui->listWidget->takeItem(row);
}

// A felhasználó lista vagy a check box kattintásával
// kiválasztható az üzenet célszemélye vagy csatornaüzenet
void MainWindow::setDestination()
{
    if(ui->checkBox->isChecked())
        ui->label->setText("Üzenet mindenkinek:");
    else
    {
        QListWidgetItem *item = ui->listWidget->currentItem();
        QString name = item->text();
        // Ha magunkat címezzük
        if(ui->listWidget->currentRow() == 0)
        {
            ui->label->setText("Üzenet magamnak:");
            // levágjuk a " (te)" végződéset
            name.chop(5);
            m_pNetHandler->setDestinationName(name);
        }
        else
        {
            ui->label->setText("Üzenet " + item->text() + "-nek:");
            m_pNetHandler->setDestinationName(item->text());
        }
    }
}

// Felhasználó tiltása
// Kivesszük a felhasználó listából
// Áttesszük a tiltólistába
void MainWindow::blockUserItem(QListWidgetItem* item)
{
    int row = ui->listWidget->currentRow();
    QString name = item->text();

    if(row != 0){
        // Hozzáadjuk a nevet a tiltólistához
        ui->listWidget_2->addItem(name);
        // Eltávolítjuk az erdetiből
        ui->listWidget->takeItem(row);
        // Az imént tiltott személy volt kattintással kijelölve,
        // ezért frissítjük a címzettet
        setDestination();
    }
}

// Tiltás feloldása
// Kivesszük a tiltólistából
// Áttesszük a felhasználó listába
void MainWindow::unblockUserItem(QListWidgetItem* item)
{
    int row = ui->listWidget_2->currentRow();
    QString name = item->text();

    // Eltávolítjuk a tiltólistából
    ui->listWidget_2->takeItem(row);
    // Hozzáadjuk a nevet a felhasználókhoz
    ui->listWidget->addItem(name);

}





