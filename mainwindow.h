#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

//------------------------------------------------------------
#include <QUdpSocket>
#include <QTimer>
#include <QMap>
#include <QTreeWidgetItem>
#include <QTextCodec>
#include <QRadioButton>
#include <QCheckBox>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QLabel>
#include <QProgressBar>
#include <QMessageBox>
#include <QTranslator>
#include "Const.h"
#include "cpacket.h"

#include "cactivesource.h"
#include "crecorder.h"
#include "cplayer.h"
#include "cfiledialog.h"
#include "cunitools.h"
#include "dialogplayfilter.h"
//------------------------------------------------------------

#define TITLE_MAIN          "M3PacketViewer v.0.0.10"
/*
* 0.0.10 add rule for ENA conversion
*
* release 2017-10-26

*/

#define TIMER_VIEW          2100
#define LOSS_THRESHOLD      3100

#define BASE_RADIO          10000
#define BASE_PHONE          20000
#define BASE_REC            30000

#define DATA_COL            30
#define DATA_ROW            120
#define INFO_ROW            4

#define PORT_CONTROL        10001
#define MC_TCMS             "234.16.0.1"

struct SCounters
{
    QVector<quint32> types;
    quint32 all;
    quint32 select;
    quint32 unit;
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0, QStringList args = QStringList());
    ~MainWindow();


public slots:
    void readReady0();
    void readReady1();
    void timerProcessing();
    void tickProcessing();
    void IdSelected();
    void TypeSelected();
    void SetTableHeaders();
    void SetTreeNodes();
    void RunButtonClicked();
    void CleanButtonClicked();
    void TreeButtonClicked();
    void RecordButtonClicked();
    void BroadcastButtonClicked();
    void RePlayButtonClicked();
    void EndPlayButtonClicked();
    void PlayFilterButtonClicked();
    void NetChanged();
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    void MenuFileOpen();
    void MenuFileDelete();
    void MenuAboutProgramm();

protected:
    QVector<quint32> curPacketCounter;
    QVector<SCounters> netPacketCounter;
    QDateTime startOfCount;

private:
    Ui::MainWindow *ui;

    QTreeWidgetItem* AddItem(const QString &name);
    //QTreeWidgetItem* AddItem(QTreeWidgetItem* item, const QString &itemName);
    QTreeWidgetItem* AddItem(QTreeWidgetItem* item, const QString &itemName, const QString &col_1 = "", const QString &col_2 = "", const QString &col_3 = "");
    void AddNodes(const QStringList &items, int index = 0);
    void AddNodes(CStringTable* items, int index = 0);
    void sleepFor(qint64 milliseconds);

    QVector<QHostAddress> hIP;
    QVector<QHostAddress> hIpBroadcast;
    QVector<QUdpSocket*> socket;
    CPacket megpacket;
    QMap<quint16, CActiveSource> sourceMap;
    QList<QString> nodeTypes;
    QList<QTreeWidgetItem *> sourceItems;
    QList<QTreeWidgetItem *> dataItems;

    QList<QString> packetTypes;
    QList<QRadioButton *> rbPacketTypes;
    QList<QString> netLabels;
    QList<QCheckBox *> chNets;
    QAction *runTrigger;
    QAction *cleanTrigger;
    QAction *treeTrigger;
    QAction *recordTrigger;
    QAction *endplayTrigger;
    QAction *replayTrigger;
    QAction *broadcastTrigger;
    QMenu *menuRun;
    QAction *playFilterTrigger;

    DialogPlayFilter *playFilter;

    QLabel *sbStatus;
    QLabel *sbRecordTime;
    QLabel *sbTickTime;
    QLabel *sbPlayFile;
    QProgressBar *sbPlayProcess;

    QNetworkInterface GetNetworkInterface(QHostAddress host);
    QHostAddress GetBroadcast(const QHostAddress &adr);
    QVector<QHostAddress> netList();

    quint16 curId;
    quint8 curMS;
    quint8 curType;
    quint8 curUltraType;
    quint8 counter;
    quint8 maxRow;
    bool runStatus;
    bool cleanStatus;
    bool treeStatus;
    bool recordStatus;
    bool broadcastStatus;
    bool replayProcess;
    bool playProcess;
    bool dontUpdateInerfacesFlag;
    CRecorder recA;
    CRecorder recB;
    CPlayer player;

    void SourceMapUpdate(CControlPacket2* pPacket, quint8 net, QHostAddress* pSender);
    void ReadPacket(CControlPacket2* pPacket, quint8 net);
    void CountPacket(CControlPacket2* pPacket, quint8 net);
    void SendPacket(const QByteArray &unit, quint8 net);
    void FillHeaderForTable(CControlPacket2* pPacket);
    void FillBodyForTable(CControlPacket2* pPacket);
    void FillHeaderForTree(CControlPacket2* pPacket);
    void FillBodyForTree(CControlPacket2* pPacket);
    bool ShowSelectInfo();
    void ShowPermanentInfo();
    void ResetOnTimer();
    void ReadSocket(quint8 net);
    void ResetSource();
    void ResetHeaderMarker(int oldCol, int newCol);
    void ReadPlayBuffer(QByteArray *pPlayData);
    void EndPlaying();

    quint8 GetType(quint16 key);
    void ResetInterfaces();
    void AddItemInterface(quint16 key);
    void DelItemInterface(quint16 key);
    void ColorItemInterface(quint16 key, bool status);
    void SetCellTextInfo(int row, int col, const QString &text);
    void SetCellTextInfoAll(int row, int col, const quint32 &data);
    void SetCellTextData(int row, int col, const QString &text);
    void SetCellTextData(int row, int col, const qint64 &data);
    void SetNodeTextData(int level_0, int level_1, const qint64 &data, const QString &text);
    void SetNodeTextData(int level_0, int level_1, const QString &data, const QString &text);
    void SetNodeTextData(int level_0, int level_1, const qint64 &data, int modeSelect);
    quint32 swapByteOrder(quint32 &ui);
    void counterNext();
    void colorUpdate();
    void ResetTableView();
    void ResetCurrentPacketCounter();
    void ResetNetPacketCounter();
    void MoveCurrentPacketCounter(int net);
    void MoveUnitPacketCounter(int net);
    void MoveNetPacketCounter(int net, int type);
    QString GetCurrentCount(int net);
    QString toENA(const QByteArray &data);
    QString TrimName(const QString &interfaceName);
    QString SelectText(const qint64 &data, const QString &trueText, const QString &falseText);
    QString SelectOnOff(const qint64 &data);
    QString SelectYesNo(const qint64 &data);
    QString GetTerminalDev(int device);
    QColor ColorMode(const QString &oldText, const QString &newText);
    void toBytes(QHostAddress *pHost, QByteArray *pBa);
    QList<QString> GetDataFiles(QMap<QDateTime, quint64> *pMap);
    void GetMapFromFile(QMap<QDateTime, quint64> *pMap, const QString &filePath);
};

#endif // MAINWINDOW_H
