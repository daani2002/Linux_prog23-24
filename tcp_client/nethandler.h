#ifndef NETHANDLER_H
#define NETHANDLER_H

#include <QObject>
#include <QtNetwork>

class NetHandler : public QObject
{
    Q_OBJECT

public:
    enum ConnStatus { Error, Connecting, Connected, Disconnected };

    enum MessageType {ClientGreeting,   // Kliens közli a szerverrel a felhasználónevet
                      ServerGreeting,   // Szerver közli a többi felhasználó nevét
                      NewClient,        // Új kliens csatlakozott, közölte nevét
                      ClientLeft,       // Egy kliens lecsatlakozott a szerverről
                      PlainText};       // Üzenet küldése a megadott címzetteknek
public:
    explicit NetHandler(QObject *parent = 0);
    virtual ~NetHandler();

    void Connect(QString addr);

    void setUserName(QString username){userName = username;}
    QString getUserName(){return userName;}
    void setDestinationName(QString str){destinationName = str;}
    QString getDestinationName(){return destinationName;}

signals:
    void signalConnectionStatus(int status);
    void packageReceived(QString str);
    void newUserItem(QString);

public slots:
    void slotReadyRead();
    void packageSend(QString str);
    void sendMessage(MessageType msgType, QString str);
    void slotDisconnected();

protected:
    QTcpSocket* m_pSocket;
    QString userName;           // Saját felhasználónév
    QString destinationName;    // Cél felhasználó neve
    char buf[1024];
};

#endif // NETHANDLER_H
