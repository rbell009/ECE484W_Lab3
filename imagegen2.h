#ifndef IMAGEGEN2_H
#define IMAGEGEN2_H

#include <QWidget>
#include <QPushButton>
#include <QFileDialog>
#include <QLabel>
#include <QDebug>
#include <QPixmap>
#include <QPainter>
#include <QSlider>
#include <string>
#include <stdio.h>
#include <QUdpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class ImageGen2; }
QT_END_NAMESPACE

class ImageGen2 : public QWidget
{
    Q_OBJECT

public:
    ImageGen2(QWidget *parent = nullptr);
    ~ImageGen2();

    void browsing();
    void display();
    void updateDisplay();
    void exitANDsave();
    void sendData();

public slots:
    // Network Functions
    void readPending();


private:
    Ui::ImageGen2 *ui;
    QPushButton *browse;
    QPushButton *kill;
    QSlider *Contrast;
    QSlider *brightness;
    QString fileInfo;
    QLabel *orgImg;
    QPixmap img;
    int bright;
    double cont;
    bool lazy;
    QUdpSocket *clientWrite;
    QUdpSocket *clientRead;
    QHostAddress sender;
    uint16_t sender_port;
    bool ack;

};
#endif // IMAGEGEN2_H
