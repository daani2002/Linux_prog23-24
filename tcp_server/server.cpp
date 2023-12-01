#include "server.h"
#include "ui_mainwindow.h"

Server::Server(QObject *parent) :
        QObject(parent)
{
    m_pServSocket = NULL;

    for(int i = 0; i < MaxClientNum; i++)
        m_pSocket[i] = NULL;
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
        m_pSocket[i]->write("sikeres kapcsolat");
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
    for(int i = 0; i < MaxClientNum; i++){
    if(m_pSocket[i] != NULL && m_pSocket[i]->bytesAvailable()){
        // Temporalis bufferba olvasunk.
        int len = m_pSocket[i]->read(buf, sizeof(buf));
        // Loopback teszt
        m_pSocket[i]->write(buf, len);
        // Jelezzük az olvasást
        emit packageReceived(buf, len);
    }
    }
}
































