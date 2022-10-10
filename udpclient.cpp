#include "udpclient.h"

udpClient::udpClient(QObject *parent)
    : QObject{parent}
{
    QByteArray Testing;
    Testing.append("This is from Client");

    cliSock = new QUdpSocket(this);

    cliSock->bind(QHostAddress("192.168.0.150"), 1234);

    connect(cliSock, SIGNAL(readyRead()),this,SLOT(readReady()));

    cliSock->writeDatagram(Testing, QHostAddress("192.168.0.150"),1234);
}

void udpClient::readReady()
{

}
