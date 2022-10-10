#include "imagegen2.h"
#include "udpclient.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    udpClient client;

    ImageGen2 w;
    w.show();
    return a.exec();
}
