#include "imagegen2.h"
#include "./ui_imagegen2.h"
#include "ui_imagegen2.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

using namespace cv;

ImageGen2::ImageGen2(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ImageGen2)
{
    // Labels for Browser, Brightness Bar, and Contrast Bar
    QLabel *browseL = new QLabel("Open Original Image", this);
    QLabel *brightL = new QLabel("Change Brightness", this);
    QLabel *contrastL = new QLabel("Change Contrast", this);

    // Label that will store Pixmap
    orgImg = new QLabel(this);

    // Configuration of Browse Button
    browse = new QPushButton("Browse Files", this);
    browse -> setGeometry(QRect(100,75,150,25));
    connect(browse, &QPushButton::released, this, &ImageGen2::display);

    // Browser Label Size and Location settings
    browseL -> setGeometry(QRect(125,50,100,25));
    
    // Brightness Bar settings and location
    brightness = new QSlider(Qt::Horizontal,this);
    brightness -> setRange(-50,50);
    brightness -> setValue(0);
    brightness -> setGeometry(QRect(450,300,200,50));
    connect(brightness,&QSlider::sliderReleased,this,&ImageGen2::updateDisplay);
    brightL -> setGeometry(QRect(450,275,100,25));
    
    // Contrast Bar settings and location
    Contrast = new QSlider(Qt::Horizontal,this);
    Contrast -> setRange(1,200);
    Contrast -> setValue(100);
    Contrast -> setGeometry(QRect(450,400,200,50));
    connect(Contrast,&QSlider::sliderReleased,this,&ImageGen2::updateDisplay);
    contrastL -> setGeometry(QRect(450,375,100,25));

    // Initial values for Brightness and Contrast
    bright = 0;
    cont = 1.0;
    lazy = false;

    // Save and Exit Button settings
    kill = new QPushButton("Save and Exit", this);
    kill -> setGeometry(QRect(100,500,150,25));
    connect(kill,&QPushButton::released,this,&ImageGen2::exitANDsave);

    // Network Configuration
    ack = false;
    clientRead = new QUdpSocket(this);
    clientWrite = new QUdpSocket(this);

    clientRead -> bind(QHostAddress::LocalHost, clientRead->localPort());
    clientWrite -> connectToHost(QHostAddress("192.168.0.97"),45546);

    QString initMSG = "Start Coms";
    QByteArray msg = initMSG.toUtf8();

    connect(clientRead, SIGNAL(readyRead()),this, SLOT(readPending()));

    clientWrite -> write(msg);

    ui->setupUi(this);
}

ImageGen2::~ImageGen2()
{
    delete ui;
}

// Function Purpose: Simple browsing function to grab file location for initial image
void ImageGen2::browsing()
{
    // Grabs file path from users selection (pops up with file explorer)
    QString filePath = QFileDialog::getOpenFileName(this, tr("Find File"), QDir::currentPath());

    // Confirms is a path was selected and displays path in console to confirm
    if (!filePath.isNull())
    {
        qDebug() << "selected file path : " << filePath.toUtf8();
    }

    // Stores File Path
    fileInfo = filePath;
}

void ImageGen2::display()
{
    int tempH, tempW;

    // Grabs File Info
    browsing();

    // If Nothing, kills fucntion
    if (fileInfo.isNull())
    {
        return;
    }

    // Sets Location and size of the label that will hold image
    orgImg -> setGeometry(QRect(75,125,300,200));

    // Grab dimensions of label
    tempH = orgImg->height();
    tempW = orgImg->width();

    // Generate Pixmap from file info, then sets pixmap on label
    QPixmap temp(fileInfo);
    img = temp;
    orgImg -> setPixmap(temp.scaled(tempW,tempH,Qt::KeepAspectRatio));
}

