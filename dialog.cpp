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

    // IMITACJA PIASKU
    check_state();
    if (Zacc > 0){
        if(licznik > 40000000) {
        licznik -= 20000000;
        }}
    else {
            if (licznik < 4200000000)licznik += 20000000;
        }

    QPointF ctr1_spada(center.x(), center.y()),
            pt11_spada(center.x()+80-off, center.y()-80+off),
            pt12_spada(center.x()-80+off, center.y()-80+off),
            ctr2_spada(center.x(), center.y()+80-off),
            pt21_spada(center.x()+off, center.y()+80),
            pt22_spada(center.x()-off, center.y()+80);

    QPolygonF gora_spada;
    QPolygonF dol_spada;

    gora_spada << ctr1_spada << pt11_spada << pt12_spada << ctr1_spada;
    dol_spada << ctr2_spada << pt21_spada << pt22_spada << ctr2_spada;

    QPointF ctr1_dogory(center.x(), center.y()),
            pt11_dogory(center.x()+80-off, center.y()-80),
            pt12_dogory(center.x()-80+off, center.y()-80),
            ctr2_dogory(center.x(), center.y()-off),
            pt21_dogory(center.x()+off, center.y()+off),
            pt22_dogory(center.x()-off, center.y()+off);

    QPolygonF gora_dogory;
    QPolygonF dol_dogory;

    gora_dogory << ctr2_dogory << pt11_dogory << pt12_dogory << ctr2_dogory;
    dol_dogory << ctr1_dogory << pt21_dogory << pt22_dogory << ctr1_dogory;


    QBrush piasek;
    piasek.setColor(Qt::yellow);
    piasek.setStyle(Qt::Dense3Pattern);
    QPainterPath path_piasek_spada;
    QPainterPath path_piasek_dogory;
    path_piasek_dogory.addPolygon(gora_dogory);
    path_piasek_dogory.addPolygon(dol_dogory);
    path_piasek_spada.addPolygon(gora_spada);
    path_piasek_spada.addPolygon(dol_spada);


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

   if (Zacc > 0){
    painter.drawPolygon(gora_spada);
    painter.drawPolygon(dol_spada);
    painter.fillPath(path_piasek_spada, piasek);
    }
   else {
    painter.drawPolygon(gora_dogory);
    painter.drawPolygon(dol_dogory);
    painter.fillPath(path_piasek_dogory, piasek);
   }

}





void Dialog::calc_xy_angles(void){
   // Using x y and z from accelerometer, calculate x and y angles
   double x_val, y_val, z_val, result;
   double x2, y2, z2; //24 bit

   // Lets get the deviations from our baseline
   x_val = Xacc_g;
   y_val = Yacc_g;
   z_val = Zacc_g;

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

void Dialog::check_state()
{   int stan = whichRange(licznik);
    if (stan < 6 && stan > 0) off = (stan - 1)*20;
    else qDebug() << "blad stan" << endl;
}

int Dialog::whichRange(unsigned long int licznik)
{
    if ( licznik <= 858993459)                          return 5;
    if ( licznik >  858993459 && licznik <= 1717986918) return 4;
    if ( licznik > 1717986918 && licznik <= 2576980377) return 3;
    if ( licznik > 2576980377 && licznik <= 3865470565) return 2;
    if ( licznik > 3865470565)                          return 1;

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


        Xacc_g = ((float) Xacc * LSM303_ACC_RESOLUTION) / (float) INT16_MAX;
        Yacc_g = ((float) Yacc * LSM303_ACC_RESOLUTION) / (float) INT16_MAX;
        Zacc_g = ((float) Zacc * LSM303_ACC_RESOLUTION) / (float) INT16_MAX;

        calc_xy_angles();
          // " " << Xacc_angle<<endl;// << " " << Zacc  << endl;
        }



