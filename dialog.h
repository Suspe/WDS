#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtGui>
#include <QtCore>
#include <QtSerialPort/qserialport.h>



namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();
    void readData();
    void calc_xy_angles(void);

private:
    Ui::Dialog *ui;

    static const quint16 stm32_vendor_id = 1155;
    static const quint16 stm32_product_id = 22336;
    QString stm32_port_name;
    bool stm32_is_available;
    char buffer[50];
    qint16 Xacc, Yacc, Zacc;
    char znak;
    qint32 suma;
    float Xacc_g, Yacc_g, Zacc_g;
    bool check = 1;
    float Xacc_center, Yacc_center, Zacc_center;
    float Xacc_angle, Yacc_angle, Zacc_angle;
    int proba=0;
    QPaintEvent *e;

protected:
    void paintEvent(QPaintEvent *e);
};

#endif // DIALOG_H
