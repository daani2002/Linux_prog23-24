#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QtNetwork>
#include "mainwindow.h"

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    virtual ~Server();

    void StartServer();

signals:
    void serverStarted();
    void packageReceived(char*, int);

public slots:
    void slotIncomingConn();
    void slotDisconnected();
    void slotReadyRead1();

protected:
    QTcpServer* m_pServSocket;
    QTcpSocket* m_pSocket1;

    // Temporalis buffer az erkezo adatok tarolasara.
    char buf[1024];
};

#endif // SERVER_H
