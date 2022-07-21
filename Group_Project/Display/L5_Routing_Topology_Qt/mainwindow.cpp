#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qdebug.h>
#include<bits/stdio.h>
#include<string>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    line12 = QLine();
    line13 = QLine();
    line1G = QLine();
    line45 = QLine();
    line46 = QLine();
    line4G = QLine();
    lineTG = QLine();
    lineWG = QLine();
    lineHG = QLine();

    ui->setupUi(this);

    // Get all available COM Ports and store them in a QList.
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();

    // Read each element on the list, but
    // add only USB ports to the combo box.
    for (int i = 0; i < ports.size(); i++) {
        if (ports.at(i).portName.contains("USB")){
            ui->comboBox_Interface->addItem(ports.at(i).portName.toLocal8Bit().constData());
        }
    }
    // Show a hint if no USB ports were found.
    if (ui->comboBox_Interface->count() == 0){
        ui->textEdit_Status->insertPlainText("No USB ports available.\nConnect a USB device and try again.");
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
       /* ui->retranslateUi(this); */
        break;
    default:
        break;
    }
}

void MainWindow::on_pushButton_open_clicked()
{
    port.setQueryMode(QextSerialPort::EventDriven);
    port.setPortName("/dev/" + ui->comboBox_Interface->currentText());
    port.setBaudRate(BAUD115200);
    port.setFlowControl(FLOW_OFF);
    port.setParity(PAR_NONE);
    port.setDataBits(DATA_8);
    port.setStopBits(STOP_1);
    port.open(QIODevice::ReadWrite);

    if (!port.isOpen())
    {
        error.setText("Unable to open port!");
        error.show();
        return;
    }

    QObject::connect(&port, SIGNAL(readyRead()), this, SLOT(receive()));

    ui->pushButton_close->setEnabled(true);
    ui->pushButton_open->setEnabled(false);
    ui->comboBox_Interface->setEnabled(false);
}

void MainWindow::on_pushButton_close_clicked()
{
    if (port.isOpen())port.close();
    ui->pushButton_close->setEnabled(false);
    ui->pushButton_open->setEnabled(true);
    ui->comboBox_Interface->setEnabled(true);
}

void MainWindow::receive()
{
    QPalette pal = ui->HR_01->palette();
    pal.setColor(QPalette::Button, QColor(Qt::white));
    ui->HR_01->setAutoFillBackground(true);
    ui->HR_01->setPalette(pal);
    ui->HR_01->update();

    ui->GW_mote->setAutoFillBackground(true);
    ui->GW_mote->setPalette(pal);
    ui->GW_mote->update();

    ui->HR_02->setAutoFillBackground(true);
    ui->HR_02->setPalette(pal);
    ui->HR_02->update();

    ui->HR_03->setAutoFillBackground(true);
    ui->HR_03->setPalette(pal);
    ui->HR_03->update();

    ui->T_mote->setAutoFillBackground(true);
    ui->T_mote->setPalette(pal);
    ui->T_mote->update();

    ui->HR_04->setAutoFillBackground(true);
    ui->HR_04->setPalette(pal);
    ui->HR_04->update();

    ui->HR_05->setAutoFillBackground(true);
    ui->HR_05->setPalette(pal);
    ui->HR_05->update();


    ui->H_mote->setAutoFillBackground(true);
    ui->H_mote->setPalette(pal);
    ui->H_mote->update();

    ui->HR_06->setAutoFillBackground(true);
    ui->HR_06->setPalette(pal);
    ui->HR_06->update();

    ui->W_mote->setAutoFillBackground(true);
    ui->W_mote->setPalette(pal);
    ui->W_mote->update();


    pal.setColor(QPalette::Button, QColor(Qt::green));
    ui->HUMIDITY->setAutoFillBackground(true);
    ui->HUMIDITY->setPalette(pal);
    ui->HUMIDITY->update();

    ui->WIND->setAutoFillBackground(true);
    ui->WIND->setPalette(pal);
    ui->WIND->update();

    static QString str;
    char ch;
    int xT= ui->T_mote->pos().x()+15;
    int yT= ui->T_mote->pos().y()+30;
    int xG= ui->GW_mote->pos().x()+15;
    int yG= ui->GW_mote->pos().y()+30;


    int n = 0;
        while (port.getChar(&ch))
        {

            n++;
            int x1= ui->HR_01->pos().x()+15;
            int y1= ui->HR_01->pos().y()+30;

            int x2= ui->HR_02->pos().x()+15;
            int y2= ui->HR_02->pos().y()+30;
            //if(ch == 'T') lineTG.setLine(xT,yT,xG,yG);
            if(ch == 'T') {
                line12.setLine(x1,y1,x2,y2);
                line1G.setLine(x1,y1,xG,yG);
                this->repaint();
                QPalette pal = ui->HR_01->palette();
                pal.setColor(QPalette::Button, QColor(Qt::blue));
                ui->HR_01->setAutoFillBackground(true);
                ui->HR_01->setPalette(pal);
                ui->HR_01->update();
                pal = ui->GW_mote->palette();
                pal.setColor(QPalette::Button, QColor(Qt::blue));
                ui->GW_mote->setAutoFillBackground(true);
                ui->GW_mote->setPalette(pal);
                ui->GW_mote->update();
                pal.setColor(QPalette::Button, QColor(Qt::blue));
                ui->HR_02->setAutoFillBackground(true);
                ui->HR_02->setPalette(pal);
                ui->HR_02->update();

            }


            this->repaint();
            str.append(ch);

            if (ch == '\n')     // End of line, start decoding
            {
                QString subs = str.mid((str.size()-n),n-1);
                QStringRef subString;

                if (subs.at( 0 ) == 'T'){

                subString = QStringRef(&subs, 13,4);
                int temp = subString.toInt();
                ui->T_value -> setText(QString::number(temp));
                if(temp > 2496)
                {
                    pal.setColor(QPalette::Button, QColor(Qt::red));
                    ui->TEMP->setAutoFillBackground(true);
                    ui->TEMP->setPalette(pal);
                    ui->TEMP->update();
                }
                else
                {
                    pal.setColor(QPalette::Button, QColor(Qt::green));
                    ui->TEMP->setAutoFillBackground(true);
                    ui->TEMP->setPalette(pal);
                    ui->TEMP->update();
                }
                }

                qDebug() << subs;

                qDebug() << subString;

                n=0;
                
                str.remove("\n", Qt::CaseSensitive);
                ui->textEdit_Status->append(str);

                lineTG.setLine(0,0,0,0);
                line12.setLine(0,0,0,0);
                line1G.setLine(0,0,0,0);
                QPalette pal = ui->HR_01->palette();
                pal.setColor(QPalette::Button, QColor(Qt::white));
                ui->HR_01->setAutoFillBackground(true);
                ui->HR_01->setPalette(pal);
                ui->HR_01->update();
                ui->GW_mote->setAutoFillBackground(true);
                ui->GW_mote->setPalette(pal);
                ui->GW_mote->update();
                ui->HR_02->setAutoFillBackground(true);
                ui->HR_02->setPalette(pal);
                ui->HR_02->update();



                // Insert your code here - *Use parts of previous exercises if needed


                this->repaint();    // Update content of window immediately
                str.clear();
            }
        }
}

