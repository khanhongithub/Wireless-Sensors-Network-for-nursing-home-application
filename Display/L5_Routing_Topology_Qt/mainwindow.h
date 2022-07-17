#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QtGui>
#include <QtCore>
#include "qextserialport.h"
#include "qextserialenumerator.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);
    void paintEvent(QPaintEvent *e);        // For the QPainter

private:
    Ui::MainWindow *ui;
    QextSerialPort port;
    QMessageBox error;
    QLine line12;
    QLine line13;
    QLine line1G;
    QLine line45;
    QLine line46;
    QLine line4G;
    QLine lineTG;
    QLine lineHG;
    QLine lineWG;

private slots:


private slots:
    void on_pushButton_close_clicked();
    void on_pushButton_open_clicked();
    void receive();
    void on_HR_01_clicked();
    void on_HR_02_clicked();
    void on_HR_03_clicked();
    void on_HR_04_clicked();
    void on_HR_05_clicked();
    void on_HR_06_clicked();
};

#endif // MAINWINDOW_H
