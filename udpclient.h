#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QObject>
#include <QUdpSocket>

class udpClient : public QObject
{
    Q_OBJECT
public:
    explicit udpClient(QObject *parent = nullptr);

signals:

public slots:
    void readReady();

private:
    QUdpSocket *cliSock;

};

#endif // UDPCLIENT_H
