#ifndef CRECORDER_H
#define CRECORDER_H

#include <QObject>
#include <QMap>
#include <QDateTime>
#include <QFile>

#include <QDebug>

#define PATH_REC            "./records"
#define PATH_DATA           "./records/data/"
#define PATH_DATA_SUF       "/data/"
#define PATH_EXT            ".prm"
#define TIMER_TICK          128
#define BLOCK_SIZE          50000
#define PACKET_BLOCK_SIZE   41
#define REPLAY_TIME         5000
//const quint64 BLOCK_SIZE = 50000;



class CRecorder
{
public:
    CRecorder();



    void Activate();
    //void AddData(const QByteArray &data);
    void AddData(const QByteArray &data, const QByteArray &sender, QChar net);
    bool AddTime();
    void ShowRecState();
    void WriteData();
    QMap<QDateTime, quint64> *GetTimeMap();
    void ClearTimeMap();
    quint64 GetMapSize();


private:
    QByteArray buffer;
    quint64 counter;
    bool active;
    QMap<QDateTime, quint64> timeMap;

};

#endif // CRECORDER_H