void MainWindow::paintEvent(QPaintEvent *e){
    QPainter painter(this);
    int coordinateX = 200; int coordinateY = 60;
    int rectwidth = 400; int rectheight = 290;
    painter.eraseRect(coordinateX, coordinateY, rectwidth, rectheight);   // Clean the designated painting area

    QRect rec(coordinateX, coordinateY, rectwidth, rectheight);
    QPen framepen(Qt::red);
    framepen.setWidth(4);
    painter.drawRect(rec);  // Draw the designated painting area
    painter.drawLine(line12);// for repainting
    painter.drawLine(line13);
    painter.drawLine(lineTG);
    painter.drawLine(lineHG);
    painter.drawLine(lineWG);
    painter.drawLine(line1G);
    painter.drawLine(line45);
    painter.drawLine(line46);
    painter.drawLine(line4G);





}

void MainWindow::on_HR_01_clicked()
{

    int temp1 = ui->HR_01->pos().x();
    int temp2 = ui->HR_01->pos().y();
    temp1 = temp1 + rand()%60 -30;
    temp2 = temp2 + rand()%60 -30;
    if(temp1<600&&temp1>250&&temp2<340&&temp2>100)
    ui->HR_01->setGeometry(temp1,temp2,31,31);

}
void MainWindow::on_HR_02_clicked()
{
    int temp1 = ui->HR_02->pos().x();
    int temp2 = ui->HR_02->pos().y();
    temp1 = temp1 + rand()%60 -30;
    temp2 = temp2 + rand()%60 -30;
    if(temp1<600&&temp1>250&&temp2<340&&temp2>100)
    ui->HR_02->setGeometry(temp1,temp2,31,31);
}
void MainWindow::on_HR_03_clicked()
{
    int temp1 = ui->HR_03->pos().x();
    int temp2 = ui->HR_03->pos().y();
    temp1 = temp1 + rand()%60 -30;
    temp2 = temp2 + rand()%60 -30;
    if(temp1<600&&temp1>250&&temp2<340&&temp2>100)
    ui->HR_03->setGeometry(temp1,temp2,31,31);
}
void MainWindow::on_HR_04_clicked()
{
    int temp1 = ui->HR_04->pos().x();
    int temp2 = ui->HR_04->pos().y();
    temp1 = temp1 + rand()%60 -30;
    temp2 = temp2 + rand()%60 -30;
    if(temp1<600&&temp1>250&&temp2<340&&temp2>100)
    ui->HR_04->setGeometry(temp1,temp2,31,31);
}
void MainWindow::on_HR_05_clicked()
{
    int temp1 = ui->HR_05->pos().x();
    int temp2 = ui->HR_05->pos().y();
    temp1 = temp1 + rand()%60 -30;
    temp2 = temp2 + rand()%60 -30;
    if(temp1<600&&temp1>250&&temp2<340&&temp2>100)
    ui->HR_05->setGeometry(temp1,temp2,31,31);
}
void MainWindow::on_HR_06_clicked()
{
    int temp1 = ui->HR_06->pos().x();
    int temp2 = ui->HR_06->pos().y();
    temp1 = temp1 + rand()%60 -30;
    temp2 = temp2 + rand()%60 -30;
    if(temp1<600&&temp1>250&&temp2<340&&temp2>100)
    ui->HR_06->setGeometry(temp1,temp2,31,31);
}
