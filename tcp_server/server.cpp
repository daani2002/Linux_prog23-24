#include "server.h"
#include "ui_mainwindow.h"

Server::Server(QObject *parent) :
        QObject(parent)
{
    m_pServSocket = NULL;

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

    if (m_pSocket1 != NULL) {
        delete pSocket;
        return;
    }

    connect(pSocket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));

    if (m_pSocket1 == NULL) {
        m_pSocket1 = pSocket;
        connect(m_pSocket1, SIGNAL(readyRead()), this, SLOT(slotReadyRead1()));
    }
    m_pSocket1->write("sikeres kapcsolat");

}

// A kapcsolat lezarasanak erzekelese.
void Server::slotDisconnected()
{
    disconnect(this, SLOT(slotDisconnected()));

    // Ha letezik meg kliens socket akkor lezarjuk.
    if (m_pSocket1) {
        m_pSocket1->deleteLater();
        m_pSocket1 = NULL;
    }
}

// Csomag erkezesenek lekezelese.
void Server::slotReadyRead1()
{
    // Temporalis bufferba olvasunk.
    int len = m_pSocket1->read(buf, sizeof(buf));
    // Loopback teszt
    m_pSocket1->write(buf, len);
    // Jelezzük az olvasást
    emit packageReceived(buf, len);
}
































