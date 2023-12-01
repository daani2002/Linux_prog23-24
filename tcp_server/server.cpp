#include "server.h"
#include "ui_mainwindow.h"

Server::Server(QObject *parent) :
        QObject(parent)
{
    m_pServSocket = NULL;

    for(int i = 0; i < MaxClientNum; i++)
    {
        m_pSocket[i] = NULL;
        userName[i] = "not registered";
    }
}

Server::~Server()
{
    delete m_pServSocket;
}

void Server::StartServer()
{
    if (m_pServSocket) return;
    // Letrehozzuk a szerver socket-et.

    m_pServSocket = new QTcpServer();
    if (!m_pServSocket->listen(
                QHostAddress::Any, 3490)) {
        delete m_pServSocket;
        m_pServSocket = NULL;
        return;
    }

    // Aszinkron modon, szignal-al figyeljuk a kapcsolodasokat.
    connect(m_pServSocket, SIGNAL(newConnection()), this, SLOT(slotIncomingConn()));

    // Jelezzük a szerver elindulását
    emit serverStarted();
}

// A bejovo kapcsolatok fogadasa.
void Server::slotIncomingConn()
{
    QTcpSocket* pSocket = m_pServSocket->nextPendingConnection();

    connect(pSocket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));

    // 20 kapcsolatot fogadunk elsőkörben, mindig az első szabad socket pointert használjuk
    for(int i = 0; i < MaxClientNum; i++)
    if (m_pSocket[i] == NULL) {
        m_pSocket[i] = pSocket;
        connect(m_pSocket[i], SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
        //m_pSocket[i]->write("sikeres kapcsolat");
        break;
    }

}

// A kapcsolat lezarasanak erzekelese.
void Server::slotDisconnected()
{
    disconnect(this, SLOT(slotDisconnected()));

    // Ha letezik meg kliens socket akkor lezarjuk.
    for(int i = 0; i < MaxClientNum; i++)
    if (m_pSocket[i] && !m_pSocket[i]->isOpen()) {
        m_pSocket[i]->deleteLater();
        m_pSocket[i] = NULL;
    }
}

// Csomag erkezesenek lekezelese.
void Server::slotReadyRead()
{
    // Megkeressük, melyik porton érkezett adat
    for(int i = 0; i < MaxClientNum; i++){
    if(m_pSocket[i] != NULL && m_pSocket[i]->bytesAvailable()){
        // Temporalis bufferba olvasunk.
        int len = m_pSocket[i]->read(buf, sizeof(buf));
        // Loopback teszt
            //m_pSocket[i]->write(buf, len);
        // Char* -> QString konverzió, így könnyebb a sting manipuláció
        QByteArray ba(buf, len);
        QString str(ba);
            // Jelezzük az olvasást a megjelenítő ablaknak
            emit textReceived(str);

        int j = 0;
        j = str.indexOf(">", 0);
        // Kiolvassuk a kapott üzenet típusát
        QString msgType = str;
        msgType.resize(j+1);
        str.remove(0, j+1);

        // ClientGreeting: közölték a sockethez tartozófelhasználónevet
        if(msgType == "<0>")
        {
            // Elmentjük a socket indexén
            userName[i] = str;
            // Visszaküldjük a többi felhasználónevet
            QString otherUsers;
            for(int x = 0; x < MaxClientNum; x++)
                if(m_pSocket[x] != NULL)
                    otherUsers.append(userName[x]);

            // ServerGreeting típusú üzenet
            otherUsers.prepend("<1>");
            QByteArray ba = otherUsers.toLocal8Bit();
            const char *c_otherUsers = ba.data();
            m_pSocket[i]->write(c_otherUsers);
        }
        // PlainText: egyszerű üzenetet küldtek adott személyeknek
        // teszt: broadcast először
        if(msgType == "<4>")
        {
            j = str.indexOf(">", 0);
            QString destination = str;
            destination.resize(j+1);
            str.remove(0, j+1);


            // QString -> char*
            QByteArray ba = str.toLocal8Bit();
            const char *c_str = ba.data();

            // Megkeressük a destinationnak megfelelő user-t/socket-et
            for(int x = 0; x < MaxClientNum; x++)
                if(QString::compare(userName[x], destination, Qt::CaseInsensitive) == 0)//userName[x] == destination)
                    m_pSocket[x]->write(c_str);

        }

    }
    }
}
































