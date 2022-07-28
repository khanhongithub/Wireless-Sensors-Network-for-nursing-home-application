#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qdebug.h>
#include <QTimer>


int MainWindow::x[9] = {470,470,420,770,770,820,620,620,620};
int MainWindow::y[9] = {195,415,305,210,430,320,310,160,460}; //coordinates of the nodes
int MainWindow::xl[9] = {495,495,445,795,795,845,645,645,645};
int MainWindow::yl[9] = {220,440,330,235,455,345,335,185,485}; //coordinates of the lines

int MainWindow::status[6] = {0,0,0,0,0,0};
int MainWindow::linkcheck[8] = {1,1,1,1,1,1,1,1};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    qDebug()<<"x="<<x;
    qDebug()<<"y="<<y;
    qDebug()<<"xl="<<xl;
    qDebug()<<"yl="<<yl;
    ui->setupUi(this);

    fTimer = new QTimer(this);
    Timer1 = new QTimer(this);
    Timer2 = new QTimer(this);
    Timer3 = new QTimer(this);
    Timer4 = new QTimer(this);
    Timer5 = new QTimer(this);
    Timer6 = new QTimer(this);
    fTimer->stop();
    fTimer->setInterval(25000);
    connect(fTimer,SIGNAL(timeout()),this,SLOT(timer_timeout()));
    connect(Timer1,SIGNAL(timeout()),this,SLOT(alarm_clear1()));
    connect(Timer2,SIGNAL(timeout()),this,SLOT(alarm_clear2()));
    connect(Timer3,SIGNAL(timeout()),this,SLOT(alarm_clear3()));
    connect(Timer4,SIGNAL(timeout()),this,SLOT(alarm_clear4()));
    connect(Timer5,SIGNAL(timeout()),this,SLOT(alarm_clear5()));
    connect(Timer6,SIGNAL(timeout()),this,SLOT(alarm_clear6()));


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
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::on_pushButton_open_clicked()
{
    fTimer->start();

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
    fTimer->stop();
    if (port.isOpen())port.close();
    ui->pushButton_close->setEnabled(false);
    ui->pushButton_open->setEnabled(true);
    ui->comboBox_Interface->setEnabled(true);
}

