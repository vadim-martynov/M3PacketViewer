#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent, QStringList args) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    counter = 0;
    curId = 0;
    curMS = 0;
    playProcess = false;
    dontUpdateInerfacesFlag = false;
    for(quint8 i=0; i<NETS_COUNT; i++)
    {
        SCounters tmp;
        for(quint8 j=0; j<10; j++)
            tmp.types.append(0);
        tmp.all = 0;
        tmp.unit = 0;
        netPacketCounter.append(tmp);
        curPacketCounter.append(0);
    }

    qDebug()<<"Args: "<< args << args.count();
    //Nets manegement
    if(args.count() > 1)
    {
        for(quint8 i=1; i<args.count(); i++)
        {
            qDebug()<<"arg"<<i;
            if(i<3) hIP.append(QHostAddress(args[i]));
        }
    }
    else
    {
        hIP = netList();
    }

    foreach (QHostAddress entry, hIP)
    {
        hIpBroadcast.append(GetBroadcast(entry));
        qDebug()<<"Use net: "<< entry.toString() << hIpBroadcast.last().toString();
    }

    bool ret;

    //Init sockets
    for(int i=0; i<hIP.size(); i++)
    {
        socket.append(new QUdpSocket(this));

#ifdef WIN32
        ret = socket[i]->bind(hIP[i], PORT_CONTROL, QUdpSocket::ShareAddress);
        //socket[i]->setSocketOption(QAbstractSocket::MulticastTtlOption, 255);
#else
        //ret = socket[i]->bind(hIpBroadcast[i], PORT_CONTROL, QUdpSocket::ShareAddress);
        ret = socket[i]->bind(PORT_CONTROL, QUdpSocket::ShareAddress);
#endif
        qDebug()<<"Bind socket: "<< i<< ret;

#if QT_VERSION >= QT_VERSION_CHECK(4,8,0)
        QHostAddress groupAddress = QHostAddress(MC_TCMS);
        QNetworkInterface nIF = GetNetworkInterface(hIP[i]);

        ret = socket[i]->joinMulticastGroup(groupAddress, nIF);
        qDebug()<<"Join socket: "<< i<< groupAddress.toString() << nIF.name() << nIF.humanReadableName() << nIF.index() << ret;
#endif

        if(i==0)
            connect(socket[i],SIGNAL(readyRead()),this,SLOT(readReady0()));
        else
            connect(socket[i],SIGNAL(readyRead()),this,SLOT(readReady1()));
    }

    //pPacket = new CControlPacket2;

    QTimer* ptimer = new QTimer(this);
    connect(ptimer, SIGNAL(timeout()), SLOT(timerProcessing()));
    ptimer->start(TIMER_VIEW);

    QTimer* ptick = new QTimer(this);
    connect(ptick, SIGNAL(timeout()), SLOT(tickProcessing()));
    ptick->start(TIMER_TICK);

    //Init treewiev
    nodeTypes.append("WS");
    nodeTypes.append("Phone");
    nodeTypes.append("R-Man");
    nodeTypes.append("Radio");
    nodeTypes.append("Record");
    foreach (QString entry, nodeTypes)
    {
        sourceItems.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(entry)));
    }
     ui->twI->insertTopLevelItems(0, sourceItems);
     ui->twI->setColumnWidth(0,70);
     ui->twI->setColumnWidth(1,22);
     ui->twI->setColumnWidth(2,60);

     //Init packettype selector
     packetTypes.append("Phone (t1)");
     packetTypes.append("Radio (t2)");
     packetTypes.append("Terminal (t3)");
     packetTypes.append("Record (t0:1)");
     packetTypes.append("IntTermCommon (t0:2)");
     packetTypes.append("IntTermRadio (t0:3)");
     packetTypes.append("Permanent (t0:4)");
     packetTypes.append("TermExtensions (t0:5)");
     packetTypes.append("DivertGroups (t0:6)");

     foreach (QString entry, packetTypes)
     {
         rbPacketTypes.append(new QRadioButton(entry));
         ui->groupTypes->layout()->addWidget(rbPacketTypes.last());
         rbPacketTypes.last()->setEnabled(false);
         connect(rbPacketTypes.last(),SIGNAL(clicked()),this,SLOT(TypeSelected()));
     }

     //Init lan check
     netLabels.append(tr("LAN A"));
     netLabels.append(tr("LAN B"));
     foreach (QString entry, netLabels)
     {
         chNets.append(new QCheckBox(entry));
         ui->groupLan->layout()->addWidget(chNets.last());
         chNets.last()->setEnabled(false);
         connect(chNets.last(),SIGNAL(toggled(bool)),this,SLOT(NetChanged()));
     }

     //Init infotable
     ui->tblInfo->setRowCount(INFO_ROW); // указываем количество строк
     ui->tblInfo->setColumnCount(NETS_COUNT); // указываем количество столбцов
     ui->tblInfo->verticalHeader()->setFixedWidth(120);
     for(int row = 0; row < ui->tblInfo->rowCount(); row++)
         for(int col = 0; col < ui->tblInfo->columnCount(); col++)
         {
               ui->tblInfo->setItem(row, col, new QTableWidgetItem());
         }
     for(quint8 i=0; i<hIP.size(); i++)
     {
         SetCellTextInfo(0, i, hIP[i].toString());
     }

     //Init infoall table
     ui->tblInfoAll->setRowCount(2 + packetTypes.size()); // указываем количество строк
     ui->tblInfoAll->setColumnCount(NETS_COUNT); // указываем количество столбцов
     ui->tblInfoAll->verticalHeader()->setFixedWidth(120);
     for(int row = 2; row < ui->tblInfoAll->rowCount(); row++)
     {
         ui->tblInfoAll->setVerticalHeaderItem(row, new QTableWidgetItem());
         ui->tblInfoAll->verticalHeaderItem(row)->setText(packetTypes[row-2]);
     }
     for(int row = 0; row < ui->tblInfoAll->rowCount(); row++)
         for(int col = 0; col < ui->tblInfoAll->columnCount(); col++)
         {
               ui->tblInfoAll->setItem(row, col, new QTableWidgetItem());
         }

     //Init datatable
     ui->tblData->setColumnCount(DATA_COL);
     for(int col = 0; col < ui->tblData->columnCount(); col++)
     {
         ui->tblData->setHorizontalHeaderItem(col, new QTableWidgetItem());
         ui->tblData->horizontalHeaderItem(col)->setText(QString::number(col));
         ui->tblData->setColumnWidth(col,40);//
     }
     ui->tblData->setRowCount(DATA_ROW); // указываем количество строк
     for(int row = 0; row < ui->tblData->rowCount(); row++)
     {
         ui->tblData->setVerticalHeaderItem(row, new QTableWidgetItem());
         ui->tblData->verticalHeaderItem(row)->setText(QString::number(row));
         ui->tblData->setRowHeight(row, 20);//27
         for(int col = 0; col < ui->tblData->columnCount(); col++)
         {
             ui->tblData->setItem(row, col, new QTableWidgetItem());
         }
     }

    connect(ui->twI,SIGNAL(clicked(QModelIndex)),this,SLOT(IdSelected()));

// statusbar preset
    sbStatus = new QLabel(this);
    statusBar()->addWidget(sbStatus);

    sbRecordTime = new QLabel(this);
    statusBar()->addWidget(sbRecordTime);
    sbRecordTime->setVisible(false);

    sbPlayProcess = new QProgressBar(this);
    statusBar()->addWidget(sbPlayProcess);
    sbPlayProcess->resize(500,0);
    sbPlayProcess->setVisible(false);

    sbTickTime = new QLabel(this);
    statusBar()->addWidget(sbTickTime);
    sbTickTime->setVisible(false);

    sbPlayFile = new QLabel(this);
    statusBar()->addWidget(sbPlayFile);
    sbPlayFile->setVisible(false);

// toolbar preset
    ui->mainToolBar->setIconSize(QSize(16,16));

    ui->mainToolBar->addAction(ui->actionOpen);
    ui->mainToolBar->addAction(ui->actionDelete);
    ui->mainToolBar->addSeparator();

    runTrigger = ui->mainToolBar->addAction("");
    runStatus = false;
    RunButtonClicked();
    connect(runTrigger,SIGNAL(triggered()),this,SLOT(RunButtonClicked()));
    ui->menuCommands->addAction(runTrigger);
//    menuRun = ui->menuCommands->addMenu("Run");
//    menuRun->addAction(runTrigger);

    cleanTrigger = ui->mainToolBar->addAction("");
    cleanStatus = false;
    CleanButtonClicked();
    connect(cleanTrigger,SIGNAL(triggered()),this,SLOT(CleanButtonClicked()));
    ui->menuCommands->addAction(cleanTrigger);

    treeTrigger = ui->mainToolBar->addAction("");
    treeStatus = true;
    TreeButtonClicked();
    connect(treeTrigger,SIGNAL(triggered()),this,SLOT(TreeButtonClicked()));
    ui->menuCommands->addAction(treeTrigger);

    recordTrigger = ui->mainToolBar->addAction("");
    recordStatus = true;
    RecordButtonClicked();
    connect(recordTrigger,SIGNAL(triggered()),this,SLOT(RecordButtonClicked()));
    ui->menuCommands->addAction(recordTrigger);

    broadcastTrigger = ui->mainToolBar->addAction("");
    broadcastStatus = true;
    BroadcastButtonClicked();
    connect(broadcastTrigger,SIGNAL(triggered()),this,SLOT(BroadcastButtonClicked()));
    ui->menuCommands->addAction(broadcastTrigger);
    broadcastTrigger->setEnabled(false);

    QString text;
    text = tr("Close record");
    endplayTrigger = ui->mainToolBar->addAction("");
    endplayTrigger->setEnabled(false);
    connect(endplayTrigger,SIGNAL(triggered()),this,SLOT(EndPlayButtonClicked()));
    QPixmap iEndplay(":/new/img/Close.png");
    endplayTrigger->setIcon(QIcon(iEndplay));
    endplayTrigger->setToolTip(text);
    endplayTrigger->setText(text);
    ui->menuCommands->addAction(endplayTrigger);

    text = tr("Replay");
    replayTrigger = ui->mainToolBar->addAction("");
    replayTrigger->setEnabled(false);
    connect(replayTrigger,SIGNAL(triggered()),this,SLOT(RePlayButtonClicked()));
    QPixmap iReplay(":/new/img/Back.png");
    replayTrigger->setIcon(QIcon(iReplay));
    replayTrigger->setToolTip(text);
    replayTrigger->setText(text);
    ui->menuCommands->addAction(replayTrigger);

//    text = tr("PlayFilter");
//    playFilterTrigger = ui->mainToolBar->addAction("");
//    //playFilterTrigger->setEnabled(false);
//    connect(playFilterTrigger,SIGNAL(triggered()),this,SLOT(PlayFilterButtonClicked()));
//    QPixmap iPlayFilter(":/new/img/microscope.png");
//    playFilterTrigger->setIcon(QIcon(iPlayFilter));
//    playFilterTrigger->setToolTip(text);
//    playFilterTrigger->setText(text);
//    ui->menuCommands->addAction(playFilterTrigger);

//    //Dialog window init
//    playFilter = new DialogPlayFilter(this),


    // window preset
    this->setWindowTitle(TITLE_MAIN);



// menu preset    
    connect(ui->actionDelete,SIGNAL(triggered()),this,SLOT(MenuFileDelete()));
    connect(ui->actionOpen,SIGNAL(triggered()),this,SLOT(MenuFileOpen()));
    connect(ui->actionAbout_programm,SIGNAL(triggered()),this,SLOT(MenuAboutProgramm()));

}

MainWindow::~MainWindow()
{
// turn off active record
    if(recordStatus)    RecordButtonClicked();

#if QT_VERSION >= QT_VERSION_CHECK(4,8,0)
    QHostAddress groupAddress = QHostAddress(MC_TCMS);
    for(int i=0; i<hIP.size(); i++)
    {
        qDebug()<< socket[i]->leaveMulticastGroup(groupAddress, GetNetworkInterface(hIP[i]));
    }
#endif
    socket.clear();
    qDebug()<< "end!";
    delete ui;
}

// //////////////////////////////////////////////////////////////////
// other functions

void MainWindow::ResetSource()
{
    curId = 0;
    curMS = 0;
    curType = 0;
    curUltraType = 0;


    for(int j=0; j<DATA_ROW; j++)
    {
        ui->tblData->verticalHeaderItem(j)->setText(QString::number(j));
        for(int k=0; k<DATA_COL; k++)
        {
            SetCellTextData(j, k, "");
            ResetHeaderMarker(k, k);
            ui->tblData->item(j, k)->setBackgroundColor(Qt::white);
            ui->tblData->item(j, k)->setStatusTip("");
        }
    }

    for(int j=1; j<INFO_ROW; j++)
        for(int k=0; k<NETS_COUNT; k++)
        {
            SetCellTextInfo(j, k, "");
        }
    counter = 0;
    ResetCurrentPacketCounter();
}



