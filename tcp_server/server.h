#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QtNetwork>
#include "mainwindow.h"

static const int MaxClientNum = 20;

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    virtual ~Server();

    void StartServer();

signals:
    void serverStarted();
    void textReceived(QString);

public slots:
    void slotIncomingConn();
    void slotDisconnected();
    void slotReadyRead();

protected:
    QTcpServer* m_pServSocket;
    QTcpSocket* m_pSocket[MaxClientNum];
    QString     userName[MaxClientNum];
    QString     serverPassword;

    // Temporalis buffer az erkezo adatok tarolasara.
    char buf[1024];
};

#endif // SERVER_H