void MainWindow::receive()
{
    static QString str;
    int head_pos = 0;
        char ch;
        while (port.getChar(&ch))
        {
            str.append(ch);
            if (ch == '\n')     // End of line, start decoding
            {
                str.remove("\n", Qt::CaseSensitive);
                ui->textEdit_Status->append(str);


                // Insert your code here - *Use parts of previous exercises if needed
                if (str.contains("Temp")){
                    linkcheck[6] = 1;
                    lineTempTo0.setLine(xl[6], yl[6], xl[8], yl[8]);
                    QStringList temp_list = str.split(QRegExp("\\s"));
                    qDebug() << "Str value: " << str;

                    if(!temp_list.isEmpty()){
                        qDebug() << "List size " << temp_list.size();
                        for (int i=0; i < temp_list.size(); i++){
                            qDebug() << "List value "<< i <<" "<< temp_list.at(i);
                            if (temp_list.at(i) == "TempHumi:") {
                                int temp = temp_list.at(i+1).toInt();
                                int humi = temp_list.at(i+2).toInt();
                                ui->Temp -> setText(QString::number(temp));
                                ui->Humi -> setText(QString::number(humi));

                                if ((temp > 26)||(temp < 15)){
                                    QPalette pal = ui->Temp->palette();
                                    pal.setColor(QPalette::Button, QColor(Qt::red));
                                    ui->Temp->setAutoFillBackground(true);
                                    ui->Temp->setPalette(pal);
                                    ui->Temp->update();
                                }
                                else
                                {
                                    QPalette pal = ui->Temp->palette();
                                    pal.setColor(QPalette::Button, QColor(Qt::green));
                                    ui->Temp->setAutoFillBackground(true);
                                    ui->Temp->setPalette(pal);
                                    ui->Temp->update();
                                }

                                if ((humi < 30)||(humi > 70)){
                                    QPalette pal = ui->Humi->palette();
                                    pal.setColor(QPalette::Button, QColor(Qt::red));
                                    ui->Humi->setAutoFillBackground(true);
                                    ui->Humi->setPalette(pal);
                                    ui->Humi->update();
                                }
                                else
                                {
                                    QPalette pal = ui->Humi->palette();
                                    pal.setColor(QPalette::Button, QColor(Qt::green));
                                    ui->Humi->setAutoFillBackground(true);
                                    ui->Humi->setPalette(pal);
                                    ui->Humi->update();
                                }

                            }

                }
         }
                    this->repaint();    // Update paths between gateway and external sensor node
    }
                if (str.contains("Windspeed")){
                    linkcheck[7] = 1;
                    lineWindTo0.setLine(xl[6], yl[6], xl[7], yl[7]);
                    QStringList wind_list = str.split(QRegExp("\\s"));
                    qDebug() << "Str value: " << str;

                    if(!wind_list.isEmpty()){
                        qDebug() << "List size " << wind_list.size();
                        for (int i=0; i < wind_list.size(); i++){
                            qDebug() << "List value "<< i <<" "<< wind_list.at(i);
                            if (wind_list.at(i) == "Windspeed:") {
                                int wind = wind_list.at(i+1).toInt();
                                ui->Wind -> setText(QString::number(wind));

                                if (wind > 1){
                                    QPalette pal = ui->Wind->palette();
                                    pal.setColor(QPalette::Button, QColor(Qt::red));
                                    ui->Wind->setAutoFillBackground(true);
                                    ui->Wind->setPalette(pal);
                                    ui->Wind->update();
                                }
                                else
                                {
                                    QPalette pal = ui->Wind->palette();
                                    pal.setColor(QPalette::Button, QColor(Qt::green));
                                    ui->Wind->setAutoFillBackground(true);
                                    ui->Wind->setPalette(pal);
                                    ui->Wind->update();
                                }
                            }

                }
         }
             this->repaint();    // Update paths between gateway and external sensor node
    }


                if (str.contains("ClusterHead:"))
                {   line10.setLine(0,0,0,0);
                    line20.setLine(0,0,0,0);
                    line30.setLine(0,0,0,0);
                    line40.setLine(0,0,0,0);
                    line50.setLine(0,0,0,0);
                    line60.setLine(0,0,0,0);

                    lineTempTo0.setLine(0,0,0,0);
                    lineWindTo0.setLine(0,0,0,0);

                    line12.setLine(0,0,0,0);
                    line13.setLine(0,0,0,0);
                    line14.setLine(0,0,0,0);
                    line15.setLine(0,0,0,0);
                    line16.setLine(0,0,0,0);
                    line23.setLine(0,0,0,0);
                    line24.setLine(0,0,0,0);
                    line25.setLine(0,0,0,0);
                    line26.setLine(0,0,0,0);
                    line34.setLine(0,0,0,0);
                    line35.setLine(0,0,0,0);
                    line36.setLine(0,0,0,0);
                    line45.setLine(0,0,0,0);
                    line46.setLine(0,0,0,0);
                    line56.setLine(0,0,0,0);

                    this->repaint();    // Erase the previous path

                    QStringList list = str.split(QRegExp("\\s"));

                    qDebug() << "Str value: " << str;

                    if(!list.isEmpty()){
                        qDebug() << "List size " << list.size();
                        for (int i=0; i < list.size(); i++){
                            qDebug() << "List value "<< i <<" "<< list.at(i);
                            if (list.at(i) == "ClusterHead:") {
                                switch (list.at(i+1).toInt())
                                {
                                case 1:
                                    line10.setLine(xl[0], yl[0], xl[6], yl[6]);
                                    head_pos = 1;
                                    break;
                                case 2:
                                    line10.setLine(xl[1], yl[1], xl[6], yl[6]);
                                    head_pos = 2;
                                    break;
                                case 3:
                                    line10.setLine(xl[2], yl[2], xl[6], yl[6]);
                                    head_pos = 3;
                                    break;
                                case 4:
                                    line10.setLine(xl[3], yl[3], xl[6], yl[6]);
                                    head_pos = 4;
                                    break;
                                case 5:
                                    line10.setLine(xl[4], yl[4], xl[6], yl[6]);
                                    head_pos = 5;
                                    break;
                                case 6:
                                    line10.setLine(xl[5], yl[5], xl[6], yl[6]);
                                    head_pos = 6;
                                    break;
                                default:
                                    break;
                                }


                                }


                            if (list.at(i) == "Node:") {
                                for (int j=0; j<6; j++){
                                    if (list.at(i+j+1).toInt()==1){
                                        linkcheck[j] = 1;
                                    }
                                }

                                switch (head_pos)
                                {
                                case 1:
                                        if(list.at(i+2).toInt()==1){
                                            line12.setLine(xl[0],yl[0],xl[1],yl[1]);
                                    }
                                        else{
                                            line12.setLine(0,0,0,0);
                                        }
                                        if(list.at(i+3).toInt()==1){
                                            line13.setLine(xl[0],yl[0],xl[2],yl[2]);
                                    }
                                        else{
                                            line13.setLine(0,0,0,0);
                                        }
                                        if(list.at(i+4).toInt()==1){
                                            line14.setLine(xl[0],yl[0],xl[3],yl[3]);
                                    }
                                        else{
                                            line14.setLine(0,0,0,0);
                                        }
                                        if(list.at(i+5).toInt()==1){
                                            line15.setLine(xl[0],yl[0],xl[4],yl[4]);
                                    }
                                        else{
                                            line15.setLine(0,0,0,0);
                                        }
                                        if(list.at(i+6).toInt()==1){
                                            line16.setLine(xl[0],yl[0],xl[5],yl[5]);
                                    }
                                        else{
                                            line16.setLine(0,0,0,0);
                                        }
                                    break;

                                case 2:
                                        if(list.at(i+1).toInt()==1){
                                            line12.setLine(xl[0],yl[0],xl[1],yl[1]);
                                    }
                                        else{
                                            line12.setLine(0,0,0,0);
                                        }
                                        if(list.at(i+3).toInt()==1){
                                            line23.setLine(xl[1],yl[1],xl[2],yl[2]);
                                    }
                                        else{
                                            line23.setLine(0,0,0,0);
                                        }
                                        if(list.at(i+4).toInt()==1){
                                            line24.setLine(xl[1],yl[1],xl[3],yl[3]);
                                    }
                                        else{
                                            line24.setLine(0,0,0,0);
                                        }
                                        if(list.at(i+5).toInt()==1){
                                            line25.setLine(xl[1],yl[1],xl[4],yl[4]);
                                    }
                                        else{
                                            line25.setLine(0,0,0,0);
                                        }
                                        if(list.at(i+6).toInt()==1){
                                            line26.setLine(xl[1],yl[1],xl[5],yl[5]);
                                    }
                                        else{
                                            line26.setLine(0,0,0,0);
                                        }
                                    break;

                                case 3:
                                        if(list.at(i+1).toInt()==1){
                                            line13.setLine(xl[0],yl[0],xl[2],yl[2]);
                                    }
                                        else{
                                            line13.setLine(0,0,0,0);
                                        }
                                        if(list.at(i+2).toInt()==1){
                                            line23.setLine(xl[1],yl[1],xl[2],yl[2]);
                                    }
                                        else{
                                            line23.setLine(0,0,0,0);
                                        }
                                        if(list.at(i+4).toInt()==1){
                                            line34.setLine(xl[2],yl[2],xl[3],yl[3]);
                                    }
                                        else{
                                            line34.setLine(0,0,0,0);
                                        }
                                        if(list.at(i+5).toInt()==1){
                                            line35.setLine(xl[2],yl[2],xl[4],yl[4]);
                                    }
                                        else{
                                            line35.setLine(0,0,0,0);
                                        }
                                        if(list.at(i+6).toInt()==1){
                                            line36.setLine(xl[2],yl[2],xl[5],yl[5]);
                                    }
                                        else{
                                            line36.setLine(0,0,0,0);
                                        }
                                    break;

                                case 4:
                                        if(list.at(i+1).toInt()==1){
                                            line14.setLine(xl[0],yl[0],xl[3],yl[3]);
                                    }
                                        else{
                                            line14.setLine(0,0,0,0);
                                        }
                                        if(list.at(i+2).toInt()==1){
                                            line24.setLine(xl[1],yl[1],xl[3],yl[3]);
                                    }
                                        else{
                                            line24.setLine(0,0,0,0);
                                        }
                                        if(list.at(i+3).toInt()==1){
                                            line34.setLine(xl[2],yl[2],xl[3],yl[3]);
                                    }
                                        else{
                                            line34.setLine(0,0,0,0);
                                        }
                                        if(list.at(i+5).toInt()==1){
                                            line45.setLine(xl[3],yl[3],xl[4],yl[4]);
                                    }
                                        else{
                                            line45.setLine(0,0,0,0);
                                        }
                                        if(list.at(i+6).toInt()==1){
                                            line46.setLine(xl[3],yl[3],xl[5],yl[5]);
                                    }
                                        else{
                                            line46.setLine(0,0,0,0);
                                        }
                                    break;

                                case 5:
                                        if(list.at(i+1).toInt()==1){
                                            line15.setLine(xl[0],yl[0],xl[4],yl[4]);
                                    }
                                        else{
                                            line15.setLine(0,0,0,0);
                                        }
                                        if(list.at(i+2).toInt()==1){
                                            line25.setLine(xl[1],yl[1],xl[4],yl[4]);
                                    }
                                        else{
                                            line25.setLine(0,0,0,0);
                                        }
                                        if(list.at(i+3).toInt()==1){
                                            line35.setLine(xl[2],yl[2],xl[4],yl[4]);
                                    }
                                        else{
                                            line35.setLine(0,0,0,0);
                                        }
                                        if(list.at(i+4).toInt()==1){
                                            line45.setLine(xl[3],yl[3],xl[4],yl[4]);
                                    }
                                        else{
                                            line45.setLine(0,0,0,0);
                                        }
                                        if(list.at(i+6).toInt()==1){
                                            line56.setLine(xl[4],yl[4],xl[5],yl[5]);
                                    }
                                        else{
                                            line56.setLine(0,0,0,0);
                                        }
                                    break;

                                case 6:
                                        if(list.at(i+1).toInt()==1){
                                            line16.setLine(xl[0],yl[0],xl[5],yl[5]);
                                    }
                                        else{
                                            line16.setLine(0,0,0,0);
                                        }
                                        if(list.at(i+2).toInt()==1){
                                            line26.setLine(xl[1],yl[1],xl[5],yl[5]);
                                    }
                                        else{
                                            line26.setLine(0,0,0,0);
                                        }
                                        if(list.at(i+3).toInt()==1){
                                            line36.setLine(xl[2],yl[2],xl[5],yl[5]);
                                    }
                                        else{
                                            line36.setLine(0,0,0,0);
                                        }
                                        if(list.at(i+4).toInt()==1){
                                            line46.setLine(xl[3],yl[3],xl[5],yl[5]);
                                    }
                                        else{
                                            line46.setLine(0,0,0,0);
                                        }
                                        if(list.at(i+5).toInt()==1){
                                            line56.setLine(xl[4],yl[4],xl[5],yl[5]);
                                    }
                                        else{
                                            line56.setLine(0,0,0,0);
                                        }

                                    default:
                                            break;
                                }

                            }

                            if (list.at(i) == "Status:") {

                                for (int j=0; j<6; j++){
                                    if (list.at(i+j+1).toInt()==3){
                                        if (j==0){
                                            ui->Node_1 -> setText("1!!!");
                                            Timer1->start(10000);
                                        }
                                        else if (j==1){
                                            ui->Node_2 -> setText("2!!!");
                                            Timer2->start(10000);
                                        }
                                        else if (j==2){
                                            ui->Node_3 -> setText("3!!!");
                                            Timer3->start(10000);
                                        }
                                        else if (j==3){
                                            ui->Node_4 -> setText("4!!!");
                                            Timer4->start(10000);
                                        }
                                        else if (j==4){
                                            ui->Node_5 -> setText("5!!!");
                                            Timer5->start(10000);
                                        }
                                        else if (j==5){
                                            ui->Node_6 -> setText("6!!!");
                                            Timer6->start(10000);
                                        }
                                    }
                                    else{
                                        if (list.at(i+j+1).toInt()!=9){
                                           status[j] = list.at(i+j+1).toInt();
                                        }
                                }
                              }
                            }

                          }


                    }

                }

                this->repaint();    // Update content of window immediately
                str.clear();
            }
        }
}

