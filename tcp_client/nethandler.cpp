#include "nethandler.h"

NetHandler::NetHandler(QObject *parent) :
    QObject(parent)
{
    m_pSocket = NULL;
}

NetHandler::~NetHandler()
{
    disconnect(this, SLOT(slotDisconnected()));

    if(m_pSocket) delete m_pSocket;
}

// Kapcsolodas a szerverhez.
void NetHandler::Connect(QString addr)
{
    // Biztonsagi ellenorzes. Ha letezik mar a socket, akkor
    // nem szabadna hogy ez a fuggveny meghivodjon.
    if (m_pSocket) return;

    // Letrehozzuk a kliens socket-et.
    m_pSocket = new QTcpSocket();

    // Jelezzuk a felhasznalo fele, hogy eppen kapcsolodni probalunk.
    emit signalConnectionStatus(Connecting);

    // Bekotjuk a kapcsolat bontasanak erzekeleset. Amikor a masik oldal lezarja
    // a kapcsolatot, akkor a kliens socket egy disconnected() szignalt general.
    connect(m_pSocket, SIGNAL(disconnected()),
            this, SLOT(slotDisconnected()));

    // Bekotjuk a bejovo csomagok erzekeleset. A kovetkezo sorban levo kapcsolo
    // bekapcsolasaval elerhetjuk, hogy amikor csomag erkezik a kliens socketen
    // keresztul, akkor azt egy readyRead() szignallal jelezze. Igy nem kell
    // hogy folyamatosan varakozzunk az erkezo csomagokra.
    connect(m_pSocket, SIGNAL(readyRead()),
            this, SLOT(slotReadyRead()));


    // A connect() fuggveny meghivasaval kapcsolodunk a szerverhez.
    m_pSocket->connectToHost(addr, 3490);
    if (!m_pSocket->waitForConnected(5000)) {
        delete m_pSocket;
        m_pSocket = NULL;

        emit signalConnectionStatus(Error);
        return;
    }

    // Jelezzuk a kapcsolat allapotanak valtozasat, vagyis hogy sikerult.
    emit signalConnectionStatus(Connected);
}

// A kapcsolat lezarasanak erzekelese.
// Ha lezartak a kapcsolatot, akkor a szignal hatasara ez a fuggveny hivodik
// meg.
void NetHandler::slotDisconnected()
{
    // Kikotjuk a socket lezarasanak erzekeleset, mert kulonben a sajatunkat is
    // erzekelnenk.
    disconnect(this, SLOT(slotDisconnected()));

    // Ha letezik meg kliens socket akkor lezarjuk.
    if (m_pSocket) {
        m_pSocket->deleteLater();
        m_pSocket = NULL;
    }

    // Jelezzuk a kapcsolat allapotanak valtozasat.
    emit signalConnectionStatus(Disconnected);

}

// Csomag erkezesenek lekezelese.
// Amikor egy csomag erkezik, akkor a szignal hatasara ez a slot hivodik meg.
void NetHandler::slotReadyRead()
{
    // kiolvassuk a csomagot, majd konvertáljuk QString-be
    int len = m_pSocket->read(buf, sizeof(buf));
    QByteArray bytearray(buf, len);
    QString str(bytearray);

        emit packageReceived(str);

    int j = 0;
    j = str.indexOf(">", 0);
    // Kiolvassuk a kapott üzenet típusát, csonkoljuk az üzenetet
    QString msgType = str;
    msgType.resize(j+1);
    str.remove(0, j+1);

    // ServerGreeting: közölték a többi user nevét
    if(msgType == "<1>")
    {
        QString newWidgetItem;
        while((j = str.indexOf(">", 0)) != -1)
        {
            // Leválasztom a user neveket
            newWidgetItem = str;
            newWidgetItem.resize(j+1);
                emit packageReceived("user: "+newWidgetItem);
            str.remove(0, j+1);
                emit packageReceived("maradék msg: "+str);
            // Jelzek a widgetItem-nek
            emit newUserItem(newWidgetItem);
        }
    }

}

// Egy adat csomag küldésére szolgál
void NetHandler::packageSend(QString str)
{
    // QString -> char konverzió

    QByteArray ba = str.toLocal8Bit();
    const char *c_str2 = ba.data();

    m_pSocket->write(c_str2);
}

// Az üzenet típusának megfelelő csomagokat küldünk,
// először a csomag azonosítója, majd paraméterek
// str-be helyezzük korábban a cél User-eket
void NetHandler::sendMessage(MessageType msgType, QString str)
{
    switch(msgType)
    {
    case ClientGreeting:
        str.insert(0, "<0><");
        str.append(">");
        packageSend(str);
        break;
    case ServerGreeting:
        break;
    case NewClient:
        break;
    case ClientLeft:
        break;
    case PlainText:
        str.insert(0, "<4><username><");
        str.append(">");
        packageSend(str);
        break;
    }
}



















