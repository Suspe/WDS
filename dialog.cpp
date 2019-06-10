#include "dialog.h"
#include "ui_dialog.h"
#include <QtSerialPort/qserialport.h>
#include <QDebug>
#include <QtSerialPort/QSerialPortInfo>
#include <QPlainTextEdit>
#include <QTextStream>
#include <QMessageBox>
#include <QLabel>
#include <QScrollBar>
#include <QTextEdit>
#include <sstream>      // std::istringstream
#include <limits.h>
#include <QtMath>
#define LSM303_ACC_RESOLUTION 2.0

QSerialPort *serial;
Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    serial = new QSerialPort();
    stm32_is_available = false;
    stm32_port_name = "";
    //ui->plot->addGraph();
    //ui->plot->graph(0)->setScatterStyle(QCPScatterStyle::ssTriangle);
   // ui->plot->graph(0)->setLineStyle(QCPGraph::lsNone);

    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
    if(serialPortInfo.hasVendorIdentifier() && serialPortInfo.hasProductIdentifier())
        if(serialPortInfo.vendorIdentifier() == stm32_vendor_id)
            if(serialPortInfo.productIdentifier() == stm32_product_id){
                stm32_port_name = serialPortInfo.portName();
                stm32_is_available = true;
            }
    }

    if (stm32_is_available){
        //open and configure the port
        serial->setPortName(stm32_port_name);
        serial->open(QIODevice::ReadWrite);
        serial->setBaudRate(QSerialPort::Baud9600);
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);
       connect(serial, &QSerialPort::readyRead, this, &Dialog::readData);




    }
    else{
        // give error message
        QMessageBox::warning(this, "port error", "can't find any stm!");
    }
}

Dialog::~Dialog()
{
    if(serial->isOpen()){
        serial->close();
    }
    delete ui;
    serial->close();

}

void Dialog::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.device()->width();

    QPen paintpen(Qt::black);
    painter.setRenderHint(QPainter::Antialiasing);
    paintpen.setWidth(2);
    paintpen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(paintpen);
    QPointF center(this->width()/2,this->height()/2);
    QPointF p1(center.x()-80,center.y()-100),
            p2(center.x()+80,center.y()-100),
            p3(center.x()-80,center.y()-80),
            p4(center.x()+80,center.y()-80),
            p5(center.x(),center.y()),
            p6(center.x()-80,center.y()+80),
            p7(center.x()+80,center.y()+80),
            p8(center.x()-80,center.y()+100),
            p9(center.x()+80,center.y()+100);
    //this->setFixedSize(400,400);
    //p1.setX(10.2);
   // p1.setY(10.5);
    QPolygonF rec_top;
    QPolygonF rec_bot;
    QPolygonF tr_top;
    QPolygonF tr_bot;
    rec_top << p1 << p2 << p4 << p3 << p1;
    tr_top << p3 << p4 << p5 << p3;
    rec_bot << p6 << p7 << p9 << p8 << p6;
    tr_bot << p6 << p7 << p5 << p6;

    QBrush fill1;
    QBrush fill2;
    fill1.setColor(Qt::black);
    fill1.setStyle(Qt::Dense2Pattern);
    fill2.setColor(Qt::blue);
    fill2.setStyle(Qt::FDiagPattern);
    QPainterPath path_tr;
    QPainterPath path_rec;

    path_rec.addPolygon(rec_bot);
    path_rec.addPolygon(rec_top);
    path_tr.addPolygon(tr_bot);
    path_tr.addPolygon(tr_top);


    painter.translate(center.x(),center.y());
    if (Zacc>0)painter.rotate(Yacc_angle);
    if (Zacc<0){
        if (Yacc_angle<0)painter.rotate(-180-Yacc_angle);
        else painter.rotate(180 - Yacc_angle);
    }
    painter.translate(-center.x(),-center.y());



    painter.drawPolygon(rec_top);
    painter.drawPolygon(rec_bot);
    painter.drawPolygon(tr_top);
    painter.drawPolygon(tr_bot);


    painter.fillPath(path_rec, fill1);
    painter.fillPath(path_tr, fill2);


}


void Dialog::calc_xy_angles(void){
   // Using x y and z from accelerometer, calculate x and y angles
   double x_val, y_val, z_val, result;
   double x2, y2, z2; //24 bit

   // Lets get the deviations from our baseline
   x_val = Xacc_g-Xacc_center;
   y_val = Yacc_g-Yacc_center;
   z_val = Zacc_g-Zacc_center;

   // Work out the squares
   x2 = x_val*x_val;
   y2 = y_val*y_val;
   z2 = z_val*z_val;

   //X Axis
   result=sqrt(y2+z2);
  //  qDebug() << "X: " <<result<<endl;
   result=x_val/result;

   Xacc_angle = atan(result);
   Xacc_angle = Xacc_angle * 180 / 3.1415;


   //Y Axis
   result=sqrt(x2+z2);
  // qDebug() << "Y: " <<result<<endl;

   result=y_val/result;
   Yacc_angle = atan(result);
   Yacc_angle = Yacc_angle * 180 /3.1415;
}

void Dialog::readData()
    {

    Dialog::update();
    //Sleep(100);
        const QByteArray data = serial->readAll();
        QDataStream input(serial->readAll());
        QByteArray ba;
        QDataStream writer(&ba, QIODevice::WriteOnly);
        writer.writeBytes(data.constData(), data.length());
        QDataStream reader(ba);
        char* raw;
        uint length;
        reader.readBytes(raw, length);
        std::istringstream IStrm(raw);
        if (IStrm.good()==true){
        IStrm >> znak >> Xacc >> Yacc >> Zacc >> suma;
       }
        if ((znak != 'X') || (Xacc+Yacc+Zacc!=suma)){
            Xacc =0;
            Yacc =0;
            Zacc =0;
            qDebug() << "blad komuinkacji" << endl;
        }

        else {
            if (check){
                Xacc_center = Xacc_g;
                Yacc_center = Yacc_g;
                Zacc_center = Zacc_g;
                check = 0;
            }
        Xacc_g = ((float) Xacc * LSM303_ACC_RESOLUTION) / (float) INT16_MAX;
        Yacc_g = ((float) Yacc * LSM303_ACC_RESOLUTION) / (float) INT16_MAX;
        Zacc_g = ((float) Zacc * LSM303_ACC_RESOLUTION) / (float) INT16_MAX;

        calc_xy_angles();
         // qDebug() << Yacc_angle <<endl;// " " << Xacc_angle<<endl;// << " " << Zacc  << endl;
        }

    }