void MainWindow::paintEvent(QPaintEvent *e){
    QPainter painter(this);

    int coordinateX = 370; int coordinateY = 110;
    int rectwidth = 550; int rectheight = 450;
    painter.eraseRect(coordinateX, coordinateY, rectwidth, rectheight);   // Clean the designated painting area

    QRect rec(coordinateX, coordinateY, rectwidth, rectheight);
    QPen framepen(Qt::red);
    framepen.setWidth(4);
    painter.drawRect(rec);                 // Draw the designated painting area

    // Insert your code here
    painter.setBrush(Qt::white);
    painter.drawEllipse(x[0], y[0], 50, 50); //x,y,w,h  node1
    painter.drawEllipse(x[1], y[1], 50, 50); //node2
    painter.drawEllipse(x[2], y[2], 50, 50);  //node3
    painter.drawEllipse(x[3], y[3], 50, 50); //node4
    painter.drawEllipse(x[4], y[4], 50, 50); //node5
    painter.drawEllipse(x[5], y[5], 50, 50); //node6

    for (int j=0; j<6; j++){
        if (status[j] == 0){
            painter.setBrush(Qt::green);
            painter.drawEllipse(x[j], y[j], 50, 50); //healthy status
        }
        else if (status[j] == 1){
            painter.setBrush(Qt::yellow);
            painter.drawEllipse(x[j], y[j], 50, 50); //warning status
        }
        else if (status[j] == 2) {
            painter.setBrush(Qt::black);
            painter.drawEllipse(x[j], y[j], 50, 50); //disconnected or default
        }

        if (linkcheck[j] == 0){
            painter.setBrush(Qt::white);
            painter.drawEllipse(x[j], y[j], 50, 50); //node timeout
        }

    }

    if (linkcheck[6] == 0){
        painter.setBrush(Qt::white);
        painter.drawEllipse(x[7], y[7], 50, 50); //node timeout
    }

    if (linkcheck[7] == 0){
        painter.setBrush(Qt::white);
        painter.drawEllipse(x[8], y[8], 50, 50); //node timeout
    }



    painter.setBrush(Qt::blue);
    painter.drawEllipse(x[6], y[6], 50, 50); //node0

    painter.setBrush(Qt::red);
    painter.drawEllipse(x[7], y[7], 50, 50); //wind
    painter.drawEllipse(x[8], y[8], 50, 50); //temperature

    painter.setPen(QPen(Qt::black, 3));
    painter.drawLine(line10);
    painter.drawLine(line20);
    painter.drawLine(line30);
    painter.drawLine(line40);
    painter.drawLine(line50);
    painter.drawLine(line60);
    painter.drawLine(line12);
    painter.drawLine(line13);
    painter.drawLine(line14);
    painter.drawLine(line15);
    painter.drawLine(line16);
    painter.drawLine(line23);
    painter.drawLine(line24);
    painter.drawLine(line25);
    painter.drawLine(line26);
    painter.drawLine(line34);
    painter.drawLine(line35);
    painter.drawLine(line36);
    painter.drawLine(line45);
    painter.drawLine(line46);
    painter.drawLine(line56);
    painter.drawLine(lineTempTo0);
    painter.drawLine(lineWindTo0);

}



void MainWindow::timer_timeout()
{
    //timer up
    for (int i=0; i<8; i++){
       linkcheck[i] = 0;
    }
    qDebug() << "Failed node checked!";
}



void MainWindow::alarm_clear1()
{
    //timer up
    ui->Node_1 -> setText("1");

}

void MainWindow::alarm_clear2()
{
    //timer up
    ui->Node_2 -> setText("2");

}

void MainWindow::alarm_clear3()
{
    //timer up
    ui->Node_3 -> setText("3");

}

void MainWindow::alarm_clear4()
{
    //timer up
    ui->Node_4 -> setText("4");

}

void MainWindow::alarm_clear5()
{
    //timer up
    ui->Node_5 -> setText("5");

}

void MainWindow::alarm_clear6()
{
    //timer up
    ui->Node_6 -> setText("6");

}
