#include "nethandler.h"

NetHandler::NetHandler(QObject *parent) :
    QObject(parent)
{
    m_pSocket = NULL;
    destinationName = "notregistered";
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
        //m_pSocket[i]->flush();
        m_pSocket = NULL;
    }
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

        //emit packageReceived("raw: "+str);

    int j = 0;
    j = str.indexOf(">", 0);
    // Kiolvassuk a kapott üzenet típusát, csonkoljuk az üzenetet
    QString msgType = str;
    msgType.resize(j+1);
    str.remove(0, j+1);

    // ServerGreeting: közölték a többi user nevét
    // <1><name1><name2><name3>...
    if(msgType == "<1>")
    {
        QString newWidgetItem;
        while((j = str.indexOf(">", 0)) != -1)
        {
            // Leválasztom a user neveket
            newWidgetItem = str;
            newWidgetItem.resize(j+1);
            newWidgetItem.chop(1);
            newWidgetItem.remove(0, 1);
                //emit packageReceived("user: "+newWidgetItem);
            str.remove(0, j+1);
                //emit packageReceived("maradék msg: "+str);
            // Jelzek a widgetItem-nek
            emit newUserItem(newWidgetItem);
        }
    }
    // NewClient: új kliens csatlakozott, közölte a nevét
    // <2><new username>
    if(msgType == "<2>")
    {
        QString newWidgetItem;
        j = str.indexOf(">", 0);
        // Leválasztom a user nevet
        newWidgetItem = str;
        newWidgetItem.resize(j+1);
        newWidgetItem.chop(1);
        newWidgetItem.remove(0, 1);
            //emit packageReceived("user: "+newWidgetItem);

             //emit packageReceived("maradék msg: "+str);

        // Kiírom az új user érkezését
        emit packageReceived("Új felhasználó csatlakozott: " + newWidgetItem);
        // Jelzek a widgetItem-nek
        emit newUserItem(newWidgetItem);
    }

    // ClientLeft: egy klines lecsatlakozását jelezte a szerver
    // <3><Küldő>
    // PlainText: egyszerű üzenet érkezett
    // <4><Küldő><Üzenet>
    // ChannelMessage: üzenet mindenkienk
    // <5><Küldő><Üzenet>
    if(msgType == "<3>" || msgType == "<4>" || msgType == "<5>")
    {
        // Leválasztom a küldő nevét
        j = str.indexOf(">", 0);
        QString Sender = str;
        Sender.resize(j);
        Sender.remove(0, 1);
        // Üzenet csonkolása
        str.remove(0, j+1);
        str.remove(0, 1);
        str.chop(1);

        if(msgType == "<3>"){
            emit packageReceived("<" + Sender + " left>");
            emit removeUserItem(Sender);
        }
        if(msgType == "<4>")
            // Vizsgálni kell, hogy nem tiltott-e a küldő
            emit controlMessage(Sender, str);
            //emit packageReceived("<from " + Sender + " to you> " + str);
        if(msgType == "<5>")
            emit packageReceived("<from " + Sender + " to everyone> " + str);
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
    case ServerGreeting:    //*
        break;              //
    case NewClient:         //  Szerver küldi őket
        break;              //*
    case ClientLeft:
        str = "<3>";
        packageSend(str);
        break;
    case PlainText:
        str.insert(0, "<4>");
        str.insert(3, "<" + destinationName + "><");
        str.append(">");
        packageSend(str);
        break;
    case ChannelMessage:
        str.insert(0, "<5>");
        str.append(">");
        packageSend(str);
        break;
    }
}


















