#include "monitor.h"

Monitor::Monitor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Monitor)
{
    ui->setupUi(this);


    this->initForm();

}

Monitor::~Monitor()
{
    delete ui;
}

bool Monitor::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick) {
        if (watched == ui->widgetTitle) {
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}
//===========================网络部分=========================/
QString Monitor::Ipupdate() //ok
{
    QString ip;
    QList<QHostAddress> addrs = QNetworkInterface::allAddresses();

    foreach (QHostAddress addr, addrs) {
        ip = addr.toString();
          if((addr.protocol() == QAbstractSocket::IPv4Protocol)&&(addr != QHostAddress(QHostAddress::LocalHost)))
        {
            break;
        }
    }
    return ip;

}
void Monitor::sendDataTcpClient(QByteArray data)//ok
{
    if (!tcpClient->isWritable()) {
        return;
   }

    if (data.isEmpty()) {
        return;
    }
    tcpClient->write(data,data.size ());
   // tcpClient->waitForBytesWritten (300);
    tcpClient->flush ();


}
void Monitor::sendDataUdpServer()//ok
{

    QString IP =Ipupdate ();
    QString IP1=QString::number(IP.section('.', 0,0).trimmed().toInt (),16).toUpper ();
    // int IP1_INT=IP1.toInt ();
    // QString IP1_HEX=QString::number(IP1_INT, 16).toUpper();
    QString IP2=QString::number(IP.section('.', 1,1).trimmed().toInt (),16).toUpper ();
    //QString IP3=QString::number(IP.section('.', 2,2).trimmed().toInt (),16).toUpper ();
    QString IP3=QString("%1").arg(IP.section('.', 2,2).trimmed().toInt (), 2, 16, QLatin1Char('0'));
    QString IP4=QString::number(IP.section('.', 3,3).trimmed().toInt (),16).toUpper ();
    QString COM=QString("%1").arg(3486, 4, 16, QLatin1Char('0'));
    QString data = IP1+IP2+IP3+IP4+COM+"07"+"01"+"01"+"01";
    sendDataUdpServer(data);
}

void Monitor::sendDataUdpServer(QString data)//ok
{
    QString ip ="192.168.1.255";//Ipupdate ();
    int port = 3486;//ui->txtUdpClientPort->text().toInt();
    sendDataUdpServer(ip, port, data);
}

void Monitor::sendDataUdpServer(QString ip, int port, QString data)//ok
{
    if (!data.isEmpty()) {
        QByteArray buffer;

        //if (App::HexSendUdpServer) {
            buffer = myHelper::hexStrToByteArray(data);
        //} else {
       //     buffer = myHelper::asciiStrToByteArray(data);
        //}

        udpServer->writeDatagram(buffer, QHostAddress(ip), port);
    }
}

void Monitor::tcpClientReadData()//ok
{
    if (tcpClient->bytesAvailable() <= 0) {
        return;
    }

    QByteArray data = tcpClient->readAll();
    //QString buffer;

    //if (App::HexReceiveTcpClient) {
    //    buffer = myHelper::byteArrayToHexStr(data);
    //} else {
    //    buffer = myHelper::byteArrayToAsciiStr(data);
    //}

     //   qDebug (qPrintable(buffer));
    if(data.at(0)==0x07&&data.at(1)==0x01)
    {
    if(data.at(10)>=0x11&&data.at(10)<=0x21)
    {
     TcponlyReply (data.at(9),data.at(10),data.at(11));
     TcpConfirm (data.at(9),data.at(10));
    }
    else if(data.at(10)==0x10)
    {
    TcpallReply (data.at(9));
    }
    else
    {return;}
    }


}

void Monitor::tcpClientReadError()//ok
{
    tcpClient->disconnectFromHost();
    qDebug (qPrintable(QString("连接服务器失败,原因 : %1").arg(tcpClient->errorString())));
    ui->label_network->setText ("服务器状态：未连接");
    TcpListenState=false;
}

void Monitor::udpServerReadData() //ok
{
    QHostAddress senderServerIP;
    quint16 senderServerPort;
    QByteArray data;
    QString buffer;

    do {
        data.resize(udpServer->pendingDatagramSize());
        udpServer->readDatagram(data.data(), data.size(), &senderServerIP, &senderServerPort);

      //  if (App::HexReceiveUdpServer) {
            buffer = myHelper::byteArrayToHexStr(data);
     //   } else {
      //      buffer = myHelper::byteArrayToAsciiStr(data);
     //   }

        QString str = QString("来自:%1[%2] ").arg(senderServerIP.toString()).arg(senderServerPort);
        //qDebug (qPrintable(str + buffer));
        if(data.at(6)==0x01&&data.at(7)==0x00&&data.at(8)==0x01)
        {
            TcpserverIp=senderServerIP.toString();
            TcpserverPort=8080;

            if(TcpListenState==false)
            {
            TcpListenState=btnTcpConnect_set(true);
            }
            Udpservertick=true;
        }
    } while (udpServer->hasPendingDatagrams());
}

void Monitor::udpServerReadError()//ok
{
qDebug (qPrintable(QString("发生错误,原因 : %1").arg(udpServer->errorString())));
ui->label_network->setText ("服务器状态：未连接");
UdpListenState=false;
}

bool Monitor::btnTcpConnect_set(bool setflag) //ok
{
    if (setflag) {
        tcpClient->connectToHost(TcpserverIp, TcpserverPort);
        if (tcpClient->waitForConnected(1000)) {
            qDebug ("tcp连接成功");
            return true;
        } else {
            qDebug ("tcp连接服务器失败");
            return false;
        }
    } else {
        tcpClient->disconnectFromHost();

        if (tcpClient->state() == QAbstractSocket::UnconnectedState || tcpClient->waitForDisconnected(1000)) {
            qDebug ("tcp断开连接");
            return true;
        }
    }
}

bool Monitor::btnUdpListen_set(bool setflag) //ok
{
    if (setflag) {
        bool ok = udpServer->bind(3486);

        if (ok) {
           // qDebug ("udp监听成功");
            sendDataUdpServer ();
            return true;
        } else {
            qDebug ("udp监听失败,请检查端口是否被占用");
            return false;
        }
    } else {
        udpServer->abort();
        qDebug ("UDP停止监听");
            return true;
    }
}

void Monitor::timer_udp()
{

    if((access("/dev/input/event2",F_OK))!=-1)
    {
        if(usbtouch_flag==1)
        {
            massage->close();
            usbtouch_flag=0;
        }
    }
    else
    {

        QIcon icon;
        icon.addFile (":/image/btncheckoff.png");
        ui->btnUseResponse->setIcon (icon);
        globalData::lcdsetswitch=0;
        emit signal_massagebox (0,"触摸设备异常，请检查线路或重启！");
        sqlmanager->SqlAddSystemInfo("触摸设备异常",DATETIME,globalData::username);

        usbtouch_flag=1;

    }

    if(!outage_flag)
    {
        duandian();
    }
    if(UdpListenState) //UDP服务判断
    {
        sendDataUdpServer();
    }
    else
    {

        UdpListenState=btnUdpListen_set(true);
        return;
    }


    if(Udpservertick)//是否接收到心跳包
    {
        if(TcpListenState)  //TCP服务判断
        {
         ui->label_network->setText ("服务器状态：正常");
         Udpservertick=false;
        }
        else
        {
            TcpListenState=btnTcpConnect_set(true);
            ui->label_network->setText ("服务器状态：未连接");
            Udpservertick=false;
            return;
        }

    }
    else
    {
        Udpservererror++;
        if(Udpservererror>=3)
        {

           Udpservererror=0;
           if(TcpListenState)
           {
               btnTcpConnect_set(false);
               TcpListenState=false;
           }
           ui->label_network->setText ("服务器状态：未连接");
           return;
        }

    }

}

void Monitor::TcpConfirm(quint8 DB_Ad_H,quint8  DB_Ad_L) //确认消息
{
    QByteArray news="";
    if(!TcpListenState)
    {return;}
    news.append ((char)0X01);
    news.append ((char)0X00);
    news.append ((char)0X07);
    news.append ((char)0X01);
    news.append ((char)0X00);
    news.append ((char)0xE0);
    news.append ((char)0x00);
    news.append ((char)0x04);
    news.append ((char)0x02);
    news.append ((char)DB_Ad_H);
    news.append ((char)DB_Ad_L);
    news.append ((char)0x00);
    sendDataTcpClient(news);
}

void Monitor::TcponlyReply(quint8 DB_Ad_H,quint8  DB_Ad_L,quint8 DataID) //单探杆单数据应答消息
{
    QByteArray news="";
    unsigned char temp;
    quint8 addr;
    if(!TcpListenState)
    {return;}
    switch(DB_Ad_H)
    {
    case 0x02: //人井
    addr=DB_Ad_L-0x11+40;
    break;
    case 0x03://管线
    addr=DB_Ad_L-0x11+20;
    break;
    case 0x04://油罐
    addr=DB_Ad_L-0x11;
    break;
    case 0x05://油盆
    addr=DB_Ad_L-0x11+60;
    break;
    default:
        return;
    break;
    }
    news.append ((char)0X01);
    news.append ((char)0X00);
    news.append ((char)0X07);
    news.append ((char)0X01);
    news.append ((char)0X00);
    news.append ((char)0x20);
    news.append ((char)0x00);
    switch(DataID)
    {
        case 0x02:
        news.append ((char)0x07);
        news.append ((char)0x02);
        news.append ((char)DB_Ad_H);
        news.append ((char)DB_Ad_L);

        news.append ((char)0x02);
        news.append ((char)0x02);

        switch (globalData::net_buf[addr]) {


        case 0://正常
             news.append ((char)0x00);
             news.append ((char)0x00);
                break;
        case 1://水
             news.append ((char)0x00);
             news.append ((char)0x10);
                break;
        case 2://油
            news.append ((char)0x00);
            news.append ((char)0x08);
                break;
        case 3://高液位
            news.append ((char)0x00);
            news.append ((char)0x20);
                break;
        case 4://低液位
            news.append ((char)0x00);
            news.append ((char)0x40);
                break;
        case 5://渗漏
            news.append ((char)0x00);
            news.append ((char)0x80);
                break;
        case 6://压力预警
            news.append ((char)0x01);
            news.append ((char)0x00);
                break;
        case 7://压力报警
            news.append ((char)0x02);
            news.append ((char)0x00);
                break;
        case 8://油气预警
            news.append ((char)0x04);
            news.append ((char)0x00);
                break;
        case 9://油气报警
            news.append ((char)0x08);
            news.append ((char)0x00);
                break;
        case 10://通讯故障
            news.append ((char)0x00);
            news.append ((char)0x01);
                break;
        case 0xff://未开通
            news.append ((char)0x00);
            news.append ((char)0x02);
                break;

        default:
            news.append ((char)0x00);
            news.append ((char)0x00);
                break;


        }
            break;
        case 0x03:
        news.append ((char)0x06);
        news.append ((char)0x02);
        news.append ((char)DB_Ad_H);
        news.append ((char)DB_Ad_L);

        news.append ((char)0x03);
        news.append ((char)0x01);
        news.append ((char)0x04);
        break;
    case 0x04:
        news.append ((char)0x06);
        news.append ((char)0x02);
        news.append ((char)DB_Ad_H);
        news.append ((char)DB_Ad_L);
        news.append ((char)0x04);
        news.append ((char)0x01);
        news.append ((char)DB_Ad_L-0x10);
        break;
    case 0x05:
        news.append ((char)0x09);
        news.append ((char)0x02);
        news.append ((char)DB_Ad_H);
        news.append ((char)DB_Ad_L);
        news.append ((char)0x05);
        news.append ((char)0x04);
        news.append ((char)0x01);
        news.append ((char)0x00);
        temp=(unsigned char)(globalData::pressure_buf[addr]);
        temp=(temp/10*16 + temp%10);
        news.append ((char)temp);
        temp= (unsigned char)((int)(globalData::pressure_buf[addr]*10)%10);
        temp=temp<<4;
        news.append ((char)temp);
        break;
    case 0x06:
        news.append ((char)0x07);
        news.append ((char)0x02);
        news.append ((char)DB_Ad_H);
        news.append ((char)DB_Ad_L);
        news.append ((char)0x06);
        news.append ((char)0x02);
        news.append ((char)0x00);
        news.append ((char)0x00);
        break;
    case 0x10:
        news.append ((char)0x06);
        news.append ((char)0x02);
        news.append ((char)DB_Ad_H);
        news.append ((char)DB_Ad_L);
        news.append ((char)0x10);
        news.append ((char)0x01);
        news.append ((char)0xaa);
    break;
    case 0x11:
        news.append ((char)0x06);
        news.append ((char)0x02);
        news.append ((char)DB_Ad_H);
        news.append ((char)DB_Ad_L);

        news.append ((char)0x11);
        news.append ((char)0x01);
        news.append ((char)0xff);
    break;
    case 0x12:
        news.append ((char)0x06);
        news.append ((char)0x02);
        news.append ((char)DB_Ad_H);
        news.append ((char)DB_Ad_L);

        news.append ((char)0x12);
        news.append ((char)0x01);
        news.append ((char)0x55);
    break;
    case 0x13:
        news.append ((char)0x06);
        news.append ((char)0x02);
        news.append ((char)DB_Ad_H);
        news.append ((char)DB_Ad_L);

        news.append ((char)0x13);
        news.append ((char)0x01);
        news.append ((char)0x11);
    break;
    case 0x14:
        news.append ((char)0x06);
        news.append ((char)0x02);
        news.append ((char)DB_Ad_H);
        news.append ((char)DB_Ad_L);

        news.append ((char)0x14);
        news.append ((char)0x01);
        news.append ((char)0xff);
    break;
        default:
            break;
        }

    sendDataTcpClient(news);

}
void Monitor::TcpallReply(quint8 DB_Ad_H)//单探杆全部数据应答消息
{
    QByteArray news="";
    quint8 addr;
    quint8  DB_Ad_L;
    quint8 i;
    if(!TcpListenState)
    {return;}

    if(DB_Ad_H==0x02)
    {
     i=0x21;
    }
    else
    {
     i=0x19;
    }

    for(DB_Ad_L=0x11;DB_Ad_L<i;DB_Ad_L++)
    {

    switch(DB_Ad_H)
    {
    case 0x02: //人井
    addr=DB_Ad_L-0x11+40;
    break;
    case 0x03://管线
    addr=DB_Ad_L-0x11+20;
    break;
    case 0x04://油罐
    addr=DB_Ad_L-0x11;
    break;
    case 0x05://油盆
    addr=DB_Ad_L-0x11+60;
    break;
    default:
        return;
    break;
    }
    news.append ((char)0X01);
    news.append ((char)0X00);
    news.append ((char)0X07);
    news.append ((char)0X01);
    news.append ((char)0X00);//应用消息
    news.append ((char)0x20);//令牌

    news.append ((char)0x00);//消息长度H
    if(DB_Ad_H==0x03)
    {
    news.append ((char)0x10);//消息长度L
    }
    else
    {
    news.append ((char)0x19);//消息长度L
    }
    //数据库地址/////////////////////////
    news.append ((char)0x02);
    news.append ((char)DB_Ad_H);
    news.append ((char)DB_Ad_L);
    //02H数据//////////////////////////
    news.append ((char)0x02);
    news.append ((char)0x02);
    switch (globalData::net_buf[addr]) {


    case 0://正常
        news.append ((char)0x00);
        news.append ((char)0x00);
        break;
    case 1://水
        news.append ((char)0x00);
        news.append ((char)0x10);
        break;
    case 2://油
        news.append ((char)0x00);
        news.append ((char)0x08);
        break;
    case 3://高液位
        news.append ((char)0x00);
        news.append ((char)0x20);
        break;
    case 4://低液位
        news.append ((char)0x00);
        news.append ((char)0x40);
        break;
    case 5://渗漏
        news.append ((char)0x00);
        news.append ((char)0x80);
        break;
    case 6://压力预警
        news.append ((char)0x01);
        news.append ((char)0x00);
        break;
    case 7://压力报警
        news.append ((char)0x02);
        news.append ((char)0x00);
        break;
    case 8://油气预警
        news.append ((char)0x04);
        news.append ((char)0x00);
        break;
    case 9://油气报警
        news.append ((char)0x08);
        news.append ((char)0x00);
        break;
    case 10://通讯故障
        news.append ((char)0x00);
        news.append ((char)0x01);
        break;
    case 0xff://未开通
        news.append ((char)0x00);
        news.append ((char)0x02);
        break;

    default:
        news.append ((char)0x00);
        news.append ((char)0x00);
        break;


    }
    //03H数据//////////////////////////
    news.append ((char)0x03);
    news.append ((char)0x01);
    news.append ((char)0x04);
    //04H数据//////////////////////////
    news.append ((char)0x04);
    news.append ((char)0x01);
    news.append ((char)DB_Ad_L-0x10); //13个
    if(DB_Ad_H==0x03)
    {
        //14H数据//////////////////////////
        news.append ((char)0x14);
        news.append ((char)0x01);
        news.append ((char)0xff);
    }
    else
    {
        //10H数据//////////////////////////
        news.append ((char)0x10);
        news.append ((char)0x01);
        news.append ((char)0xaa);
        //11H数据//////////////////////////
        news.append ((char)0x11);
        news.append ((char)0x01);
        news.append ((char)0xff);
        //12H数据//////////////////////////
        news.append ((char)0x12);
        news.append ((char)0x01);
        news.append ((char)0x55);
        //13H数据//////////////////////////
        news.append ((char)0x13);
        news.append ((char)0x01);
        news.append ((char)0x11);

    }
    sendDataTcpClient(news);
    news.clear ();
    usleep(200000);
    }

}
void Monitor::TcpActive(quint8 DB_Ad_H,quint8  DB_Ad_L,quint8 DataEl)  //无确认主动消息
{
   QByteArray news="";
   if(!TcpListenState)
   {return;}
   news.append ((char)0X01);
   news.append ((char)0X00);
   news.append ((char)0X07);
   news.append ((char)0X01);
   news.append ((char)0X00);
   news.append ((char)0x80);
   news.append ((char)0x00);
   news.append ((char)0x0D);
   news.append ((char)0x02);
   news.append ((char)DB_Ad_H);
   news.append ((char)DB_Ad_L);
   news.append ((char)0x02);
   news.append ((char)0x02);
   switch (DataEl) {

   case 0://正常
        news.append ((char)0x00);
        news.append ((char)0x00);
           break;
   case 1://水
        news.append ((char)0x00);
        news.append ((char)0x10);
           break;
   case 2://油
       news.append ((char)0x00);
       news.append ((char)0x08);
           break;
   case 3://高液位
       news.append ((char)0x00);
       news.append ((char)0x20);
           break;
   case 4://低液位
       news.append ((char)0x00);
       news.append ((char)0x40);
           break;
   case 5://渗漏
       news.append ((char)0x00);
       news.append ((char)0x80);
           break;
   case 6://压力预警
       news.append ((char)0x01);
       news.append ((char)0x00);
           break;
   case 7://压力报警
       news.append ((char)0x02);
       news.append ((char)0x00);
           break;
   case 8://油气预警
       news.append ((char)0x04);
       news.append ((char)0x00);
           break;
   case 9://油气报警
       news.append ((char)0x08);
       news.append ((char)0x00);
           break;
   case 10://通讯故障
       news.append ((char)0x00);
       news.append ((char)0x01);
           break;

   default:
       news.append ((char)0x00);
       news.append ((char)0x00);
           break;


   }
   news.append ((char)0x03);
   news.append ((char)0x01);
   news.append ((char)0x04);
   news.append ((char)0x04);
   news.append ((char)0x01);
   news.append ((char)DB_Ad_L-0x10);
   sendDataTcpClient(news);

}
