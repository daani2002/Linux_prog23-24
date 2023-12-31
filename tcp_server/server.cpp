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

    // NEM MŰKÖDÖTT JÓL
    //connect(pSocket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));

    // 20 kapcsolatot fogadunk elsőkörben, mindig az első szabad socket pointert használjuk
    for(int i = 0; i < MaxClientNum; i++)
    if (m_pSocket[i] == NULL) {
        m_pSocket[i] = pSocket;
        connect(m_pSocket[i], SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
        //m_pSocket[i]->write("sikeres kapcsolat");
        break;
    }

    // Seed előállítása aktuális időpont alapján
    QTime current = QTime::currentTime();
    int msecs = current.msec();
    QString seed = QString::number(msecs);
    QString str;
    // Seed továbbításához használt szabványos üzenet
    str = "<6><" + seed + ">";
    QByteArray ba = str.toLocal8Bit();
    const char *c_str = ba.data();
    pSocket->write(c_str);

    // Hash kódolás
    QCryptographicHash hashObject(QCryptographicHash::Sha1);
    ba = seed.toLocal8Bit();
    hashObject.addData(ba);
    ba = hashObject.result();
    // Hash kód kiírása
    emit textReceived("Hash: " + QString(ba));

    // Szerver jelszó állítása a hash kódra
    // Jó esetben a kliens oldalon is ez jön majd ki
    serverPassword = QString(ba);
}

