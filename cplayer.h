#ifndef CPLAYER_H
#define CPLAYER_H

#include <QObject>
#include <QMap>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QStringList>

#include "crecorder.h"

class CPlayer
{
public:
    CPlayer();

    void Open(QMap<QDateTime, quint64> *pMap, QStringList *pFiles, QString *pDataPath);
    void ShowRecordMap();

    bool GetDataByTick(QByteArray *pData, QDateTime *pDateTime, bool *pRePlay);
    quint32 progress;

private:
//    QMap<QDateTime, quint64> *pRecMap;
//    QStringList *pRecFiles;
    QMap<QDateTime, quint64> recMap;
    QStringList recFiles;
    QString dataPath;
    QFile curFile;
    quint64 curFileCounter;
    quint64 curLine;
    QByteArray dataBuffer;
    bool newFlag;
    void ReadData();

};

#endif // CPLAYER_H
