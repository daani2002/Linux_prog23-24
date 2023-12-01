#ifndef NETHANDLER_H
#define NETHANDLER_H

#include <QObject>
#include <QtNetwork>

class NetHandler : public QObject
{
    Q_OBJECT

public:
    enum ConnStatus { Error, Connecting, Connected, Disconnected };
public:
    explicit NetHandler(QObject *parent = 0);
    virtual ~NetHandler();
    void Connect(QString addr);

signals:
    void signalConnectionStatus(int status);
    void packageReceived(QString str);

public slots:
    void slotReadyRead();
    void packageSend(QString str);
    void slotDisconnected();

protected:
    QTcpSocket* m_pSocket;
    QString userName;
    char buf[1024];
};

#endif // NETHANDLER_H
