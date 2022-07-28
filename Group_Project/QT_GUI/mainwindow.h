#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QtGui>
#include <QtCore>
#include "qextserialport.h"
#include "qextserialenumerator.h"
#include <QTimer>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    static int x[9];
    static int y[9];
    static int xl[9];
    static int yl[9];
    static int status[6];
    static int linkcheck[8];

protected:
    void changeEvent(QEvent *e);
    void paintEvent(QPaintEvent *e);        // For the QPainter

private:
    Ui::MainWindow *ui;
    QextSerialPort port;
    QMessageBox error;

    QTimer *fTimer;
    QTimer *Timer1;
    QTimer *Timer2;
    QTimer *Timer3;
    QTimer *Timer4;
    QTimer *Timer5;
    QTimer *Timer6;

    QLine line10;
    QLine line20;
    QLine line30;
    QLine line40;
    QLine line50;
    QLine line60;

    QLine line12;
    QLine line13;
    QLine line14;
    QLine line15;
    QLine line16;
    QLine line23;
    QLine line24;
    QLine line25;
    QLine line26;
    QLine line34;
    QLine line35;
    QLine line36;
    QLine line45;
    QLine line46;
    QLine line56;

    QLine lineTempTo0;
    QLine lineWindTo0;

private slots:


private slots:
    void alarm_clear1();
    void alarm_clear2();
    void alarm_clear3();
    void alarm_clear4();
    void alarm_clear5();
    void alarm_clear6();
    void timer_timeout();
    void on_pushButton_close_clicked();
    void on_pushButton_open_clicked();
    void receive();
};

#endif // MAINWINDOW_H