// Function Purpose: Update Display when changes to either Brightness or Contrast are made
void ImageGen2::updateDisplay()
{
    int tempB;
    double tempC;

    // Network Trigger
    sendData();

    // Lazy exists to avoid errors with algorithm
    if (lazy == false)
    {
        // Pulls original image from stored file info
        Mat orgImg1 = imread(fileInfo.toStdString(),IMREAD_GRAYSCALE);

        // Creates empty image to store updated image in
        Mat newImg = Mat::zeros(orgImg1.size(), orgImg1.type());

        // Grabs current Contrast and Brightness data, Contrast is divided by 100 to scale it down.
        tempB = brightness->value();
        tempC = Contrast->value() / 100.0;

        // Loop to complete Brightness and Contrast adjustments, multiplies pixel value by contrast value,
        // and adds the brightness value (if greater than previous brightness) or subtracts.
        for (int y = 0; y < orgImg1.rows; y++) {
            for (int x = 0; x < orgImg1.cols; x++) {
                newImg.at<uchar>(y,x) = saturate_cast<uchar>(tempC * double(orgImg1.at<uchar>(y,x)) + tempB);
            }
        }

        // Write Image to temp file, to store edited image.
        imwrite("temp.bmp",newImg);

        // Replaces Image to new temp file
        QImage replace((const unsigned char*)newImg.data,newImg.cols, newImg.rows, QImage::Format_Indexed8);
        QPixmap temp;
        temp.convertFromImage(replace);

        orgImg -> setPixmap(temp.scaled(orgImg->width(),orgImg->height(),Qt::KeepAspectRatio));
        orgImg -> show();

        lazy = true;
        img = temp;
    }
    else
    {
        // This condition operates the same except it pulls the temp file rather than the original

        Mat upImg = imread("temp.bmp", IMREAD_GRAYSCALE);

        Mat newImg = Mat::zeros(upImg.size(),upImg.type());

        tempB = brightness -> value();
        tempC = Contrast -> value() / 100.0;

        for (int y = 0; y < upImg.rows; y++) {
            for (int x = 0; x < upImg.cols; x++) {
                newImg.at<uchar>(y,x) = saturate_cast<uchar>(tempC * double(upImg.at<uchar>(y,x)) + tempB);
            }
        }

        imwrite("temp.bmp",newImg);

        QImage replace((const unsigned char*)newImg.data,newImg.cols, newImg.rows, QImage::Format_Indexed8);
        QPixmap temp;
        temp.convertFromImage(replace);

        orgImg -> setPixmap(temp.scaled(orgImg->width(),orgImg->height(),Qt::KeepAspectRatio));
        orgImg -> show();

        cont = tempC;
        bright = tempB;

        img = temp;
    }



}

// Function Purpose: To Save and Exit the program while also deleting temp image.
void ImageGen2::exitANDsave()
{
    QImage outputImg = img.toImage();

    // Grab Save location
    QString saveLocation = QFileDialog::getSaveFileName(this,tr("Save Image File"), QString());

    if (!saveLocation.isEmpty())
    {
        outputImg.save(saveLocation);
    }

    // Delete Temp Image File
    remove("temp.bmp");

    // Kills Program/GUI
    this -> close();
}


void ImageGen2::sendData()
{
    int contrastD, brightD;
    String contrastS, brightS,tempC,tempB, msg;
    QString holdMSG;
    QByteArray sendMSG;

    tempC = "0";
    tempB = "0";

    contrastD = Contrast->value() / 2;
    brightD = brightness->value() + 50;

    if (contrastD < 10)
    {
        contrastS = tempC + std::to_string(contrastD);
    }
    else
    {
        contrastS = std::to_string(contrastD);
    }

    if (brightD < 10)
    {
        brightS = tempB + std::to_string(brightD);
    }
    else
    {
        brightS = std::to_string(brightD);
    }

    msg = brightS + contrastS;
    holdMSG = QString::fromStdString(msg);

    sendMSG = holdMSG.toUtf8();

    clientWrite -> write(sendMSG);

}

void ImageGen2::readPending()
{
    QByteArray DataIn;


    while (clientRead->hasPendingDatagrams()) {
        clientRead->readDatagram(DataIn.data(),-1,&sender,&sender_port);

        QString DataMsg = QString(DataIn);

        qDebug() << "Message From Server: " << DataMsg;
    }
}