// A kapcsolat lezarasanak erzekelese.
// NEM IGAZÁN MŰKÖDIK
void Server::slotDisconnected()
{
    disconnect(this, SLOT(slotDisconnected()));

    QString username;

    // Ha letezik meg kliens socket akkor lezarjuk.
    for(int i = 0; i < MaxClientNum; i++)
    if (m_pSocket[i] && !m_pSocket[i]->isOpen()) {
        m_pSocket[i]->deleteLater();
        m_pSocket[i] = NULL;
    }

    // Megkeressük a lezárt socket-hez tartozó user nevet
    for(int i = 0; i < MaxClientNum; i++)
        if(m_pSocket[i] == NULL && userName[i] != "not registered")
        {
            username = userName[i];
            userName[i] = "not registered";
                emit textReceived(username);
        }

    // Többi kliensnek ClientLeft üzenet
    // <3>
    QString str = "<3>";
    str.append("<" + username + ">");
    // QString->char* konverzió
    QByteArray ba = str.toLocal8Bit();
    const char *c_str = ba.data();
    for(int i = 0; i < MaxClientNum; i++)
    {
        if(m_pSocket[i])
        {
            m_pSocket[i]->write(c_str);
            emit textReceived("out: " + str);
        }
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

        // Char* -> QString konverzió, így könnyebb a string manipuláció
        QByteArray ba(buf, len);
        QString str(ba);
            // Jelezzük az olvasást a megjelenítő ablaknak
            emit textReceived("in: " + str);

        int j = 0;
        j = str.indexOf(">", 0);
        // Kiolvassuk a kapott üzenet típusát
        QString msgType = str;
        msgType.resize(j+1);
        str.remove(0, j+1);

        // ClientGreeting: közölték a sockethez tartozó felhasználónevet és jelszót
        // Visszaküldjük a többiek nevét
        // A többieknek elküldjük az új user nevét
        if(msgType == "<0>")
        {
            // Kiolvassuk a küldő nevét
            j = str.indexOf(">", 0);
            QString Sender = str;
            Sender.resize(j);
            Sender.remove(0, 1);
            // Csonkoljuk az üzenetet
            str.remove(0, j+1);

            // Kiolvassuk a jelszót
            j = str.indexOf(">", 0);
            QString Psw = str;
            Psw.resize(j);
            Psw.remove(0, 1);


            // Ha nem egyeznek, nem regisztráljuk a klienst
            if(Psw != serverPassword){
                m_pSocket[i]->deleteLater();
                m_pSocket[i] = NULL;
                return;
            }

            // Elmentjük a socket indexén
            userName[i] = Sender;

            // Összeírjuk a többi felhasználónevet
            QString otherUsers;
            for(int x = 0; x < MaxClientNum; x++)
                if(m_pSocket[x] != NULL && x != i)
                    otherUsers.append("<" + userName[x] + ">");

            // ServerGreeting üzenet küldése
            // amiben közöljük a többi user nevét
            // <1><name1><name2><name3>...
            otherUsers.prepend("<1>");
            ba = otherUsers.toLocal8Bit();
            const char *c_otherUsers = ba.data();
            m_pSocket[i]->write(c_otherUsers);

            // NewClient üzenet a többi kliensnek
            // <2><új username>
            QString NewClientMsg = "<2><" + userName[i] + ">";
            ba = NewClientMsg.toLocal8Bit();
            const char *c_NewClientMsg = ba.data();

            for(int j = 0; j < MaxClientNum; j++){
                if(m_pSocket[j] != NULL && j != i){
                    m_pSocket[j]->write(c_NewClientMsg);
                    emit textReceived("out: " + NewClientMsg);
                }
            }
        }

        // ClientLeft: egy kliens lecsatlakozott
        // érkezett: <3>
        // küldött:  <3><küldő>
        if(msgType == "<3>")
        {
            str.append("<3><" + userName[i] + ">");
            ba = str.toLocal8Bit();
            const char* c_str = ba.data();

            // A többi kliensnek küldjük a kilépő nevét
            for(int x = 0; x < MaxClientNum; x++)
            {
                if(m_pSocket[x] != NULL && x != i)
                {
                    m_pSocket[x]->write(c_str);
                        emit textReceived("out: " + str);
                }
            }

            // Kikotjuk a socket lezarasanak erzekeleset, mert kulonben a sajatunkat is
            // erzekelnenk.
            //disconnect(this, SLOT(slotDisconnected()));

            // Ha letezik meg kliens socket akkor lezarjuk.
            if (m_pSocket[i]) {
                m_pSocket[i]->deleteLater();
                m_pSocket[i] = NULL;
                userName[i] = "not registered";
            }
        }

        // PlainText: egyszerű üzenetet küldtek adott személyeknek
        // érkezett: <4><cél><üzenet>
        if(msgType == "<4>")
        {
            // Levágjuk a cél user nevét
            j = str.indexOf(">", 0);
            QString destination = str;
            destination.resize(j+1);
            destination.remove(0, 1);
            destination.chop(1);
            str.remove(0, j+1);


            // Összeállítjuk az üzenet keretet
            // <4><küldő><üzenet>
            str.prepend("<" + userName[i] + ">");
            str.prepend("<4>");

            // QString -> char*
            ba = str.toLocal8Bit();
            const char *c_str = ba.data();

            // Megkeressük a destinationnak megfelelő user-t és socket-et
            for(int x = 0; x < MaxClientNum; x++)
                if(QString::compare(userName[x], destination, Qt::CaseInsensitive) == 0)//userName[x] == destination)
                {
                    m_pSocket[x]->write(c_str);
                    emit textReceived("out: "+str);
                }
        }
        // ChannelMessage: mindenkinek továbbítjuk az üzenetet
        // érkezett: <5><üzenet>
        // küldött:  <5><küldő><üzenet>
        if(msgType == "<5>")
        {
            // Küldő név beszúrása
            str.prepend("<5><" + userName[i] + ">");
            // QString -> char*
            ba = str.toLocal8Bit();
            const char *c_str = ba.data();

            for(int x = 0; x < MaxClientNum; x++)
            {
                if(m_pSocket[x] != NULL && x != i)  // magunknak nem küldjük megint
                {
                    m_pSocket[x]->write(c_str);
                        emit textReceived("out: " + str);
                }
            }
        }

    }
    }
}