QNetworkInterface MainWindow::GetNetworkInterface(QHostAddress host)
{
    QNetworkInterface workInterface;

    QList<QNetworkInterface> interfaceList = QNetworkInterface::allInterfaces();
    foreach(QNetworkInterface interface, interfaceList)
        {
            if(interface.flags().testFlag(QNetworkInterface::IsUp) &&
               (!interface.flags().testFlag(QNetworkInterface::IsLoopBack)))
            {
                foreach(QNetworkAddressEntry entry, interface.addressEntries())
                {
                   if(entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
                   {
                       if(host == entry.ip())
                       {
                           qDebug() << host.toString() << interface.humanReadableName();
                           workInterface = interface;
                       }
                   }
                }
            }
        }
    return workInterface;
}

QHostAddress MainWindow::GetBroadcast(const QHostAddress &adr)
{
    QList<QNetworkInterface> interfaceList = QNetworkInterface::allInterfaces();
    foreach(QNetworkInterface interface, interfaceList)
        {
            if(interface.flags().testFlag(QNetworkInterface::IsUp) &&
               (!interface.flags().testFlag(QNetworkInterface::IsLoopBack)))
            {
                foreach(QNetworkAddressEntry entry, interface.addressEntries())
                {
                   if(entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
                   {
                       if(entry.ip() == adr)
                       {
                           return entry.broadcast();
                       }
                   }
                }
            }
        }
    return QHostAddress::Broadcast;
}

QVector<QHostAddress> MainWindow::netList()
{
    QMap<QString, QHostAddress> ip;
    QList<QNetworkInterface> interfaceList = QNetworkInterface::allInterfaces();
    //qDebug() << "net list" << interfaceList;
    foreach(QNetworkInterface interface, interfaceList)
        {
            if(interface.flags().testFlag(QNetworkInterface::IsUp) &&
               (!interface.flags().testFlag(QNetworkInterface::IsLoopBack)))
            {
                foreach(QNetworkAddressEntry entry, interface.addressEntries())
                {
                   if(entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
                   {
                       if(entry.ip().toString().left(3)=="172")
                            ip.insert(entry.ip().toString(), entry.ip());
                   }
                }
            }
        }
    QVector<QHostAddress> list;
    QMap<QString, QHostAddress>::const_iterator i;
    int j = 1;
    for (i = ip.constBegin(); i != ip.constEnd(); ++i)
    {
        if(j++ > ip.count()-2)
        {
//            qDebug() << i.key() << ":" << i.value() << ip.count();
            list.append(i.value());
        }
    }
    return list;
}


QTreeWidgetItem* MainWindow::AddItem(const QString &name)
{
  QStringList lst;
  lst << name;
  QTreeWidgetItem* pItem = new QTreeWidgetItem(ui->twI , lst, 0);
  return pItem;
}

//QTreeWidgetItem* MainWindow::AddItem(QTreeWidgetItem* item, const QString &itemName)
//{
//  QStringList lst;
//  lst << itemName;
//  QTreeWidgetItem* pItem = new QTreeWidgetItem(item, lst, 0);
//  return pItem;
//}

QTreeWidgetItem* MainWindow::AddItem(QTreeWidgetItem* item, const QString &itemName, const QString &col_1, const QString &col_2, const QString &col_3)
{
  QStringList lst;
  lst << itemName << col_1 << col_2 << col_3;
  QTreeWidgetItem* pItem = new QTreeWidgetItem(item, lst, 0);
  return pItem;
}

void MainWindow::ShowPermanentInfo()
{
    for(quint8 i=0; i<NETS_COUNT; i++)
    {
        SetCellTextInfoAll(0, i, netPacketCounter[i].unit);
        SetCellTextInfoAll(1, i, netPacketCounter[i].all);
        for(quint8 j=1; j<netPacketCounter[i].types.size(); j++)
            SetCellTextInfoAll(1 + j, i, netPacketCounter[i].types[j]);
    }
    ResetNetPacketCounter();
}

void MainWindow::IdSelected()
{
    bool selectState = ShowSelectInfo();
    if(selectState)
        TypeSelected();
    //runTrigger->setEnabled(selectState);
    ui->tblInfoAll->setVisible(!selectState);
    ui->tblInfoAll->setEnabled(!selectState);
    ui->tblInfo->setVisible(selectState);
    ui->tblInfo->setEnabled(selectState);
}

bool MainWindow::ShowSelectInfo()
{
    //qDebug()<< ui->twI->currentItem()->text(0);
    if(ui->twI->currentItem()->parent() == NULL)
    {
        ResetSource();
        for(quint8 i=1; i<=rbPacketTypes.count(); i++)
        {
            rbPacketTypes[i-1]->setEnabled(false);
        }
        for(quint8 i=0; i<chNets.count(); i++)
        {
            chNets[i]->setEnabled(false);
        }

        qDebug()<< "++++++++++++++ no show";
        return false;
    }
    quint16 base = 0, key = 0;
    for(quint8 i=0; i<nodeTypes.size(); i++)
    {
        if(ui->twI->currentItem()->parent()->text(0) == nodeTypes[i])
        {
            switch(i)
            {
            case I_WS:
            case I_PHONE:
            case I_R_MAN:
                base = BASE_PHONE;
                break;
            case I_RADIO:
                base = BASE_RADIO;
                break;
            case I_REC:
                base = BASE_REC;
                break;
            default:
                break;
            }
        }
    }
    static quint16 oldkey;
    curId = ui->twI->currentItem()->text(0).toInt();
    curMS = ui->twI->currentItem()->text(1).toInt();
    key = base + curId*2 + curMS;
    for(quint8 i=0; i<NETS_COUNT; i++)
    {
        SetCellTextInfo(1, i, QHostAddress(sourceMap[key].sourceIp[i]).toString());
        SetCellTextInfo(2, i, QHostAddress(sourceMap[key].senderIp[i]).toString());
        if(runStatus) SetCellTextInfo(3, i, GetCurrentCount(i));
    }

    qDebug()<< "++++++++++++++ show "<< ui->twI->currentItem()->parent()->text(0)<< curId <<curMS;

    //Packet type view
    for(quint8 i=1; i<=rbPacketTypes.count(); i++)
    {
        rbPacketTypes[i-1]->setEnabled(sourceMap[key].typeList[i]);
    }
    if(key!=oldkey)
    {
        for(quint8 i=0; i<rbPacketTypes.count(); i++)
        {
            if(rbPacketTypes[i]->isEnabled())
            {
                rbPacketTypes[i]->setChecked(true);
                break;
            }
        }
    }
    //Nets wiew
    for(quint8 i=0; i<chNets.count(); i++)
    {
        chNets[i]->setEnabled(sourceMap[key].nets[i] == 1);
        if(key!=oldkey) chNets[i]->setChecked(sourceMap[key].nets[i] == 1);
    }


        //qDebug() << "start reset";
    if(cleanStatus) ResetOnTimer();
        //qDebug() << "end reset";
    oldkey = key;
    return true;
}

void MainWindow::ResetOnTimer()
{
//    sourceMap.clear();
    QMap<quint16, CActiveSource>::iterator it = sourceMap.begin();

    //qDebug() << it.key() << sourceMap.end().key();

    for(it = sourceMap.begin(); it != sourceMap.end(); ++it )
    {
        //qDebug() << "reset key" << it.key();
        it.value().Reset();
    }

}



void MainWindow::SetCellTextInfo(int row, int col, const QString &text)
{
    ui->tblInfo->item(row, col)->setText(text);
}

void MainWindow::SetCellTextInfoAll(int row, int col, const quint32 &data)
{
    QString text = QString::number(data * 1000 / TIMER_VIEW);
    ui->tblInfoAll->item(row, col)->setText(text);
}

void MainWindow::SetCellTextData(int row, int col, const QString &text)
{
    ui->tblData->item(row,col)->setText(text);
}

void MainWindow::SetCellTextData(int row, int col, const qint64 &data)
{
    ui->tblData->item(row,col)->setText( QString::number(data));
}

void MainWindow::SetNodeTextData(int level_0, int level_1, const qint64 &data, const QString &text = "")
{
    if(dataItems[level_0]->childCount() > 0)
    {
        dataItems[level_0]->child(level_1)->setText(1, QString::number(data));
        text.isEmpty()?
            dataItems[level_0]->child(level_1)->setText(2, QString::number(data)):
            dataItems[level_0]->child(level_1)->setText(2, text);
    }
    else
    {
        dataItems[level_0]->setText(1, QString::number(data));
        text.isEmpty()?
            dataItems[level_0]->setText(2, QString::number(data)):
            dataItems[level_0]->setText(2, text);
    }
}

void MainWindow::SetNodeTextData(int level_0, int level_1, const QString &data, const QString &text = "")
{
    if(dataItems[level_0]->childCount() > 0)
    {
        dataItems[level_0]->child(level_1)->setText(1, data);
        dataItems[level_0]->child(level_1)->setText(2, text);
    }
    else
    {
        dataItems[level_0]->setText(1, data);
        dataItems[level_0]->setText(2, text);
    }
}

void MainWindow::SetNodeTextData(int level_0, int level_1, const qint64 &data, int modeSelect)
{
    QString text;
    switch (modeSelect)
    {
        case SELECT_ON_OFF:        text = SelectOnOff(data);            break;
        case SELECT_YES_NO:        text = SelectYesNo(data);            break;
        case SELECT_DEVICE:        text = GetTerminalDev(data);         break;
        case SELECT_VOL_R:         text = QString::number(data - 70);   break;
        case SELECT_VOL_D:
        {
            qint64 vol;
            (data > 71)?
            vol = 128 - data:
            vol = - data;
            text = QString::number(vol);   break;
        }
        default:  break;
    }
    if(dataItems[level_0]->childCount() > 0)
    {
        dataItems[level_0]->child(level_1)->setText(1, QString::number(data));
        dataItems[level_0]->child(level_1)->setText(2, text);
    }
    else
    {
        dataItems[level_0]->setText(1, QString::number(data));
        dataItems[level_0]->setText(2, text);
    }
}

void MainWindow::readReady0()
{
    ReadSocket(0);
}

void MainWindow::readReady1()
{
    ReadSocket(1);
}

void MainWindow::ReadSocket(quint8 net)
{
    if(playProcess)
    {
        //unused net data utilization
        while (socket[net]->hasPendingDatagrams())
        {
            QByteArray buffer;
            buffer.resize(socket[net]->pendingDatagramSize());
            QHostAddress sender;
            quint16 port;
            socket[net]->readDatagram(buffer.data(), buffer.size(), &sender, &port);
        }
        return;
    }

    while (socket[net]->hasPendingDatagrams())
    {
        QByteArray buffer;
        buffer.resize(socket[net]->pendingDatagramSize());
        QHostAddress sender;
        QByteArray senderBytes;
        quint16 port;
        socket[net]->readDatagram(buffer.data(), buffer.size(), &sender, &port);
        //
        if(sender.toString().left(7) != hIP[net].toString().left(7))    return;
        //
        MoveUnitPacketCounter(net);
        //qDebug()<< "datagramm size" << buffer.size() << sender;

        for(quint64 j=0; j < buffer.size()/VCSS_P_CTRL_SIZE; j++)
        {
            QByteArray unit = buffer.mid(VCSS_P_CTRL_SIZE * j, VCSS_P_CTRL_SIZE);
            const unsigned char* pBuff = (const unsigned char*)unit.data();

            CControlPacket2 packet;
            CControlPacket2* pPacket = &packet;
            megpacket.UnPackControl(pPacket, pBuff);

            if(pPacket->m_Header.nSignature1 == VCSS_SIGNATURE_1)
                if(pPacket->m_Header.nSignature2 == VCSS_SIGNATURE_2)
                    if(pPacket->m_Header.nSignature3 == VCSS_SIGNATURE_3)
                    {
                        SourceMapUpdate(pPacket, net, &sender);
                        CountPacket(pPacket, net);
                        if(runStatus)
                            if(chNets[net]->isChecked())
                            {
                                ReadPacket(pPacket, net);
                            }
                        if(recordStatus)
                        {
                            toBytes(&sender, &senderBytes);
                            recA.AddData(unit, senderBytes, net);
                            recB.AddData(unit, senderBytes, net);
                        }
                    }
        }
    }
}

void MainWindow::toBytes(QHostAddress *pHost, QByteArray *pBa)
{
    quint32 data = pHost->toIPv4Address();
    char a = (data >> 24) & 0XFF;
    char b = (data >> 16) & 0XFF;
    char c = (data >> 8) & 0XFF;
    char d = data & 0XFF;
    pBa->clear();
    pBa->append(d);
    pBa->append(c);
    pBa->append(b);
    pBa->append(a);
}


void MainWindow::ReadPlayBuffer(QByteArray *pPlayData)
{
    if(!playProcess)  return;

        QByteArray senderBytes;
        quint32 uiAddr;
        quint8 net;
        quint8 unitSize = VCSS_P_CTRL_SIZE + 5;

        for(quint64 j=0; j < (uint)(pPlayData->size()/unitSize); j++)
        {
            QByteArray unit = pPlayData->mid(unitSize * j, unitSize);
            net = (unsigned char)(unit[0]);
            senderBytes = unit.mid(1,4);
            memcpy(&uiAddr, senderBytes.constData(),4);
            QHostAddress sender(uiAddr);


            unit = unit.right(VCSS_P_CTRL_SIZE);
            const unsigned char* pBuff = (const unsigned char*)unit.data();

            CControlPacket2 packet;
            CControlPacket2* pPacket = &packet;
            megpacket.UnPackControl(pPacket, pBuff);

            if(pPacket->m_Header.nSignature1 == VCSS_SIGNATURE_1)
                if(pPacket->m_Header.nSignature2 == VCSS_SIGNATURE_2)
                    if(pPacket->m_Header.nSignature3 == VCSS_SIGNATURE_3)
                    {
                        SourceMapUpdate(pPacket, net, &sender);
                        CountPacket(pPacket, net);
                        if(runStatus)
                        {
                            if(chNets[net]->isChecked())
                            {
                                ReadPacket(pPacket, net);
                            }
                            if(broadcastStatus)
                            {
                                SendPacket(unit, net);
                            }
                        }
                    }
        }


}

void MainWindow::tickProcessing()
{
    if(recordStatus)
    {
        bool flag_recB_Activate = false;
        if(recA.AddTime())
        {
            flag_recB_Activate = true;
            recB.Activate();
            recA.WriteData();
            qDebug()<< "recA write file";
        }
        if(!flag_recB_Activate)
        {
            if(recB.AddTime())
            {
                recA.Activate();
                recB.WriteData();
                qDebug()<< "recB write file";
            }
        }
        QTime t(0,0,0);
        t = t.addMSecs((recA.GetMapSize() + recB.GetMapSize()) * TIMER_TICK);
        sbRecordTime->setText(t.toString("hh:mm:ss.zzz"));
    }
    if(playProcess && runStatus)
    {
//        static quint32 progress;
        QByteArray tickData;
        QDateTime tickTime;
        bool rePlay = replayProcess;
//        qDebug()<< "Tick & play" << playProcess << rePlay;
        if(replayProcess) ResetTableView();
        playProcess = player.GetDataByTick(&tickData, &tickTime, &rePlay);
        replayProcess = rePlay;
        ReadPlayBuffer(&tickData);
        sbPlayProcess->setValue(player.progress);
//        qDebug()<< "PlayProcess" << player.progress << tickTime.toString("hh:mm:ss.zzz");
        QTime t(0,0,0);
        t = t.addMSecs((sbPlayProcess->maximum() - player.progress) * TIMER_TICK);
        sbRecordTime->setText(t.toString("hh:mm:ss.zzz"));
        sbTickTime->setText(tickTime.toString("yyyy-MM-dd hh:mm:ss.zzz"));

//        qDebug()<< "Data size" << tickData.size();
//        if(tickData.size() > 0)
//              qDebug()<< "Data" << tickData.left(1).toInt() << tickData.mid(5,3) << tickTime.toMSecsSinceEpoch() ;

        if(!playProcess)
        {
            //End of playing
            EndPlaying();
        }
    }
}

void MainWindow::EndPlaying()
{
    //End of playing
    ResetInterfaces();
    sbStatus->setText(tr("Online"));
    sbRecordTime->setVisible(false);
    recordTrigger->setEnabled(true);
    sbPlayProcess->setValue(0);
    sbPlayProcess->setVisible(false);
    sbTickTime->setVisible(false);
    sbPlayFile->setVisible(false);
    replayTrigger->setEnabled(false);
    endplayTrigger->setEnabled(false);
    if(broadcastStatus) BroadcastButtonClicked();
    broadcastTrigger->setEnabled(false);
}

void MainWindow::ResetInterfaces()
{
    dontUpdateInerfacesFlag = true;
    QMap<quint16, CActiveSource>::iterator it = sourceMap.begin();
    for(it = sourceMap.begin(); it != sourceMap.end(); )
    {
           DelItemInterface(it.key());
           if(sourceMap.size() > 1)
           {
               sourceMap.remove(it++.key());
           }
           else
           {
               sourceMap.remove(it.key());
               break;
           }
    }
    dontUpdateInerfacesFlag = false;
}

void MainWindow::timerProcessing()
{
    static quint64 counter;
    qDebug()<<"=+++="<< ++counter << "Play" << playProcess << "Run" << runStatus;

    if(dontUpdateInerfacesFlag) return;
    if(playProcess && !runStatus) return;

    QMap<quint16, CActiveSource>::iterator it = sourceMap.begin();
    for(it = sourceMap.begin(); it != sourceMap.end(); )
    {
       quint64 timeout = it.value().GetTime();
       it.value().TypeAnalysis(it.key());
       AddItemInterface(it.key());

//       QString s;
//       foreach (quint8 entry, it.value().typeList)
//       {
//           s+=QString::number(entry);
//       }
//       qDebug() << it.key() << " : " << s << it.value().iType << it.value().ws << timeout  ;

       bool missing = timeout > LOSS_THRESHOLD;
       bool remove = missing && cleanStatus;

       if(!cleanStatus)
       {
           if(it.value().offLine != missing)
           {
               ColorItemInterface(it.key(), missing);
               it.value().offLine = missing;
           }
       }

       if(remove)
       {
//           quint8 itemType = it.value().iType;
//           qDebug()<<"Start Map size: " << sourceMap.size() << "TW count::" << items[itemType]->childCount();
           DelItemInterface(it.key());
//           qDebug()<<"start remove " << it.key();
           if(sourceMap.size() > 1)
           {
               sourceMap.remove(it++.key());
//               qDebug()<<"end remove";
           }
           else
           {
               sourceMap.remove(it.key());
//               qDebug()<<"break after last item remove";
               break;
           }

//           qDebug()<<"End   Map size: " << sourceMap.size() << "TW count:" << items[itemType]->childCount();
       }
       else
       {
           ++it;
       }
    }

    if(ui->twI->currentItem() != NULL) ShowSelectInfo();
    ShowPermanentInfo();

//    qDebug()<<"end tp" ;
}

void MainWindow::AddItemInterface(quint16 key)
{
    //        qDebug() <<"+++++++++++++++++";
    if(!sourceMap[key].view)
    {
        QTextCodec *codec = QTextCodec::codecForName("KOI8-R");
        QString interfaceName = codec->toUnicode(sourceMap[key].byteName);
//        qDebug() <<"+++++start add " << key << sourceMap[key].iType ;
        AddItem(sourceItems[sourceMap[key].iType], sourceMap[key].GetIdText(), sourceMap[key].GetMsText(), TrimName(interfaceName));
//        qDebug() <<"+++++end add " << key << sourceMap[key].iType ;
        sourceItems[sourceMap[key].iType]->sortChildren(1, Qt::AscendingOrder);
        sourceItems[sourceMap[key].iType]->sortChildren(0, Qt::AscendingOrder);
        sourceMap[key].view = true;
    }
}

void MainWindow::DelItemInterface(quint16 key)
{
    //qDebug()<<"--need delete! " <<key;
    QString findItem = sourceMap[key].GetIdText();
    QString findItemMS = sourceMap[key].GetMsText();
    QString findItemsParent = nodeTypes[sourceMap[key].iType];
    QString findItemsParentEx = "";
    if(sourceMap[key].iType == I_PHONE) findItemsParentEx = nodeTypes[I_WS];
    QTreeWidgetItemIterator it(ui->twI);
    while (*it)
    {
        //qDebug()<<"sr"<<(*it)->text(0) << findItem << (*it)->text(1) << findItemMS ;
        if((*it)->text(0) == findItem && (*it)->text(1) == findItemMS)
        {
            //qDebug()<<"srp"<<(*it)->parent()->text(0) << findItemsParent;
            if((*it)->parent()->text(0) == findItemsParent
                    || (*it)->parent()->text(0) == findItemsParentEx)
            {
               //qDebug()<<"--success " << findItem << findItemMS << findItemsParent;
               // qDebug()<<"--suc!" << (*it)->text(0) << (*it)->text(1) << (*it)->parent()->text(0) << (*it)->parent()->childCount();
                (*it)->parent()->removeChild((*it));
               // qDebug()<<"rch";
                break;
            }
        }
        ++it;
    }
}

void MainWindow::ColorItemInterface(quint16 key, bool status)
{
        static QBrush redBrush = *(new QBrush(Qt::red));
        static QBrush blackBrush = *(new QBrush(Qt::black));
        QBrush current;
        status? current=redBrush: current=blackBrush;
        QString findItem = sourceMap[key].GetIdText();
        QString findItemMS = sourceMap[key].GetMsText();
        QString findItemsParent = nodeTypes[sourceMap[key].iType];
        QString findItemsParentEx = "";
        if(sourceMap[key].iType == I_PHONE) findItemsParentEx = nodeTypes[I_WS];
        QTreeWidgetItemIterator it(ui->twI);
        while (*it)
        {
            if((*it)->text(0) == findItem && (*it)->text(1) == findItemMS)
            {
                if((*it)->parent()->text(0) == findItemsParent
                        || (*it)->parent()->text(0) == findItemsParentEx)
                {
                    for(quint8 i=0; i<3; i++)
                        (*it)->setForeground(i, current);
                }
            }
            ++it;
        }
}

void MainWindow::SourceMapUpdate(CControlPacket2 *pPacket, quint8 net, QHostAddress *pSender)
{
    quint16 key = 0;
    QByteArray bufferName;
    quint32 ip0 = 0, ip1 = 0;
    quint8 ms = 0;
    bool ws = false;

    switch ( pPacket->m_Header.nType )
    {
        case VCSS_TYPE_ULTRA:
        {
            switch(pPacket->m_Ctrl.t0.nUltraType)
            {
                case VCSS_TYPE_ULTRA_REC:
                {
                    key = pPacket->m_Header.nId*2 + BASE_REC;

                    ip0 = pPacket->m_Ctrl.t0.body.record.nIP0;
                    ip1 = pPacket->m_Ctrl.t0.body.record.nIP1;

                    break;
                }
                case VCSS_TYPE_ULTRA_PERMANENT_DIAG:
                {
//                    qDebug() << "(net)" << net << pPacket->m_Ctrl.t0.nUltraType << sender;
                    key = pPacket->m_Header.nId*2 +  pPacket->m_Header.nSubId + BASE_PHONE;
                    ms = pPacket->m_Header.nSubId;
                    break;
                }
                case VCSS_TYPE_ULTRA_INTERFACE_TERMINAL_DIAG_COMMON:
                case VCSS_TYPE_ULTRA_INTERFACE_TERMINAL_DIAG_RADIO:
                case VCSS_TYPE_ULTRA_TERMINAL_EXTENSIONS:
                case VCSS_TYPE_ULTRA_DIVERT_GROUPS:
                {
                    key = pPacket->m_Header.nId*2 +  pPacket->m_Header.nSubId + BASE_PHONE;
                    ms = pPacket->m_Header.nSubId;
                    break;
                }
            }
            break;
        }
        case VCSS_TYPE_PHONE:
        {
            QByteArray buff;
            buff.resize(sizeof(pPacket->m_Ctrl.t1.dwlENA));
            memmove(buff.data(), &(pPacket->m_Ctrl.t1.dwlENA), sizeof(pPacket->m_Ctrl.t1.dwlENA));
            bufferName = buff;

            ip0 = pPacket->m_Ctrl.t1.nIP0;
            ip1 = pPacket->m_Ctrl.t1.nIP1;
            ws = pPacket->m_Ctrl.t1.bCallQueue == 1;
        }
        case VCSS_TYPE_TERMINAL:
        {
            key = pPacket->m_Header.nId*2 + pPacket->m_Header.nSubId + BASE_PHONE;
            ms = pPacket->m_Header.nSubId;            break;


        }
        case VCSS_TYPE_RADIO:
        {
            key = pPacket->m_Header.nId*2 + pPacket->m_Header.nDevId + BASE_RADIO;
            for(quint8 i=0; i<8; i++)
            {
                bufferName.append(pPacket->m_Ctrl.t2.cName[i]);
            }
            ms = pPacket->m_Header.nDevId;
            ip0 = pPacket->m_Ctrl.t2.nIP0;
            ip1 = pPacket->m_Ctrl.t2.nIP1;

            break;
        }
    }

    CActiveSource *pSource = &sourceMap[key];
    pSource->id = pPacket->m_Header.nId;
    pSource->ms = ms;
    pSource->typeList[pPacket->m_Header.nType] = true;
    if(pPacket->m_Header.nType == 0)
        pSource->typeList[pPacket->m_Ctrl.t0.nUltraType + 3] = true;
    if(!pSource->ws) pSource->ws = ws;
    if(pSource->sourceIp[0] == 0)       pSource->SetSourceIp(ip0, ip1);
    if(pSource->nets[net] == 0)         pSource->nets[net] = 1;
    if(pSource ->senderIp[net] == 0)    pSource->senderIp[net] = pSender->toIPv4Address();

    pSource->SetTime();
    if(pSource->byteName.size() == 0) pSource->byteName = bufferName;
}

void MainWindow::CountPacket(CControlPacket2 *pPacket, quint8 net)
{
    // move common counters
    if(pPacket->m_Header.nType > 0)
        MoveNetPacketCounter(net, pPacket->m_Header.nType);
    else
        MoveNetPacketCounter(net, pPacket->m_Ctrl.t0.nUltraType + 3);
}

void MainWindow::ReadPacket(CControlPacket2 *pPacket, quint8 net)
{
    // read selected
    if(pPacket->m_Header.nId == curId)
    {
        if(pPacket->m_Header.nType == curType)
        {
            if((pPacket->m_Header.nDevId == curMS && curType == VCSS_TYPE_RADIO)
                    || (pPacket->m_Header.nSubId == curMS && curType != VCSS_TYPE_RADIO))
            {
                if((pPacket->m_Header.nType == 0 &&
                        pPacket->m_Ctrl.t0.nUltraType == curUltraType)
                        || pPacket->m_Header.nType >0)
                {

//                    qDebug()<< "yes__ _ _ _ _ __________________";
//                    qDebug()<<pPacket->m_Header.nId<<curId;
//                    qDebug()<<pPacket->m_Header.nType<<curType;
//                    qDebug()<<pPacket->m_Header.nDevId<<curMS;
//                    qDebug()<<pPacket->m_Ctrl.t0.nUltraType<<curUltraType;
//                    qDebug()<<"<*"<< pPacket->m_Header.nId<<pPacket->m_Ctrl.t2.bSquelsh<<pPacket->m_Ctrl.t2.nQOS<<pPacket->m_Ctrl.t2.nLevel;


                    MoveCurrentPacketCounter(net);
                    if(treeStatus)
                    {
                        FillHeaderForTree(pPacket);
                        FillBodyForTree(pPacket);
                    }
                    else
                    {
                        FillHeaderForTable(pPacket);
                        FillBodyForTable(pPacket);
                    }
                }
            }
        }
    }
}

void MainWindow::SendPacket(const QByteArray &unit, quint8 net)
{
    if(net < socket.size())
    {
        socket[net]->writeDatagram(unit, hIpBroadcast[net], PORT_CONTROL);
    }
}

void MainWindow::FillBodyForTable(CControlPacket2 *pPacket)
{
    QTextCodec *codec = QTextCodec::codecForName("KOI8-R");

    switch ( pPacket->m_Header.nType )
    {
        case VCSS_TYPE_ULTRA:
        {
            SetCellTextData(14, counter, pPacket->m_Ctrl.t0.nUltraType);
            switch(pPacket->m_Ctrl.t0.nUltraType)
            {
                case VCSS_TYPE_ULTRA_REC:
                {
                    quint32 ip0 = pPacket->m_Ctrl.t0.body.record.nIP0,
                            ip1 = pPacket->m_Ctrl.t0.body.record.nIP1;
                    SetCellTextData(15, counter, QHostAddress(swapByteOrder(ip0)).toString());
                    SetCellTextData(16, counter, QHostAddress(swapByteOrder(ip1)).toString());
                    SetCellTextData(17, counter, pPacket->m_Ctrl.t0.body.record.nChannelsNumber);
                    for(quint8 i=0; i<VCSS_P_ULTRA_RECORD_CHANNELS_NUM; i++)
                    {
                        SetCellTextData(18 + i*4, counter, pPacket->m_Ctrl.t0.body.record.channels[i].nId);
                        SetCellTextData(19 + i*4, counter, pPacket->m_Ctrl.t0.body.record.channels[i].nType);
                        SetCellTextData(20 + i*4, counter, pPacket->m_Ctrl.t0.body.record.channels[i].bRadioMain);
                        SetCellTextData(21 + i*4, counter, pPacket->m_Ctrl.t0.body.record.channels[i].bRes);
                    }
                    counterNext();
                    break;
                }
                case VCSS_TYPE_ULTRA_PERMANENT_DIAG:
                {
                    SetCellTextData(15, counter, pPacket->m_Ctrl.t0.body.permanentDiag.nPermanentDiagType);
                    SetCellTextData(16, counter, pPacket->m_Ctrl.t0.body.permanentDiag.body.permanentDiagTerminalE1.dwLineStatus);
                    SetCellTextData(17, counter, pPacket->m_Ctrl.t0.body.permanentDiag.body.permanentDiagTerminalE1.bInCAS);
                    SetCellTextData(18, counter, pPacket->m_Ctrl.t0.body.permanentDiag.body.permanentDiagTerminalE1.bOutCAS);
                    SetCellTextData(19, counter, pPacket->m_Ctrl.t0.body.permanentDiag.body.permanentDiagTerminalE1.bChannelNoReady);
                    SetCellTextData(20, counter, pPacket->m_Ctrl.t0.body.permanentDiag.body.permanentDiagTerminalE1.bRes2);
//                    QByteArray bufferName;
//                    for(quint8 i=0; i<18; i++)
//                    {
//                        bufferName.append(pPacket->m_Ctrl.t0.body.permanentDiag.body.permanentDiagTerminalE1.nRes1[i]);
//                    }
//                    SetCellTextData(20, counter, codec->toUnicode(bufferName));
                    counterNext();
                    break;
                }
                case VCSS_TYPE_ULTRA_INTERFACE_TERMINAL_DIAG_COMMON:
                {
                    SetCellTextData(15, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.micPhoneActive);
                    SetCellTextData(16, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.micRadioActive);
                    SetCellTextData(17, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bHeadset1Present);
                    SetCellTextData(18, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bHeadset2Present);
                    SetCellTextData(19, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwMasterVolPhone);
                    SetCellTextData(20, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwMasterVolRadio);
                    SetCellTextData(21, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwMechTangentaSlot);
                    SetCellTextData(22, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bExtraLoud);
                    SetCellTextData(23, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bSelfListen);
                    SetCellTextData(24, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bDefaultSettings);
                    SetCellTextData(25, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bWorkBtnPushed);
                    SetCellTextData(26, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bOperativeCall);
                    SetCellTextData(27, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bFaceIsDying);
                    SetCellTextData(28, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwVolHeadset1);
                    SetCellTextData(29, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwVolHeadset2);
                    SetCellTextData(30, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwVolHandset);
                    SetCellTextData(31, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwDefPhoneOutDev);
                    SetCellTextData(32, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwDefPhoneVol);
                    for(quint8 i=0; i<VCSS_P_ULTRA_TERM_DIAG_COMMON_ABONENTS_NUM; i++)
                    {
                        SetCellTextData(33 + i*5, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.abonents[i].nId);
                        SetCellTextData(34 + i*5, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.abonents[i].dwOutDev);
                        SetCellTextData(35 + i*5, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.abonents[i].dwRes1);
                        SetCellTextData(36 + i*5, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.abonents[i].dwVol);
                        SetCellTextData(37 + i*5, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.abonents[i].nRes2);
                    }

                    SetCellTextData(58, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwDivert);
                    SetCellTextData(59, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwRingVol);
                    SetCellTextData(60, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwBrightness);
                    SetCellTextData(61, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bRxAdd);
                    SetCellTextData(62, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bPTTBlocked);
                    SetCellTextData(63, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwRes);
                    counterNext();
                    break;
                }
                case VCSS_TYPE_ULTRA_INTERFACE_TERMINAL_DIAG_RADIO:
                {
                for(quint8 i=0; i<VCSS_MAX_RADIOS_ON_WP; i++)
                {
                        SetCellTextData(15 + i*6, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.m_slots[i].dwSlotVol);
                        SetCellTextData(16 + i*6, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.m_slots[i].dwSlotOutDev);
                        SetCellTextData(17 + i*6, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.m_slots[i].bSlotBSS);
                        SetCellTextData(18 + i*6, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.m_slots[i].bSlotLOTS);
                        SetCellTextData(19 + i*6, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.m_slots[i].bSlotAllTx);
                        SetCellTextData(20 + i*6, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.m_slots[i].nSlotRetranslation);
                    }

                    SetCellTextData(87, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.dwRes1);
                    SetCellTextData(88, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.dwRes2);
                    SetCellTextData(89, counter, pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.dwRes3);
                    counterNext();
                    break;
                }

                case VCSS_TYPE_ULTRA_TERMINAL_EXTENSIONS:
                {
                    SetCellTextData(15, counter, pPacket->m_Ctrl.t0.body.terminalExtensions.nExtensionType);
                    for(quint8 i=0; i<VCSS_P_ULTRA_TERMINAL_EXTENSIONS_LISTEN_NUM; i++)
                    {
                        SetCellTextData(16 + i, counter, pPacket->m_Ctrl.t0.body.terminalExtensions.body.terminalExtensionsListen.narListenId[i]);
                    }
                    QByteArray bufferName;
                    for(quint8 i=0; i<14; i++)
                    {
                        bufferName.append(pPacket->m_Ctrl.t0.body.terminalExtensions.body.terminalExtensionsListen.res[i]);
                    }
                    SetCellTextData(22, counter, codec->toUnicode(bufferName));
                    counterNext();
                    break;
                }
                case VCSS_TYPE_ULTRA_DIVERT_GROUPS:
                {
                    SetCellTextData(15, counter, pPacket->m_Ctrl.t0.body.divertGroups.nTotalAliases);
                    SetCellTextData(16, counter, pPacket->m_Ctrl.t0.body.divertGroups.nDivertDst);
                    SetCellTextData(17, counter, pPacket->m_Ctrl.t0.body.divertGroups.nRes);
                    for(quint8 i=0; i<VCSS_DIVERT_GROUP_SIZE; i++)
                    {
                        SetCellTextData(18 + i, counter, pPacket->m_Ctrl.t0.body.divertGroups.Aliases[i]);
                    }
                    counterNext();
                    break;
                }
            }
            break;
        }
        case VCSS_TYPE_PHONE:
        {
            SetCellTextData(14, counter, pPacket->m_Ctrl.t1.nLevel);
            SetCellTextData(15, counter, pPacket->m_Ctrl.t1.nListenId);
            SetCellTextData(16, counter, pPacket->m_Ctrl.t1.nGroupSign);
            SetCellTextData(17, counter, pPacket->m_Ctrl.t1.nGroup);
            SetCellTextData(18, counter, pPacket->m_Ctrl.t1.nDestId);
            SetCellTextData(19, counter, pPacket->m_Ctrl.t1.nCallType);
            SetCellTextData(20, counter, pPacket->m_Ctrl.t1.bIntr);
            SetCellTextData(21, counter, pPacket->m_Ctrl.t1.bHalfDuplex);
            SetCellTextData(22, counter, pPacket->m_Ctrl.t1.nConnType);
            SetCellTextData(23, counter, pPacket->m_Ctrl.t1.nCmd);
            SetCellTextData(24, counter, pPacket->m_Ctrl.t1.nPriority);
            SetCellTextData(25, counter, pPacket->m_Ctrl.t1.bBusy);
            SetCellTextData(26, counter, pPacket->m_Ctrl.t1.nRes3);
            QByteArray buff;
                        buff.resize(sizeof(pPacket->m_Ctrl.t1.dwlENA));
                        memmove(buff.data(), &(pPacket->m_Ctrl.t1.dwlENA), sizeof(pPacket->m_Ctrl.t1.dwlENA));
//            if(pPacket->m_Ctrl.t1.nDestId == 0)
//            if(pPacket->m_Ctrl.t1.nDestId != 0 && pPacket->m_Ctrl.t1.nPriority != 0 )
            if(pPacket->m_Ctrl.t1.nDestId != 0 && pPacket->m_Ctrl.t1.nPriority != 0 && pPacket->m_Ctrl.t1.nCmd != VCSS_PCMD_HOLD)
            {
                SetCellTextData(27, counter, toENA(buff));
            }
            else
            {
                SetCellTextData(27, counter,  TrimName(codec->toUnicode(buff)));
            }
            quint32 ip0 = pPacket->m_Ctrl.t1.nIP0,
                    ip1 = pPacket->m_Ctrl.t1.nIP1;
            SetCellTextData(28, counter, QHostAddress(swapByteOrder(ip0)).toString());
            SetCellTextData(29, counter, QHostAddress(swapByteOrder(ip1)).toString());
            SetCellTextData(30, counter, pPacket->m_Ctrl.t1.nHWstate);
            SetCellTextData(31, counter, pPacket->m_Ctrl.t1.nBusy);
            SetCellTextData(32, counter, pPacket->m_Ctrl.t1.bNeedEC);
            SetCellTextData(33, counter, pPacket->m_Ctrl.t1.bCallQueue);
            SetCellTextData(34, counter, pPacket->m_Ctrl.t1.bInDivert);
            SetCellTextData(35, counter, pPacket->m_Ctrl.t1.nRes2);
            counterNext();
            break;
        }
        case VCSS_TYPE_TERMINAL:
        {
            SetCellTextData(14, counter, pPacket->m_Ctrl.t3.bPriority);
            SetCellTextData(15, counter, pPacket->m_Ctrl.t3.nCmd);
            SetCellTextData(16, counter, pPacket->m_Ctrl.t3.nData);
            for(quint8 i=0; i<VCSS_MAX_RADIOS_ON_WP; i++)
            {
                SetCellTextData(17 + i*8, counter, pPacket->m_Ctrl.t3.Channels[i].nId);
                SetCellTextData(18 + i*8, counter, pPacket->m_Ctrl.t3.Channels[i].bTransmit);
                SetCellTextData(19 + i*8, counter, pPacket->m_Ctrl.t3.Channels[i].nRxMS);
                SetCellTextData(20 + i*8, counter, pPacket->m_Ctrl.t3.Channels[i].bTxMS);
                SetCellTextData(21 + i*8, counter, pPacket->m_Ctrl.t3.Channels[i].nCmd);
                SetCellTextData(22 + i*8, counter, pPacket->m_Ctrl.t3.Channels[i].bRx);
                SetCellTextData(23 + i*8, counter, pPacket->m_Ctrl.t3.Channels[i].bTx);
                SetCellTextData(24 + i*8, counter, pPacket->m_Ctrl.t3.Channels[i].bEmergency);
            }
            counterNext();
            break;
        }
        case VCSS_TYPE_RADIO:
        {
            SetCellTextData(14, counter, pPacket->m_Ctrl.t2.bRxEnable);
            SetCellTextData(15, counter, pPacket->m_Ctrl.t2.bTxEnable);
            SetCellTextData(16, counter, pPacket->m_Ctrl.t2.nRxMS);
            SetCellTextData(17, counter, pPacket->m_Ctrl.t2.bTxMS);
            SetCellTextData(18, counter, pPacket->m_Ctrl.t2.bSquelsh);
            SetCellTextData(19, counter, pPacket->m_Ctrl.t2.nFreqInt);
            SetCellTextData(20, counter, pPacket->m_Ctrl.t2.nFreqFract);
            SetCellTextData(21, counter, pPacket->m_Ctrl.t2.nOwner);
            SetCellTextData(22, counter, pPacket->m_Ctrl.t2.bDegradation);
            SetCellTextData(23, counter, pPacket->m_Ctrl.t2.nRes0);
            SetCellTextData(24, counter, pPacket->m_Ctrl.t2.nCmd);
            SetCellTextData(25, counter, pPacket->m_Ctrl.t2.nLevel);
            SetCellTextData(26, counter, pPacket->m_Ctrl.t2.nRes1);
            QByteArray bufferName;
            for(int i=0; i<8; i++)
            {
                bufferName.append(pPacket->m_Ctrl.t2.cName[i]);
            }
            quint32 ip0 = pPacket->m_Ctrl.t2.nIP0,
                    ip1 = pPacket->m_Ctrl.t2.nIP1;
            SetCellTextData(27, counter, TrimName(codec->toUnicode(bufferName)));
            SetCellTextData(28, counter, QHostAddress(swapByteOrder(ip0)).toString());
            SetCellTextData(29, counter, QHostAddress(swapByteOrder(ip1)).toString());
            SetCellTextData(30, counter, pPacket->m_Ctrl.t2.nHWstate);
            SetCellTextData(31, counter, pPacket->m_Ctrl.t2.nQOS);
            SetCellTextData(32, counter, pPacket->m_Ctrl.t2.nRes2);
            counterNext();
            break;
        }
    }

}



void MainWindow::FillBodyForTree(CControlPacket2 *pPacket)
{
    QTextCodec *codec = QTextCodec::codecForName("KOI8-R");
    quint8 curNode;

    switch ( pPacket->m_Header.nType )
    {
        case VCSS_TYPE_ULTRA:
        {
            QString text;
            switch (pPacket->m_Ctrl.t0.nUltraType)
            {
                case VCSS_TYPE_ULTRA_REC:                               text = tr("Record"); break;
                case VCSS_TYPE_ULTRA_INTERFACE_TERMINAL_DIAG_COMMON:    text = tr("Terminal diagnostics common"); break;
                case VCSS_TYPE_ULTRA_INTERFACE_TERMINAL_DIAG_RADIO:     text = tr("Terminal diagnostics radio"); break;
                case VCSS_TYPE_ULTRA_PERMANENT_DIAG:                    text = tr("Permanent diagnostics"); break;
                case VCSS_TYPE_ULTRA_TERMINAL_EXTENSIONS:               text = tr("Terminal extensions"); break;
                case VCSS_TYPE_ULTRA_DIVERT_GROUPS:                     text = tr("Divert groups"); break;
                case VCSS_TYPE_ULTRA_LOG_SIP:                           text = tr("Log SIP"); break;
            }

            SetNodeTextData(12, 0, pPacket->m_Ctrl.t0.nUltraType, text);
            switch(pPacket->m_Ctrl.t0.nUltraType)
            {
                case VCSS_TYPE_ULTRA_REC:
                {
                    quint32 ip0 = pPacket->m_Ctrl.t0.body.record.nIP0,
                            ip1 = pPacket->m_Ctrl.t0.body.record.nIP1;
                    SetNodeTextData(13, 0, QHostAddress(swapByteOrder(ip0)).toString(), tr("IP address LAN A"));
                    SetNodeTextData(14, 0, QHostAddress(swapByteOrder(ip1)).toString(), tr("IP address LAN B"));
                    SetNodeTextData(15, 0, pPacket->m_Ctrl.t0.body.record.nChannelsNumber);
                    for(quint8 i=0; i<VCSS_P_ULTRA_RECORD_CHANNELS_NUM; i++)
                    {
                        curNode = 16 + i;
                        SetNodeTextData(curNode, 0, pPacket->m_Ctrl.t0.body.record.channels[i].nId);
                        switch (pPacket->m_Ctrl.t0.body.record.channels[i].nType)
                        {
                            case 0: text = tr("Phone"); break;
                            case 1: text = tr("Radio-terminal"); break;
                            case 2: text = tr("Transparent cannel"); break;
                            case 3: text = tr("Radio-station"); break;
                        }
                        SetNodeTextData(curNode, 1, pPacket->m_Ctrl.t0.body.record.channels[i].nType, text);
                        SetNodeTextData(curNode, 2, pPacket->m_Ctrl.t0.body.record.channels[i].bRadioMain,
                                        SelectText(pPacket->m_Ctrl.t0.body.record.channels[i].bRadioMain, tr("Main"), tr("Stanby")));
                        SetNodeTextData(curNode, 3, pPacket->m_Ctrl.t0.body.record.channels[i].bRes);
                    }
                    break;
                }
                case VCSS_TYPE_ULTRA_PERMANENT_DIAG:
                {
                switch (pPacket->m_Ctrl.t0.body.permanentDiag.nPermanentDiagType)
                {
                    case 1: text = tr("Terminal E1"); break;
                    default: text = tr("None"); break;
                }
                    SetNodeTextData(13, 0, pPacket->m_Ctrl.t0.body.permanentDiag.nPermanentDiagType, text);
                    SetNodeTextData(14, 0, pPacket->m_Ctrl.t0.body.permanentDiag.body.permanentDiagTerminalE1.dwLineStatus);
                    SetNodeTextData(15, 0, pPacket->m_Ctrl.t0.body.permanentDiag.body.permanentDiagTerminalE1.bInCAS, SELECT_YES_NO);
                    SetNodeTextData(16, 0, pPacket->m_Ctrl.t0.body.permanentDiag.body.permanentDiagTerminalE1.bOutCAS, SELECT_YES_NO);
                    SetNodeTextData(17, 0, pPacket->m_Ctrl.t0.body.permanentDiag.body.permanentDiagTerminalE1.bChannelNoReady, SELECT_YES_NO);
                    SetNodeTextData(18, 0, pPacket->m_Ctrl.t0.body.permanentDiag.body.permanentDiagTerminalE1.bRes2);
                    break;
                }
                case VCSS_TYPE_ULTRA_INTERFACE_TERMINAL_DIAG_COMMON:
                {
                    SetNodeTextData(13, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.micPhoneActive, SELECT_DEVICE);
                    SetNodeTextData(14, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.micRadioActive, SELECT_DEVICE);
                    SetNodeTextData(15, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bHeadset1Present, SELECT_YES_NO);
                    SetNodeTextData(16, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bHeadset2Present, SELECT_YES_NO);
                    SetNodeTextData(17, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwMasterVolPhone, SELECT_VOL_R);
                    SetNodeTextData(18, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwMasterVolRadio, SELECT_VOL_R);
                    SetNodeTextData(19, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwMechTangentaSlot);
                    SetNodeTextData(20, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bExtraLoud, SELECT_ON_OFF);
                    SetNodeTextData(21, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bSelfListen, SELECT_ON_OFF);
                    SetNodeTextData(22, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bDefaultSettings, SELECT_YES_NO);
                    SetNodeTextData(23, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bWorkBtnPushed, SELECT_YES_NO);
                    SetNodeTextData(24, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bOperativeCall, SELECT_ON_OFF);
                    SetNodeTextData(25, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bFaceIsDying, SELECT_YES_NO);
                    SetNodeTextData(26, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwVolHeadset1, SELECT_VOL_D);
                    SetNodeTextData(27, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwVolHeadset2, SELECT_VOL_D);
                    SetNodeTextData(28, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwVolHandset, SELECT_VOL_D);
                    SetNodeTextData(29, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwDefPhoneOutDev, SELECT_DEVICE);
                    SetNodeTextData(30, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwDefPhoneVol, SELECT_VOL_D);
                    for(quint8 i=0; i<VCSS_P_ULTRA_TERM_DIAG_COMMON_ABONENTS_NUM; i++)
                    {
                        curNode = 31 + i;
                        SetNodeTextData(curNode, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.abonents[i].nId);
                        SetNodeTextData(curNode, 1, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.abonents[i].dwOutDev, SELECT_DEVICE);
                        SetNodeTextData(curNode, 2, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.abonents[i].dwRes1);
                        SetNodeTextData(curNode, 3, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.abonents[i].dwVol, SELECT_VOL_D);
                        SetNodeTextData(curNode, 4, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.abonents[i].nRes2);
                    }

                    SetNodeTextData(36, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwDivert);
                    SetNodeTextData(37, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwRingVol, SELECT_VOL_R);
                    SetNodeTextData(38, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwBrightness);
                    SetNodeTextData(39, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bRxAdd, SELECT_ON_OFF);
                    SetNodeTextData(40, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.bPTTBlocked, SELECT_YES_NO);
                    SetNodeTextData(41, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagCommon.dwRes);
                    break;
                }
                case VCSS_TYPE_ULTRA_INTERFACE_TERMINAL_DIAG_RADIO:
                {
                    for(quint8 i=0; i<VCSS_MAX_RADIOS_ON_WP; i++)
                    {
                        curNode = 13 + i;
                        SetNodeTextData(curNode, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.m_slots[i].dwSlotVol, SELECT_VOL_D);
                        SetNodeTextData(curNode, 1, pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.m_slots[i].dwSlotOutDev, SELECT_DEVICE);
                        SetNodeTextData(curNode, 2, pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.m_slots[i].bSlotBSS, SELECT_ON_OFF);
                        SetNodeTextData(curNode, 3, pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.m_slots[i].bSlotLOTS, SELECT_ON_OFF);
                        SetNodeTextData(curNode, 4, pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.m_slots[i].bSlotAllTx, SELECT_ON_OFF);
                        switch (pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.m_slots[i].nSlotRetranslation)
                        {
                            case 1:        text = tr("Partial");       break;
                            case 2:        text = tr("Complete");      break;
                            default:       text = tr("None");          break;
                        }
                        SetNodeTextData(curNode, 5, pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.m_slots[i].nSlotRetranslation, text);
                    }

                    SetNodeTextData(25, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.dwRes1);
                    SetNodeTextData(26, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.dwRes2);
                    SetNodeTextData(27, 0, pPacket->m_Ctrl.t0.body.interaceTerminalDiagRadio.dwRes3);
                    break;
                }

                case VCSS_TYPE_ULTRA_TERMINAL_EXTENSIONS:
                {
                switch (pPacket->m_Ctrl.t0.body.terminalExtensions.nExtensionType)
                {
                    case VCS_TERMINAL_EXTENSION_LISTEN:         text = tr("Listen");   break;
                    default:                                    text = tr("None");     break;
                }
                    SetNodeTextData(13, counter, pPacket->m_Ctrl.t0.body.terminalExtensions.nExtensionType, text);
                    for(quint8 i=0; i<VCSS_P_ULTRA_TERMINAL_EXTENSIONS_LISTEN_NUM; i++)
                    {
                        curNode = 14 + i;
                        SetNodeTextData(curNode, 0, pPacket->m_Ctrl.t0.body.terminalExtensions.body.terminalExtensionsListen.narListenId[i]);
                    }
//                    QByteArray bufferName;
//                    for(quint8 i=0; i<14; i++)
//                    {
//                        bufferName.append(pPacket->m_Ctrl.t0.body.terminalExtensions.body.terminalExtensionsListen.res[i]);
//                    }
//                    SetNodeTextData(20, counter, codec->toUnicode(bufferName));
                    break;
                }
                case VCSS_TYPE_ULTRA_DIVERT_GROUPS:
                {
                    SetNodeTextData(13, 0, pPacket->m_Ctrl.t0.body.divertGroups.nTotalAliases);
                    SetNodeTextData(14, 0, pPacket->m_Ctrl.t0.body.divertGroups.nDivertDst);
                    SetNodeTextData(15, 0, pPacket->m_Ctrl.t0.body.divertGroups.nRes);
                    for(quint8 i=0; i<VCSS_DIVERT_GROUP_SIZE; i++)
                    {
                        SetNodeTextData(16 + i, 0, pPacket->m_Ctrl.t0.body.divertGroups.Aliases[i]);
                    }
                    break;
                }
            }
            break;
        }
        case VCSS_TYPE_PHONE:
        {
            SetNodeTextData(12, 0, pPacket->m_Ctrl.t1.nLevel);
            SetNodeTextData(13, 0, pPacket->m_Ctrl.t1.nListenId);
            SetNodeTextData(14, 0, pPacket->m_Ctrl.t1.nGroupSign);
            SetNodeTextData(15, 0, pPacket->m_Ctrl.t1.nGroup);
            SetNodeTextData(16, 0, pPacket->m_Ctrl.t1.nDestId);
            QString text;
            switch (pPacket->m_Ctrl.t1.nCallType)
            {
                case VCSS_CALLTYPE_INDIRECT:    text = tr("Indirect"); break;
                case VCSS_CALLTYPE_DIRECT:      text = tr("Direct"); break;
                case VCSS_CALLTYPE_OPERATIVE:   text = tr("Operative"); break;
                default:                        text = tr("No call");
            }
            SetNodeTextData(17, 0, pPacket->m_Ctrl.t1.nCallType, text);
            SetNodeTextData(18, 0, pPacket->m_Ctrl.t1.bIntr, SELECT_ON_OFF);
            SetNodeTextData(19, 0, pPacket->m_Ctrl.t1.bHalfDuplex,
                SelectText(pPacket->m_Ctrl.t1.bHalfDuplex, tr("Half"), tr("Full")));
            switch (pPacket->m_Ctrl.t1.nConnType)
            {
                case VCSS_CONNTYPE_SIMPLE:      text = tr("Simple"); break;
                case VCSS_CONNTYPE_UNSELECT:    text = tr("Uselect"); break;
                case VCSS_CONNTYPE_CIRCULAR:    text = tr("Circular"); break;
                case VCSS_CONNTYPE_CONF_NEW:    text = tr("Conference"); break;
                case VCSS_CONNTYPE_DPC:         text = tr("DPC"); break;
                default:                        text = tr("No connection");
            }
            SetNodeTextData(20, 0, pPacket->m_Ctrl.t1.nConnType, text);
            switch (pPacket->m_Ctrl.t1.nCmd)
            {
                case VCSS_PCMD_NONE:            text = tr("Empty"); break;
                case VCSS_PCMD_OUTGOINGCALL:    text = tr("Outgoing call"); break;
                case VCSS_PCMD_INCOMINGCALL:    text = tr("Incomoing call"); break;
                case VCSS_PCMD_PICKCALL:        text = tr("Pick call"); break;
                case VCSS_PCMD_CONNECTED:       text = tr("Connected"); break;
                case VCSS_PCMD_HOLD:            text = tr("Hold"); break;
                case VCSS_PCMD_SUSPEND:         text = tr("Suspend"); break;
                case VCSS_PCMD_INTERCEPT:       text = tr("Intercept"); break;
                case VCSS_PCMD_TRANSFER:        text = tr("Transfer"); break;
                case VCSS_PCMD_DIVERT:          text = tr("Divert"); break;
                case VCSS_PCMD_EXT_DISCONNECT:  text = tr("External disconnect"); break;
            }
            SetNodeTextData(21, 0, pPacket->m_Ctrl.t1.nCmd, text);
            SetNodeTextData(22, 0, pPacket->m_Ctrl.t1.nPriority);
            SetNodeTextData(23, 0, pPacket->m_Ctrl.t1.bBusy, SELECT_ON_OFF);
            SetNodeTextData(24, 0, pPacket->m_Ctrl.t1.nRes3);
            QByteArray buff;
            buff.resize(sizeof(pPacket->m_Ctrl.t1.dwlENA));
            memmove(buff.data(), &(pPacket->m_Ctrl.t1.dwlENA), sizeof(pPacket->m_Ctrl.t1.dwlENA));
//            if(pPacket->m_Ctrl.t1.nDestId == 0)
//            if(pPacket->m_Ctrl.t1.nDestId != 0 && pPacket->m_Ctrl.t1.nPriority != 0 )
            if(pPacket->m_Ctrl.t1.nDestId != 0 && pPacket->m_Ctrl.t1.nPriority != 0 && pPacket->m_Ctrl.t1.nCmd != VCSS_PCMD_HOLD)
            {
                SetNodeTextData(25, 0, tr("(ENA)"), toENA(buff));
            }
            else
            {
                SetNodeTextData(25, 0, tr("(Name)"), TrimName(codec->toUnicode(buff)));
            }
            quint32 ip0 = pPacket->m_Ctrl.t1.nIP0,
                    ip1 = pPacket->m_Ctrl.t1.nIP1;
            SetNodeTextData(26, 0, tr("(IP)"), QHostAddress(swapByteOrder(ip0)).toString());
            SetNodeTextData(27, 0, tr("(IP)"), QHostAddress(swapByteOrder(ip1)).toString());
            Bits_quint16 oBits;
            oBits.nData = pPacket->m_Ctrl.t1.nHWstate;
            if(pPacket->m_Header.nHWversion == 0)
            {
                SetNodeTextData(28, 0, tr("(card1)"), SelectOnOff(oBits.bit1 + 1));
                SetNodeTextData(28, 1, tr("(card2)"), SelectOnOff(oBits.bit2 + 1));
                SetNodeTextData(28, 2, tr("(card3)"), SelectOnOff(oBits.bit3 + 1));
                SetNodeTextData(28, 3, tr("(card4)"), SelectOnOff(oBits.bit4 + 1));
                SetNodeTextData(28, 4, tr("(card5)"), SelectOnOff(oBits.bit5 + 1));
                SetNodeTextData(28, 5, tr("(card6)"), SelectOnOff(oBits.bit6 + 1));
                SetNodeTextData(28, 6, tr("(card7)"), SelectOnOff(oBits.bit7 + 1));
                SetNodeTextData(28, 7, tr("(card8)"), SelectOnOff(oBits.bit8 + 1));
                SetNodeTextData(28, 8, tr("(card9)"), SelectOnOff(oBits.bit9 + 1));
                SetNodeTextData(28, 9, tr("(card10)"), SelectOnOff(oBits.bit10 + 1));
                SetNodeTextData(28, 10, tr("(card11)"), SelectOnOff(oBits.bit11 + 1));
                SetNodeTextData(28, 11, tr("(card12)"), SelectOnOff(oBits.bit12 + 1));
                SetNodeTextData(28, 12, tr("(card13)"), SelectOnOff(oBits.bit13 + 1));
                SetNodeTextData(28, 13, tr("(card14)"), SelectOnOff(oBits.bit14 + 1));
                SetNodeTextData(28, 14, tr("(card15)"), SelectOnOff(oBits.bit15 + 1));
                SetNodeTextData(28, 15, tr("(card16)"), SelectOnOff(oBits.bit16 + 1));
            }
            else
            {
                SetNodeTextData(28, 0, tr("(ptt1)"), SelectOnOff(oBits.bit1));
                SetNodeTextData(28, 1, tr("(ptt2)"), SelectOnOff(oBits.bit2));
                SetNodeTextData(28, 2, tr("(ptt3)"), SelectOnOff(oBits.bit3));
                SetNodeTextData(28, 3, tr("(ptt4)"), SelectOnOff(oBits.bit4));
                SetNodeTextData(28, 4, tr("(ptt5)"), SelectOnOff(oBits.bit5));
                SetNodeTextData(28, 5, tr("(ptt6)"), SelectOnOff(oBits.bit6));
                SetNodeTextData(28, 6, tr("(ptt7)"), SelectOnOff(oBits.bit7));
                SetNodeTextData(28, 7, tr("(ptt8)"), SelectOnOff(oBits.bit8));
                SetNodeTextData(28, 8, tr("(head1)"), SelectOnOff(oBits.bit9));
                SetNodeTextData(28, 9, tr("(head2)"), SelectOnOff(oBits.bit10));
                SetNodeTextData(28, 10, tr("(hand)"), SelectOnOff(oBits.bit11));
                SetNodeTextData(28, 11, tr("(hook)"), SelectOnOff(oBits.bit12));
            }
            SetNodeTextData(29, 0, pPacket->m_Ctrl.t1.nBusy);
            SetNodeTextData(30, 0, pPacket->m_Ctrl.t1.bNeedEC, SELECT_YES_NO);
            SetNodeTextData(31, 0, pPacket->m_Ctrl.t1.bCallQueue, SELECT_YES_NO);
            SetNodeTextData(32, 0, pPacket->m_Ctrl.t1.bInDivert, SELECT_ON_OFF);
            SetNodeTextData(33, 0, pPacket->m_Ctrl.t1.nRes2);
            break;
        }
        case VCSS_TYPE_TERMINAL:
        {
            SetNodeTextData(12, 0, pPacket->m_Ctrl.t3.bPriority, SELECT_YES_NO);
            QString text;
            switch (pPacket->m_Ctrl.t3.nCmd)
            {
                case VCSS_RCMD_NONE:            text = tr("No command"); break;
                case VCSS_RCMD_FREQ:            text = tr("Set frequency"); break;
                case VCSS_RCMD_CHANNEL:         text = tr("Select channel"); break;
                case VCSS_RCMD_SELCAL:          text = tr("Send SELCAL"); break;
                case VCSS_RCMD_ICOM:            text = tr("Intercomm transmission"); break;
                case VCSS_RCMD_COUPLING:        text = tr("Coupling transmission"); break;
            }
            SetNodeTextData(13, 0, pPacket->m_Ctrl.t3.nCmd, text);
            SetNodeTextData(14, 0, pPacket->m_Ctrl.t3.nData);
            for(quint8 i=0; i<VCSS_MAX_RADIOS_ON_WP; i++)
            {
                curNode = 15 + i;
                SetNodeTextData(curNode, 0, pPacket->m_Ctrl.t3.Channels[i].nId);
                SetNodeTextData(curNode, 1, pPacket->m_Ctrl.t3.Channels[i].bTransmit, SELECT_YES_NO);
                switch (pPacket->m_Ctrl.t3.Channels[i].nRxMS)
                {
                    case 1:             text = tr("Main"); break;
                    case 2:             text = tr("Standby"); break;
                    case 3:             text = tr("Both"); break;
                    default:            text = tr("None");
                }
                SetNodeTextData(curNode, 2, pPacket->m_Ctrl.t3.Channels[i].nRxMS, text);
                SetNodeTextData(curNode, 3, pPacket->m_Ctrl.t3.Channels[i].bTxMS,
                                SelectText(pPacket->m_Ctrl.t3.Channels[i].bTxMS, tr("Main"), tr("Standby")));
                SetNodeTextData(curNode, 4, pPacket->m_Ctrl.t3.Channels[i].nCmd, SELECT_YES_NO);
                SetNodeTextData(curNode, 5, pPacket->m_Ctrl.t3.Channels[i].bRx, SELECT_YES_NO);
                SetNodeTextData(curNode, 6, pPacket->m_Ctrl.t3.Channels[i].bTx, SELECT_YES_NO);
                SetNodeTextData(curNode, 7, pPacket->m_Ctrl.t3.Channels[i].bEmergency, SELECT_YES_NO);
            }
            break;
        }
        case VCSS_TYPE_RADIO:
        {
            SetNodeTextData(12, 0, pPacket->m_Ctrl.t2.bRxEnable, SELECT_YES_NO);
            SetNodeTextData(13, 0, pPacket->m_Ctrl.t2.bTxEnable, SELECT_YES_NO);
            QString text;
            switch (pPacket->m_Ctrl.t2.nRxMS)
            {
                case 1:             text = tr("Main"); break;
                case 2:             text = tr("Standby"); break;
                case 3:             text = tr("Both"); break;
                default:            text = tr("None");
            }
            SetNodeTextData(14, 0, pPacket->m_Ctrl.t2.nRxMS, text);
            SetNodeTextData(15, 0, pPacket->m_Ctrl.t2.bTxMS,
                            SelectText(pPacket->m_Ctrl.t2.bTxMS, tr("Main"), tr("Standby")));
            SetNodeTextData(16, 0, pPacket->m_Ctrl.t2.bSquelsh, SELECT_YES_NO);
            SetNodeTextData(17, 0, pPacket->m_Ctrl.t2.nFreqInt);
            SetNodeTextData(18, 0, pPacket->m_Ctrl.t2.nFreqFract);
            SetNodeTextData(19, 0, pPacket->m_Ctrl.t2.nOwner);
            SetNodeTextData(20, 0, pPacket->m_Ctrl.t2.bDegradation, SELECT_ON_OFF);
            SetNodeTextData(21, 0, pPacket->m_Ctrl.t2.nRes0);
            switch (pPacket->m_Ctrl.t2.nCmd)
            {
                case VCSS_RCMD_NONE:            text = tr("No command"); break;
                case VCSS_RCMD_FREQ:            text = tr("Set frequency"); break;
                case VCSS_RCMD_CHANNEL:         text = tr("Select channel"); break;
                case VCSS_RCMD_SELCAL:          text = tr("Send SELCAL"); break;
                case VCSS_RCMD_ICOM:            text = tr("Intercomm transmission"); break;
                case VCSS_RCMD_COUPLING:        text = tr("Coupling transmission"); break;
            }
            SetNodeTextData(22, 0, pPacket->m_Ctrl.t2.nCmd, text);
            SetNodeTextData(23, 0, pPacket->m_Ctrl.t2.nLevel);
            SetNodeTextData(24, 0, pPacket->m_Ctrl.t2.nRes1);
            QByteArray bufferName;
            for(int i=0; i<8; i++)
            {
                bufferName.append(pPacket->m_Ctrl.t2.cName[i]);
            }
            quint32 ip0 = pPacket->m_Ctrl.t2.nIP0,
                    ip1 = pPacket->m_Ctrl.t2.nIP1;
            SetNodeTextData(25, 0, tr("(Name)"), TrimName(codec->toUnicode(bufferName)));
            SetNodeTextData(26, 0, tr("(IP)"), QHostAddress(swapByteOrder(ip0)).toString());
            SetNodeTextData(27, 0, tr("(IP)"), QHostAddress(swapByteOrder(ip1)).toString());
//            SetNodeTextData(28, 0, pPacket->m_Ctrl.t2.nHWstate);
            Bits_quint16 oBits;
            oBits.nData = pPacket->m_Ctrl.t2.nHWstate;
            if(pPacket->m_Header.nHWversion == 0)
            {
                SetNodeTextData(28, 0, tr("(card1)"), SelectOnOff(oBits.bit1 + 1));
                SetNodeTextData(28, 1, tr("(card2)"), SelectOnOff(oBits.bit2 + 1));
                SetNodeTextData(28, 2, tr("(card3)"), SelectOnOff(oBits.bit3 + 1));
                SetNodeTextData(28, 3, tr("(card4)"), SelectOnOff(oBits.bit4 + 1));
                SetNodeTextData(28, 4, tr("(card5)"), SelectOnOff(oBits.bit5 + 1));
                SetNodeTextData(28, 5, tr("(card6)"), SelectOnOff(oBits.bit6 + 1));
                SetNodeTextData(28, 6, tr("(card7)"), SelectOnOff(oBits.bit7 + 1));
                SetNodeTextData(28, 7, tr("(card8)"), SelectOnOff(oBits.bit8 + 1));
                SetNodeTextData(28, 8, tr("(card9)"), SelectOnOff(oBits.bit9 + 1));
                SetNodeTextData(28, 9, tr("(card10)"), SelectOnOff(oBits.bit10 + 1));
                SetNodeTextData(28, 10, tr("(card11)"), SelectOnOff(oBits.bit11 + 1));
                SetNodeTextData(28, 11, tr("(card12)"), SelectOnOff(oBits.bit12 + 1));
                SetNodeTextData(28, 12, tr("(card13)"), SelectOnOff(oBits.bit13 + 1));
                SetNodeTextData(28, 13, tr("(card14)"), SelectOnOff(oBits.bit14 + 1));
                SetNodeTextData(28, 14, tr("(card15)"), SelectOnOff(oBits.bit15 + 1));
                SetNodeTextData(28, 15, tr("(card16)"), SelectOnOff(oBits.bit16 + 1));
            }
            else
            {
                SetNodeTextData(28, 0, tr("(line)"), SelectOnOff(oBits.bit1 + 1));
                SetNodeTextData(28, 1, tr("(station)"), SelectOnOff(oBits.bit2 + 1));
            }
            SetNodeTextData(29, 0, pPacket->m_Ctrl.t2.nQOS);
            SetNodeTextData(30, 0, pPacket->m_Ctrl.t2.nRes2);
            break;
        }

    }

}

void MainWindow::AddNodes(const QStringList &items, int index)
{
    foreach (QString entry, items)
    {
        QStringList e;
                e << entry << "1" << "2";
        dataItems.append(new QTreeWidgetItem((QTreeWidget*)0, e));
        //dataItems.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(entry)));
        qDebug() << entry;
    }
//    items.clear();
    ui->twData->insertTopLevelItems(index, dataItems);
}


void MainWindow::AddNodes(CStringTable *items, int index)
{
    for(int i=0; i<items->count; i++)
    {
        QStringList e;
        e << items->FirstPart(i)<< "" << "" <<  items->SecondPart(i);
        dataItems.append(new QTreeWidgetItem((QTreeWidget*)0, e));
    }
    ui->twData->insertTopLevelItems(index, dataItems);
}

quint32 MainWindow::swapByteOrder(quint32 &ui)
{
    quint32 ret;
    ret = (ui >> 24) |
         ((ui<<8) & 0x00FF0000) |
         ((ui>>8) & 0x0000FF00) |
         (ui << 24);
    return ret;
}

void MainWindow::counterNext()
{
    colorUpdate();
    counter++;
    if(counter == DATA_COL) counter=0;
}

void MainWindow::colorUpdate()
{
    quint8 preCounter;
    if(counter == 0) preCounter = DATA_COL - 1; else preCounter = counter - 1;
    ResetHeaderMarker(preCounter, counter);
    for(int i=0; i < maxRow; i++)
    {
        //qDebug() <<"&&&"<<i << counter << ui->tblData->item(i,counter)->text()<<ui->tblData->item(i,counter)->statusTip();
        ui->tblData->item(i,preCounter)->setForeground(Qt::black);
        ui->tblData->item(i,counter)->setForeground(Qt::red);
        ui->tblData->item(i,counter)->setBackgroundColor(ColorMode(ui->tblData->item(i,counter)->statusTip(), ui->tblData->item(i,counter)->text()));
        ui->tblData->item(i,counter)->setStatusTip(ui->tblData->item(i,counter)->text());
    }
}

QColor MainWindow::ColorMode(const QString &oldText, const QString &newText)
{
    bool ok;
    long newValue = newText.toLong(&ok);
    if (!ok)
    {// not an integer
        if(oldText > newText)       return Qt::darkCyan;
        else if(oldText < newText)  return Qt::darkYellow;
    }
    else
    {// an integer
        long oldValue = oldText.toLong();
        if(oldValue > newValue)         return Qt::cyan;
        else if(oldValue < newValue)    return Qt::yellow;
    }
    return Qt::white;
}


void MainWindow::TypeSelected()
{
    for(int i=0; i < rbPacketTypes.count(); i++)
    {
        if(rbPacketTypes[i]->isChecked())
        {
            qDebug() << i << packetTypes[i].toLocal8Bit();
            if(i<3)
            {
                curType=i+1;
                curUltraType=0;
            }
            else
            {
                curType=0;
                curUltraType=i-2;
            }
//            qDebug() << "%%"<< curType << curUltraType << curId;
        }
    }
    SetTableHeaders();
    SetTreeNodes();
}

void MainWindow::SetTreeNodes()
{
    dataItems.clear();
    ui->twData->clear();
    ui->twData->setColumnWidth(0,140);
    ui->twData->setColumnWidth(1,140);
    ui->twData->setColumnWidth(2,140);

    // for headers
    CStringTable treeNodes;
    treeNodes.AddItem("Signature",   tr("M3 packet signature"));
    treeNodes.AddItem("bData",       tr("Data type"));
    treeNodes.AddItem("nType",       tr("Packet type"));
    treeNodes.AddItem("nId",         tr("Internal address"));
    treeNodes.AddItem("nSubId",      tr("Module"));
    treeNodes.AddItem("nDevId",      tr("Device"));
    treeNodes.AddItem("bAct",        tr("Activity state"));
    treeNodes.AddItem("bSignal",     tr("Useful signal"));
    treeNodes.AddItem("nHWversion",  tr("Hardware version"));
    treeNodes.AddItem("nRes",        tr("Reserve field"));
    treeNodes.AddItem("bOOS",        tr("Out of service"));
    treeNodes.AddItem("nCount",      tr("Packet counter"));
    const unsigned HEADER_NODES_COUNT = 12;
    AddNodes(&treeNodes);

    // add header subnode
    for(int i=1; i<4; i++)
    {
        AddItem(dataItems[0], "nSignature" + QString::number(i),"","",tr("Byte %1").arg(i));
    }

    // for body
    treeNodes.Clear();
    switch (curType)
    {
        case VCSS_TYPE_ULTRA:
        {
            treeNodes.AddItem("nUltraType",      tr("t0 packet ultra type"));
            switch(curUltraType)
            {
                case VCSS_TYPE_ULTRA_REC:
                {
                    treeNodes.AddItem("nIP0",                   tr("IP address LAN A"));
                    treeNodes.AddItem("nIP1",                   tr("IP address LAN B"));
                    treeNodes.AddItem("nChannelsNumber",        tr("Number of channels"));
                    QString s;
                    for(quint8 j=0; j<VCSS_P_ULTRA_RECORD_CHANNELS_NUM; j++)
                    {
                        s=QString::number(j);
                        s=s.rightJustified(2,'0')+':';
                        treeNodes.AddItem("Channel" + s,        tr("Channel %1").arg(j));
                    }
                    AddNodes(&treeNodes, HEADER_NODES_COUNT);
                    for(quint8 j=0; j<VCSS_P_ULTRA_RECORD_CHANNELS_NUM; j++)
                    {
                        AddItem(dataItems[16 + j], "nId", "", "",           tr("Internal address"));
                        AddItem(dataItems[16 + j], "nType", "", "",         tr("Sousce type"));
                        AddItem(dataItems[16 + j], "bRadioMain", "", "",    tr("Radio device"));
                        AddItem(dataItems[16 + j], "bRes", "", "",          tr("Reserve field"));
                    }
                    break;
                }
                case VCSS_TYPE_ULTRA_PERMANENT_DIAG:
                {
                    treeNodes.AddItem("nPermanentDiagType",     tr("Permanent diagnostics packet type"));
                    treeNodes.AddItem("dwLineStatus",           tr("Line status"));
                    treeNodes.AddItem("bInCAS",                 tr("Incoming CAS"));
                    treeNodes.AddItem("bOutCAS",                tr("Outcoming CAS"));
                    treeNodes.AddItem("bCannelNoReady",         tr("Channel no ready"));
                    treeNodes.AddItem("bRes2",                  tr("Reserve field"));
                    AddNodes(&treeNodes, HEADER_NODES_COUNT);
                    break;
                }
                case VCSS_TYPE_ULTRA_INTERFACE_TERMINAL_DIAG_COMMON:
                {
                    treeNodes.AddItem("micPhoneActive",         tr("Active microphone for phone"));
                    treeNodes.AddItem("micRadioActive",         tr("Active microphone for radio"));
                    treeNodes.AddItem("bHeadset1Present",       tr("Headset 1 present"));
                    treeNodes.AddItem("bHeadset2Present",       tr("Headset 2 present"));
                    treeNodes.AddItem("dwMasterVolPhone",       tr("Master volume for phone"));
                    treeNodes.AddItem("dwMasterVolRadio",       tr("Master volume for radio"));
                    treeNodes.AddItem("dwMechTangentaSlot",     tr("Main PTT attachment slot"));
                    treeNodes.AddItem("bExtraLoud",             tr("Extra loud state"));
                    treeNodes.AddItem("bSelfListen",            tr("Self listen statee"));
                    treeNodes.AddItem("bDefaultSettings",       tr("Default settings button pushed"));
                    treeNodes.AddItem("bWorkBtnPushed",         tr("Work button pushed"));
                    treeNodes.AddItem("bOperativeCall",         tr("Operative call state"));
                    treeNodes.AddItem("bFaceIsDying",           tr("Touchscreen software stopped"));
                    treeNodes.AddItem("dwVolHeadset1",          tr("Headset 1 volume"));
                    treeNodes.AddItem("dwVolHeadset2",          tr("Headset 2 volume"));
                    treeNodes.AddItem("dwVolHandset",           tr("Handset volume"));
                    treeNodes.AddItem("dwDefPhoneOutDev",       tr("Default phone output device"));
                    treeNodes.AddItem("dwDefPhoneVol",          tr("Default phone volume"));
                    QString s;
                    for(quint8 j=0; j<VCSS_P_ULTRA_TERM_DIAG_COMMON_ABONENTS_NUM; j++)
                    {
                        s=QString::number(j);
                        s=s.rightJustified(2,'0')+':';
                        treeNodes.AddItem("Abonent" + s,        tr("Phone settings for abonent %1").arg(j));
                    }
                    treeNodes.AddItem("dwDivert",               tr("Divert to internal address"));
                    treeNodes.AddItem("dwRingVol",              tr("Ring volume"));
                    treeNodes.AddItem("dwBrightness",           tr("Brightness level"));
                    treeNodes.AddItem("bRxAdd",                 tr("Rx add state"));
                    treeNodes.AddItem("bPTTBlocked",            tr("PTT blocked"));
                    treeNodes.AddItem("dwRes",                  tr("Reserve field"));
                    AddNodes(&treeNodes, HEADER_NODES_COUNT);

                    for(quint8 j=0; j<VCSS_P_ULTRA_TERM_DIAG_COMMON_ABONENTS_NUM; j++)
                    {
                        AddItem(dataItems[31 + j], "nId", "", "",           tr("Internal address"));
                        AddItem(dataItems[31 + j], "dwOutDev", "", "",      tr("Output device"));
                        AddItem(dataItems[31 + j], "dwRes1", "", "",        tr("Reserve field"));
                        AddItem(dataItems[31 + j], "dwVol", "", "",         tr("Volume"));
                        AddItem(dataItems[31 + j], "nRes2", "", "",         tr("Reserve field"));
                    }
                    break;
                }

                case VCSS_TYPE_ULTRA_INTERFACE_TERMINAL_DIAG_RADIO:
                {
                    QString s;
                    for(quint8 j=0; j<VCSS_MAX_RADIOS_ON_WP; j++)
                    {
                        s=QString::number(j);
                        s=s.rightJustified(2,'0')+':';
                        treeNodes.AddItem("Radio" + s,          tr("Radio settings for slot %1").arg(j));
                    }
                    treeNodes.AddItem("dwRes1",                 tr("Reserve field"));
                    treeNodes.AddItem("dwRes2",                 tr("Reserve field"));
                    treeNodes.AddItem("dwRes3",                 tr("Reserve field"));
                    AddNodes(&treeNodes, HEADER_NODES_COUNT);
                    for(quint8 j=0; j<VCSS_MAX_RADIOS_ON_WP; j++)
                    {
                        AddItem(dataItems[13 + j], "dwSlotVol", "", "",     tr("Volume"));
                        AddItem(dataItems[13 + j], "dwSlotOutDev", "", "",  tr("Output device"));
                        AddItem(dataItems[13 + j], "bSlotBSS", "", "",      tr("BSS mode"));
                        AddItem(dataItems[13 + j], "bSlotLOTS", "", "",     tr("LOTS mode"));
                        AddItem(dataItems[13 + j], "bSlotAllTx", "", "",    tr("ALLTx mode"));
                        AddItem(dataItems[13 + j], "nSlotRetranslation", "", "",      tr("Retranslation mode"));
                    }
                    break;
                }

                case VCSS_TYPE_ULTRA_TERMINAL_EXTENSIONS:
                {
                    treeNodes.AddItem("nExtensionType",         tr("Terminal extension packet type"));
                    QString s;
                    for(quint8 j=0; j<VCSS_P_ULTRA_TERMINAL_EXTENSIONS_LISTEN_NUM; j++)
                    {
                        s=QString::number(j);
                        treeNodes.AddItem("narListenId[" + s + "]",         tr("Listen abonent %1 internall addres").arg(j));
                    }
                    treeNodes.AddItem("res",                    tr("Reserve field"));
                    AddNodes(&treeNodes, HEADER_NODES_COUNT);
                    break;
                }
                case VCSS_TYPE_ULTRA_DIVERT_GROUPS:
                {
                    treeNodes.AddItem("nTotalAliases",          tr("Aliases total count"));
                    treeNodes.AddItem("nDivertDst",             tr("Divert destination abonent internal addres"));
                    treeNodes.AddItem("nRes",                   tr("Reserve field"));
                    QString s;
                    for(int j=0; j<VCSS_DIVERT_GROUP_SIZE; j++)
                    {
                        s=QString::number(j);
                        s=s.rightJustified(2,'0');
                        treeNodes.AddItem("Alias" + s,          tr("Alias %1").arg(j));
                    }
                    AddNodes(&treeNodes, HEADER_NODES_COUNT);
                    break;
                }
            }
            break;
        }
        case VCSS_TYPE_PHONE:
        {
            treeNodes.AddItem("nLevel",             tr("Graph level"));
            treeNodes.AddItem("nListenId",          tr("Listen abonent internal address"));
            treeNodes.AddItem("nGroupSign",         tr("Dynamic group member add/delete signal"));
            treeNodes.AddItem("nGroup",             tr("Dynamic group internal address"));
            treeNodes.AddItem("nDestId",            tr("Destination internal address"));
            treeNodes.AddItem("nCallType",          tr("Call type"));
            treeNodes.AddItem("bIntr",              tr("Intrusion state"));
            treeNodes.AddItem("bHalfDuplex",        tr("Duplex mode"));
            treeNodes.AddItem("nConnType",          tr("Connection type "));
            treeNodes.AddItem("nCmd",               tr("Command"));
            treeNodes.AddItem("nPriority",          tr("Priority level"));
            treeNodes.AddItem("bBusy",              tr("Busy state"));
            treeNodes.AddItem("nRes3",              tr("Reserve field"));
            treeNodes.AddItem("dwlENA",             tr("Name/ENA field"));
            treeNodes.AddItem("nIP0",               tr("IP address LAN A"));
            treeNodes.AddItem("nIP1",               tr("IP address LAN B"));
            treeNodes.AddItem("nHWstate",           tr("Hardware device state"));
            treeNodes.AddItem("nBusy",              tr("Busy signal destination internal addres"));
            treeNodes.AddItem("bNeedEC",            tr("Need echo canceling"));
            treeNodes.AddItem("bCallQueue",         tr("Use call queue"));
            treeNodes.AddItem("bInDivert",          tr("Divert state"));
            treeNodes.AddItem("nRes2",              tr("Reserve field"));
            AddNodes(&treeNodes, HEADER_NODES_COUNT);
            QString s;
            for(quint8 j=1; j<=16; j++)
            {
                s=QString::number(j);
                s=s.rightJustified(2,'0')+':';
                AddItem(dataItems[28], "Bit" + s, "", "",       tr("Bit %1").arg(j));
            }
            break;
        }
        case VCSS_TYPE_TERMINAL:
        {
            treeNodes.AddItem("bPriority",          tr("Radio priority"));
            treeNodes.AddItem("nCmd",               tr("Command"));
            treeNodes.AddItem("nData",              tr("Data for command"));
            QString s;
            for(quint8 j=0; j<VCSS_MAX_RADIOS_ON_WP; j++)
            {
                s=QString::number(j);
                s=s.rightJustified(2,'0')+':';
                treeNodes.AddItem("Slot" + s,       tr("Slot %1").arg(j));
            }
            AddNodes(&treeNodes, HEADER_NODES_COUNT);
            for(quint8 j=0; j<VCSS_MAX_RADIOS_ON_WP; j++)
            {
                AddItem(dataItems[15 + j], "nId", "", "",       tr("Radio internal address"));
                AddItem(dataItems[15 + j], "bTransmit", "", "", tr("Transmit keying"));
                AddItem(dataItems[15 + j], "nRxMS", "", "",     tr("Select receiver"));
                AddItem(dataItems[15 + j], "bTxMS", "", "",     tr("Select transmiter"));
                AddItem(dataItems[15 + j], "nCmd", "", "",      tr("Send command"));
                AddItem(dataItems[15 + j], "bRx", "", "",       tr("Receive state"));
                AddItem(dataItems[15 + j], "bTx", "", "",       tr("Transmit state"));
                AddItem(dataItems[15 + j], "bEmergency", "", "",tr("Emergency state"));
            }
            break;
        }
        case VCSS_TYPE_RADIO:
        {
            treeNodes.AddItem("bRxEnable",          tr("Receiver enabled"));
            treeNodes.AddItem("bTxEnable",          tr("Transmitter enabled"));
            treeNodes.AddItem("nRxMS",              tr("Receiver mode"));
            treeNodes.AddItem("bTxMS",              tr("Transmitter mode"));
            treeNodes.AddItem("bSquelsh",           tr("Squelsh"));
            treeNodes.AddItem("nFreqInt",           tr("Frequency integer part"));
            treeNodes.AddItem("nFreqFract",         tr("Frequency fractional part"));
            treeNodes.AddItem("nOwner",             tr("Ttransmitting abonent internal address"));
            treeNodes.AddItem("bDegradation",       tr("Degradation state"));
            treeNodes.AddItem("nRes0",              tr("Reserve field"));
            treeNodes.AddItem("nCmd",               tr("Command"));
            treeNodes.AddItem("nLevel",             tr("Graph level"));
            treeNodes.AddItem("nRes1",              tr("Reserve field"));
            treeNodes.AddItem("cName[8]",           tr("Name"));
            treeNodes.AddItem("nIP0",               tr("IP address LAN A"));
            treeNodes.AddItem("nIP1",               tr("IP address LAN B"));
            treeNodes.AddItem("nHWstate",           tr("Hardware device state"));
            treeNodes.AddItem("nQOS",               tr("Quality of signal"));
            treeNodes.AddItem("nRes2",              tr("Reserve field"));
            AddNodes(&treeNodes, HEADER_NODES_COUNT);
            QString s;
            for(quint8 j=1; j<=16; j++)
            {
                s=QString::number(j);
                s=s.rightJustified(2,'0')+':';
                AddItem(dataItems[28], "Bit" + s, "", "",       tr("Bit %1").arg(j));
            }
            break;
        }
    }
    ResetCurrentPacketCounter();
}


void MainWindow::SetTableHeaders()
{
    QList<QString> headerItems;
    int i=0;
    headerItems.append("nSignature1");
    headerItems.append("nSignature2");
    headerItems.append("nSignature3");
    headerItems.append("bData");
    headerItems.append("nType");
    headerItems.append("nId");
    headerItems.append("nSubId");
    headerItems.append("nDevId");
    headerItems.append("bAct");
    headerItems.append("bSignal");
    headerItems.append("nHWversion");
    headerItems.append("nRes");
    headerItems.append("bOOS");
    headerItems.append("nCount");
    foreach (QString entry, headerItems)
    {
        ui->tblData->verticalHeaderItem(i++)->setText(entry);
    }

    QList<QString> packetItems;

    switch (curType)
    {
        case VCSS_TYPE_ULTRA:
        {
            packetItems.append("nUltraType");
            switch(curUltraType)
            {
                case VCSS_TYPE_ULTRA_REC:
                {
                    packetItems.append("nIP0");
                    packetItems.append("nIP1");
                    packetItems.append("nChannelsNumber");
                    QString s;
                    for(quint8 j=0; j<VCSS_P_ULTRA_RECORD_CHANNELS_NUM; j++)
                    {
                        s=QString::number(j);
                        s=s.rightJustified(2,'0')+':';
                        packetItems.append(s + "nId");
                        packetItems.append(s + "nType");
                        packetItems.append(s + "bRadioMain");
                        packetItems.append(s + "bRes");
                    }
                    maxRow = 54;
                    break;
                }
                case VCSS_TYPE_ULTRA_PERMANENT_DIAG:
                {
                    packetItems.append("nPermanentDiagType");
                    packetItems.append("dwLineStatus");
                    packetItems.append("bInCAS");
                    packetItems.append("bOutCAS");
                    packetItems.append("bCannelNoReady");
                    packetItems.append("bRes2");

                    maxRow = 21;
                    break;
                }
                case VCSS_TYPE_ULTRA_INTERFACE_TERMINAL_DIAG_COMMON:
                {
                    packetItems.append("micPhoneActive");
                    packetItems.append("micRadioActive");
                    packetItems.append("bHeadset1Present");
                    packetItems.append("bHeadset2Present");
                    packetItems.append("dwMasterVolPhone");
                    packetItems.append("dwMasterVolRadio");
                    packetItems.append("dwMechTangentaSlot");
                    packetItems.append("bExtraLoud");
                    packetItems.append("bSelfListen");
                    packetItems.append("bDefaultSettings");
                    packetItems.append("bWorkBtnPushed");
                    packetItems.append("bOperativeCall");
                    packetItems.append("bFaceIsDying");
                    packetItems.append("dwVolHeadset1");
                    packetItems.append("dwVolHeadset2");
                    packetItems.append("dwVolHandset");
                    packetItems.append("dwDefPhoneOutDev");
                    packetItems.append("dwDefPhoneVol");
                    QString s;
                    for(quint8 j=0; j<VCSS_P_ULTRA_TERM_DIAG_COMMON_ABONENTS_NUM; j++)
                    {
                        s=QString::number(j);
                        s=s.rightJustified(2,'0')+':';
                        packetItems.append(s + "nId");
                        packetItems.append(s + "dwOutDev");
                        packetItems.append(s + "dwRes1");
                        packetItems.append(s + "dwVol");
                        packetItems.append(s + "nRes2");
                    }
                    packetItems.append("dwDivert");
                    packetItems.append("dwRingVol");
                    packetItems.append("dwBrightness");
                    packetItems.append("bRxAdd");
                    packetItems.append("bPTTBlocked");
                    packetItems.append("dwRes");
                    maxRow = 64;
                    break;
                }

                case VCSS_TYPE_ULTRA_INTERFACE_TERMINAL_DIAG_RADIO:
                {
                    QString s;
                    for(quint8 j=0; j<VCSS_MAX_RADIOS_ON_WP; j++)
                    {
                        s=QString::number(j);
                        s=s.rightJustified(2,'0')+':';
                        packetItems.append(s + "dwSlotVol");
                        packetItems.append(s + "dwSlotOutDev");
                        packetItems.append(s + "bSlotBSS");
                        packetItems.append(s + "bSlotLOTS");
                        packetItems.append(s + "bSlotAllTx");
                        packetItems.append(s + "nSlotRetranslation");
                    }
                    packetItems.append("dwRes1");
                    packetItems.append("dwRes2");
                    packetItems.append("dwRes3");
                    maxRow = 90;
                    break;
                }

                case VCSS_TYPE_ULTRA_TERMINAL_EXTENSIONS:
                {
                    packetItems.append("nExtensionType");
                    QString s;
                    for(quint8 j=0; j<VCSS_P_ULTRA_TERMINAL_EXTENSIONS_LISTEN_NUM; j++)
                    {
                        s=QString::number(j);
                        s=s.rightJustified(2,'0')+':';
                        packetItems.append(s + "narListenId");
                    }
                    packetItems.append("res");
                    maxRow = 23;
                    break;
                }
                case VCSS_TYPE_ULTRA_DIVERT_GROUPS:
                {
                packetItems.append("nTotalAliases");
                packetItems.append("nDivertDst");
                packetItems.append("nRes");
                QString s;
                for(int j=0; j<VCSS_DIVERT_GROUP_SIZE; j++)
                {
                    s=QString::number(j);
                    s=s.rightJustified(2,'0')+':';
                    packetItems.append(s + "Aliases");
                }
                maxRow = 30;
                break;
                }
            }
            break;
        }
        case VCSS_TYPE_PHONE:
        {
            packetItems.append("nLevel");
            packetItems.append("nListenId");
            packetItems.append("nGroupSign");
            packetItems.append("nGroup");
            packetItems.append("nDestId");
            packetItems.append("nCallType");
            packetItems.append("bIntr");
            packetItems.append("bHalfDuplex");
            packetItems.append("nConnType");
            packetItems.append("nCmd");
            packetItems.append("nPriority");
            packetItems.append("bBusy");
            packetItems.append("nRes3");
            packetItems.append("dwlENA");
            packetItems.append("nIP0");
            packetItems.append("nIP1");
            packetItems.append("nHWstate");
            packetItems.append("nBusy");
            packetItems.append("bNeedEC");
            packetItems.append("bCallQueue");
            packetItems.append("bInDivert");
            packetItems.append("nRes2");
            maxRow = 36;
            break;
        }
        case VCSS_TYPE_TERMINAL:
        {
            packetItems.append("bPriority");
            packetItems.append("nCmd");
            packetItems.append("nData");
            QString s;
            for(quint8 j=0; j<VCSS_MAX_RADIOS_ON_WP; j++)
            {
                s=QString::number(j);
                s=s.rightJustified(2,'0')+':';
                packetItems.append(s + "nId");
                packetItems.append(s + "bTransmit");
                packetItems.append(s + "nRxMS");
                packetItems.append(s + "bTxMS");
                packetItems.append(s + "nCmd");
                packetItems.append(s + "bRx");
                packetItems.append(s + "bTx");
                packetItems.append(s + "bEmergency");
            }
            maxRow = 113;
            break;
        }
        case VCSS_TYPE_RADIO:
        {
            packetItems.append("bRxEnable");
            packetItems.append("bTxEnable");
            packetItems.append("nRxMS");
            packetItems.append("bTxMS");
            packetItems.append("bSquelsh");
            packetItems.append("nFreqInt");
            packetItems.append("nFreqFract");
            packetItems.append("nOwner");
            packetItems.append("bDegradation");
            packetItems.append("nRes0");
            packetItems.append("nCmd");
            packetItems.append("nLevel");
            packetItems.append("nRes1");
            packetItems.append("cName[8]");
            packetItems.append("nIP0");
            packetItems.append("nIP1");
            packetItems.append("nHWstate");
            packetItems.append("nQOS");
            packetItems.append("nRes2");
            maxRow = 33;
            break;
        }
    }
    for(quint8 j=headerItems.count() + packetItems.count() - 1; j<DATA_ROW; j++)
    {
        ui->tblData->verticalHeaderItem(j)->setText(QString::number(j));
    }
    foreach (QString entry, packetItems)
    {
        ui->tblData->verticalHeaderItem(i++)->setText(entry);
    }
    ResetTableView();
    ResetCurrentPacketCounter();
}

void MainWindow::ResetHeaderMarker(int oldCol, int newCol)
{
    if(false)
    {
            ui->tblData->horizontalHeaderItem(oldCol)->setBackground(QBrush());
            ui->tblData->horizontalHeaderItem(oldCol)->setTextColor(Qt::black);

            ui->tblData->horizontalHeaderItem(newCol)->setBackgroundColor(Qt::gray);
            ui->tblData->horizontalHeaderItem(newCol)->setTextColor(Qt::red);
    }
}

void MainWindow::ResetTableView()
{
    for(int j=0; j<DATA_ROW; j++)
        for(int k=0; k<DATA_COL; k++)
        {
            SetCellTextData(j, k, "");
            ResetHeaderMarker(k, k);
            ui->tblData->item(j, k)->setBackgroundColor(Qt::white);
            ui->tblData->item(j, k)->setStatusTip("");
        }

    counter = 0;

}

void MainWindow::NetChanged()
{
//    qDebug()<<"NetChanged";
    ResetCurrentPacketCounter();
}

void MainWindow::ResetCurrentPacketCounter()
{
    for(quint8 i=0; i<NETS_COUNT; i++)
    {
        curPacketCounter[i] = 0;
    }
    startOfCount = QDateTime::currentDateTime();
}

void MainWindow::ResetNetPacketCounter()
{
    for(quint8 i=0; i<NETS_COUNT; i++)
    {
        for(quint8 j=0; j<10; j++)
            netPacketCounter[i].types[j] = 0;
        netPacketCounter[i].all = 0;
        netPacketCounter[i].unit = 0;
    }
}

void MainWindow::MoveCurrentPacketCounter(int net)
{
    curPacketCounter[net]++;
}

void MainWindow::MoveUnitPacketCounter(int net)
{
    netPacketCounter[net].unit++;
}

void MainWindow::MoveNetPacketCounter(int net, int type)
{
    netPacketCounter[net].types[type]++;
    netPacketCounter[net].all++;
}

QString MainWindow::GetCurrentCount(int net)
{
    QDateTime dt(QDateTime::currentDateTime());
    quint64 timeShift = dt.toMSecsSinceEpoch() - startOfCount.toMSecsSinceEpoch();
    float num = curPacketCounter[net]*1000;
    num = num  / timeShift;

    //


    return QString::number(num, 'f').replace(".", ",");


}

void MainWindow::FillHeaderForTable(CControlPacket2 *pPacket)
{
    SetCellTextData(0, counter, QChar((int)pPacket->m_Header.nSignature1));
    SetCellTextData(1, counter, QChar((int)pPacket->m_Header.nSignature2));
    SetCellTextData(2, counter, QChar((int)pPacket->m_Header.nSignature3));
    SetCellTextData(3, counter, pPacket->m_Header.bData);
    SetCellTextData(4, counter, pPacket->m_Header.nType);
    SetCellTextData(5, counter, pPacket->m_Header.nId);
    SetCellTextData(6, counter, pPacket->m_Header.nSubId);
    SetCellTextData(7, counter, pPacket->m_Header.nDevId);
    SetCellTextData(8, counter, pPacket->m_Header.bAct);
    SetCellTextData(9, counter, pPacket->m_Header.bSignal);
    SetCellTextData(10, counter, pPacket->m_Header.nHWversion);
    SetCellTextData(11, counter, pPacket->m_Header.nRes);
    SetCellTextData(12, counter, pPacket->m_Header.bOOS);
    SetCellTextData(13, counter, pPacket->m_Header.nCount);
}

void MainWindow::FillHeaderForTree(CControlPacket2 *pPacket)
{
    QString str;
    SetNodeTextData(0, 0, pPacket->m_Header.nSignature1, QChar((int)pPacket->m_Header.nSignature1));
    SetNodeTextData(0, 1, pPacket->m_Header.nSignature2, QChar((int)pPacket->m_Header.nSignature2));
    SetNodeTextData(0, 2, pPacket->m_Header.nSignature3, QChar((int)pPacket->m_Header.nSignature3));
    SetNodeTextData(1, 0, pPacket->m_Header.bData, SelectText(pPacket->m_Header.bData, tr("Voice"), tr("Control")));
    str = tr("t%1").arg(QString::number(pPacket->m_Header.nType));
    SetNodeTextData(2, 0, pPacket->m_Header.nType, str);
    SetNodeTextData(3, 0, pPacket->m_Header.nId);
    SetNodeTextData(4, 0, pPacket->m_Header.nSubId, SelectText(pPacket->m_Header.nSubId, tr("2nd"), tr("1st")));
    SetNodeTextData(5, 0, pPacket->m_Header.nDevId, SelectText(pPacket->m_Header.nDevId, tr("Main"), tr("Standby")));
    SetNodeTextData(6, 0, pPacket->m_Header.bAct, SelectText(pPacket->m_Header.bAct, tr("Active"), tr("Reserve")));
    SetNodeTextData(7, 0, pPacket->m_Header.bSignal, SelectText(pPacket->m_Header.bSignal, tr("On"), tr("Off")));
    SetNodeTextData(8, 0, pPacket->m_Header.nHWversion, SelectText(pPacket->m_Header.nHWversion, tr("Meg3"), tr("Meg2")));
    SetNodeTextData(9, 0, pPacket->m_Header.nRes);
    SetNodeTextData(10, 0, pPacket->m_Header.bOOS, SelectText(pPacket->m_Header.bOOS, tr("Yes"), tr("No")));
    SetNodeTextData(11, 0, pPacket->m_Header.nCount);
}

QString MainWindow::SelectText(const qint64 &data, const QString &trueText, const QString &falseText)
{
    QString str;
    (data == 1)? str = trueText: str = falseText;
    return str;
}

QString MainWindow::SelectOnOff(const qint64 &data)
{
    QString str;
    (data == 1)? str = tr("On"): str = tr("Off");
    return str;
}

QString MainWindow::SelectYesNo(const qint64 &data)
{
    QString str;
    (data == 1)? str = tr("Yes"): str = tr("No");
    return str;
}

void MainWindow::sleepFor(qint64 milliseconds)
{
    qint64 timeToExitFunction = QDateTime::currentMSecsSinceEpoch()+milliseconds;
    while(timeToExitFunction>QDateTime::currentMSecsSinceEpoch())
    {
        QApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

void MainWindow::RunButtonClicked()
{
    qDebug()<< "click_";
    QString text;
    QPixmap iStart(":/new/img/Start-icon.png");
    QPixmap iStop(":/new/img/Stop-icon.png");
    runStatus = !runStatus;
    if(runStatus)
    {
        text = tr("Stop");
        runTrigger->setIcon(QIcon(iStop));
        ResetCurrentPacketCounter();
    }
    else
    {
        text = tr("Start");
        runTrigger->setIcon(QIcon(iStart));
    }
    runTrigger->setText(text);
    runTrigger->setToolTip(text);
}

void MainWindow::CleanButtonClicked()
{
    qDebug()<< "click_";
    QString text;
    QPixmap cleanoff(":/new/img/Clean-off.png");
    QPixmap cleanon(":/new/img/Clean-on.png");
    cleanStatus = !cleanStatus;
    if(cleanStatus)
    {
        text = tr("Don't clean");
        cleanTrigger->setIcon(QIcon(cleanoff));
    }
    else
    {
        text = tr("Clean");
        cleanTrigger->setIcon(QIcon(cleanon));
    }
    cleanTrigger->setText(text);
    cleanTrigger->setToolTip(text);
}

void MainWindow::TreeButtonClicked()
{
    qDebug()<< "click_";
    QString text;
    QPixmap iTable(":/new/img/table-icon.png");
    QPixmap iTree(":/new/img/node-icon.png");
    treeStatus = !treeStatus;
    if(treeStatus)
    {
        text = tr("To table view");
        treeTrigger->setIcon(QIcon(iTable));
    }
    else
    {
        text = tr("To tree view");
        treeTrigger->setIcon(QIcon(iTree));
    }
    treeTrigger->setText(text);
    treeTrigger->setToolTip(text);
    //action
    ui->tblData->setVisible(!treeStatus);
    ui->tblData->setEnabled(!treeStatus);
    ui->twData->setVisible(treeStatus);
    ui->twData->setEnabled(treeStatus);
}

void MainWindow::BroadcastButtonClicked()
{
    qDebug()<< "click_";
    QString text;
    QPixmap iBroadcastOff(":/new/img/broadcast-g1.png");
    QPixmap iBroadcastOn(":/new/img/broadcast-y1.png");
    broadcastStatus = !broadcastStatus;
    if(broadcastStatus)
    {
        text = tr("Stop broadcasting");
        broadcastTrigger->setIcon(QIcon(iBroadcastOff));
    }
    else
    {
        text = tr("Start broadcasting");
        broadcastTrigger->setIcon(QIcon(iBroadcastOn));
    }
    broadcastTrigger->setText(text);
    broadcastTrigger->setToolTip(text);

}

void MainWindow::RePlayButtonClicked()
{
    qDebug()<< "click_";
    if(replayProcess) return;
    replayProcess = true;
}

void MainWindow::EndPlayButtonClicked()
{
    playProcess = false;
    EndPlaying();
}

void MainWindow::PlayFilterButtonClicked()
{
    playFilter->show();
}

void MainWindow::RecordButtonClicked()
{
    static bool ready;
    qDebug()<< "click_";
    QString text;
    QPixmap recordoff(":/new/img/ledon.png");
    QPixmap recordon(":/new/img/ledoff.png");
    QDir d;
    recordStatus = !recordStatus;
    if(recordStatus)
    {
        text = tr("Stop record");
        recordTrigger->setIcon(QIcon(recordoff));
        if(ready)
        {
            if(!d.exists(PATH_REC)) d.mkdir(PATH_REC);
            if(!d.exists(PATH_DATA)) d.mkdir(PATH_DATA);
            recA.Activate();
        }
        sbStatus->setText(tr("Recording"));
    }
    else
    {
        text = tr("Start record");
        recordTrigger->setIcon(QIcon(recordon));
        if(ready)
        {
            QMap<QDateTime, quint64> recordMap;
            qDebug()<< "record from A";
            QMap<QDateTime, quint64> *pMap = recA.GetTimeMap();
            QMap<QDateTime, quint64>::iterator it = pMap->begin();
            for(;it != pMap->end(); ++it)
            {
                recordMap.insert(it.key(), it.value());
                qDebug()<< it.key().toMSecsSinceEpoch() << " : " << it.value();
            }
            recA.ClearTimeMap();
            qDebug()<< "record from B";
            pMap = recB.GetTimeMap();
            it = pMap->begin();
            for(;it != pMap->end(); ++it)
            {
                recordMap.insert(it.key(), it.value());
                qDebug()<< it.key().toMSecsSinceEpoch() << " : " << it.value();
            }
            recB.ClearTimeMap();

            //qDebug()<< "record from all to file";
            CFileDialog dialog;
            QString filePath = dialog.SaveFileAsPrm(PATH_REC);

            if (!filePath.isEmpty())
            {
                QFileInfo fileInf(filePath);
                QString dataPath = fileInf.absoluteDir().absolutePath() + PATH_DATA_SUF;
                QFile file(filePath + PATH_EXT);
                if (file.open(QIODevice::WriteOnly | QIODevice::Text))
                {
                    QTextStream out(&file);
                    it = recordMap.begin();
                    for(;it != recordMap.end(); ++it)
                    {
                        out << it.key().toMSecsSinceEpoch() << "\t" << it.value() << "\n";
                    }
                    file.close();
                    QFileInfo defDir(QDir(PATH_DATA), "");
                     if((defDir.absoluteDir().absolutePath()+ PATH_DATA_SUF) != dataPath)
                    {
                         //if user set new dir to save prm-file: find and move all record data into new dir
                         if(!d.exists(dataPath)) d.mkdir(dataPath);

                         foreach (QString file, GetDataFiles(&recordMap))
                         {
                              if(QFile::exists(PATH_DATA + file))
                              {
                                  QFile::copy(PATH_DATA + file, dataPath + file);
                                  QFile::remove(PATH_DATA + file);
                              }
                         }
                    }
                }
            }
            else
            {
                //if no save prm-file: find and delete all record data
                foreach (QString file, GetDataFiles(&recordMap))
                {
                    if(QFile::exists(PATH_DATA + file)) QFile::remove(PATH_DATA + file);
                    //if(QFile::exists(dataPath + file)) QFile::remove(dataPath + file);
                }
            }
        }
        sbStatus->setText(tr("Online"));
    }
    ready = true;
    recordTrigger->setText(text);
    recordTrigger->setToolTip(text);
    sbRecordTime->setVisible(recordStatus);
}

void MainWindow::MenuFileDelete()
{
    CFileDialog dialog;
    QStringList files = dialog.DeletePrmFiles(PATH_REC);
    if(files.size() == 0) return;

    qDebug()<< "MenuFileDelete";
    foreach (QString filePath, files)
    {
        QMap<QDateTime, quint64> recordMap;
        QString str;
        QFileInfo fileInf(filePath);
        QString dataPath = fileInf.absoluteDir().absolutePath() + PATH_DATA_SUF;
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&file);
            while(!in.atEnd())
                {
                    str = in.readLine();
                    QDateTime dt;
                    quint64 c;
                    c = str.left(13).toLongLong();
                    dt = dt.fromMSecsSinceEpoch(c);
                    c = str.mid(14).toLongLong();
                    recordMap.insert(dt, c);
                }
            file.close();
            //remove data-files
            foreach (QString file, GetDataFiles(&recordMap))
            {
                //if(QFile::exists(PATH_DATA + file)) QFile::remove(PATH_DATA + file);
                if(QFile::exists(dataPath + file)) QFile::remove(dataPath + file);
            }
            //remove prm-file
            if(QFile::exists(filePath)) QFile::remove(filePath);
        }
    }
}

void MainWindow::MenuFileOpen()
{
    CFileDialog dialog;
    QString filePath = dialog.OpenPrmFile(PATH_REC);
    if(filePath.isEmpty()) return;


    qDebug()<< "MenuFileOpen" << filePath;
    //stop record if active
    if(recordStatus)      RecordButtonClicked();
    //disable record start
    recordTrigger->setEnabled(false);
    //set play mode
    playProcess = true;
    //clear source map
    ResetInterfaces();
    //load record data
    QMap<QDateTime, quint64> recordMap;
    GetMapFromFile(&recordMap, filePath);
    QStringList files = GetDataFiles(&recordMap);
    QFileInfo fileInf(filePath);
    QString dataPath = fileInf.absoluteDir().absolutePath() + PATH_DATA_SUF;
    //init player
    player.Open(&recordMap, &files, &dataPath);
    sbStatus->setText(tr("Playing"));
    sbPlayFile->setText(fileInf.fileName());
    sbPlayFile->setVisible(true);
    sbRecordTime->setVisible(true);
    sbPlayProcess->setMaximum(recordMap.size());
    sbPlayProcess->setVisible(true);
    sbTickTime->setVisible(true);
    endplayTrigger->setEnabled(true);
    replayTrigger->setEnabled(true);
    replayProcess = false;
    broadcastTrigger->setEnabled(true);
}


void MainWindow::MenuAboutProgramm()
{
    QMessageBox::about(this,this->windowTitle(),
                       tr("Megaphone3 control packet \nViewer & Recorder & Player \nby Vadim Martynov, NITA LLC"));
}

void MainWindow::GetMapFromFile(QMap<QDateTime, quint64> *pMap, const QString &filePath)
{
    QString str;
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        while(!in.atEnd())
            {
                str = in.readLine();
                QDateTime dt;
                quint64 c;
                c = str.left(13).toLongLong();
                dt = dt.fromMSecsSinceEpoch(c);
                c = str.mid(14).toLongLong();
                pMap->insert(dt, c);
            }
        file.close();
    }
}


QList<QString> MainWindow::GetDataFiles(QMap<QDateTime, quint64> *pMap)
{
    QList<QString> files;
    quint64 preTime = 0, preCount = 0;
    QMap<QDateTime, quint64>::iterator it = pMap->begin();
    for(;it != pMap->end(); ++it)
    {
        if(it.value() < preCount)
        {
            files << QString::number(preTime);
            qDebug()<< "add: " << preTime;
        }
        preCount = it.value();
        preTime = it.key().toMSecsSinceEpoch();
    }
    files << QString::number(preTime);
    qDebug()<< "add end: " << preTime;
    return files;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        RunButtonClicked();
        break;
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    static QDateTime olddt;
    if(event->key() == Qt::Key_Space)
    {
        QDateTime dt(QDateTime::currentDateTime());
        if((dt.toMSecsSinceEpoch() - olddt.toMSecsSinceEpoch()) > 1000)
        {
            RunButtonClicked();
            olddt=dt;
        }
    }
}


QString MainWindow::toENA(const QByteArray &data)
{
    QString ENA;
    for(qint8 i=7; i>=0; i--)
    {
        ENA += QString::number ((uchar) data[i], 16).rightJustified(2,'0');
    }
    ENA = ENA.mid(1) + ENA.left(1);
    ENA = ENA.left(ENA.indexOf('e'));
    ENA = ENA.replace('a', '*');
    ENA = ENA.replace('b', '#');
    ENA = ENA.replace('c', 'A');
    return ENA;
}

QString MainWindow::GetTerminalDev(int device)
{
    QString text;
    switch (device)
    {
        case DEV_MIC:           text = tr("Console mic"); break;
        case DEV_HANDSET:       text = tr("Handset"); break;
        case DEV_HEADSET_1:     text = tr("Headset1"); break;
        case DEV_HEADSET_2:     text = tr("Headset2"); break;
        case DEV_LS_1:          text = tr("Speaker1"); break;
        case DEV_LS_2:          text = tr("Speaker2"); break;
        case DEV_LS_3:          text = tr("Speaker3"); break;
        case DEV_LS_4:          text = tr("Speaker4"); break;
        default:                text = tr("None"); break;
    }
    return text;
}

QString MainWindow::TrimName(const QString &interfaceName)
{
    QString s = "";
    for(int i=0; i<interfaceName.size(); i++)
    {
        if(interfaceName[i].isPrint())  s += interfaceName[i];
    }
    return s;
}
