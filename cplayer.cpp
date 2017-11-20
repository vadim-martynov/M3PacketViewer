#include "cplayer.h"

CPlayer::CPlayer()
{
    newFlag = false;
}


void CPlayer::Open(QMap<QDateTime, quint64> *pMap, QStringList *pFiles, QString *pDataPath)
{
    recMap = *pMap;
    recFiles = *pFiles;
    dataPath = *pDataPath;
    curFileCounter = 0;
    qDebug()<< "first file: " << recFiles.at(curFileCounter) << recFiles.size();
    //curFile.setFileName(PATH_DATA + recFiles.at(curFileCounter));
    curFile.setFileName(dataPath + recFiles.at(curFileCounter));
    ReadData();
    newFlag = true;
    progress = 0;
}

void CPlayer::ReadData()
{
    curFile.open(QIODevice::ReadOnly);
    dataBuffer = curFile.readAll();
    curFile.close();
}

void CPlayer::ShowRecordMap()
{
    QMap<QDateTime, quint64>::iterator it = recMap.begin();
    for(;it != recMap.end(); ++it)
    {
            qDebug()<< "item: " << it.key();
    }

}

bool CPlayer::GetDataByTick(QByteArray *pData, QDateTime *pDateTime, bool *pRePlay)
{
    static quint64 packetsFrom = 0;
    static QMap<QDateTime, quint64>::iterator it = recMap.begin();
    if(newFlag)
    {
        packetsFrom = 0;
        it = recMap.begin();
        newFlag = false;
    }
    if(it != recMap.end())
    {
        if(*pRePlay)
        {
            qDebug()<< "REPLAY__" << *pRePlay;
            quint64 preTime = 0, preCount = 0, reCounter = 0;
            quint64 progressR = 0;
            QMap<QDateTime, quint64>::iterator itR = recMap.begin();
            for(;itR != recMap.end(); ++itR)
            {
                if(itR.value() < preCount)
                {
                    reCounter++;
                    //qDebug()<< "REPLAY file: "<< reCounter << QString::number(preTime);
                }
                if(itR.key().toMSecsSinceEpoch() + REPLAY_TIME > it.key().toMSecsSinceEpoch())
                {
                    qDebug()<< "replay item: " << reCounter << itR.key().toMSecsSinceEpoch() << it.key().toMSecsSinceEpoch();
                    if(reCounter < curFileCounter)
                    {
                        curFileCounter = reCounter;
                        //curFile.setFileName(PATH_DATA + recFiles.at(curFileCounter));
                        curFile.setFileName(dataPath + recFiles.at(curFileCounter));
                        ReadData();
                    }
                    it = itR;
                    progress = progressR;
                    packetsFrom = preCount;

                    break;
                }
                preCount = itR.value();
                preTime = itR.key().toMSecsSinceEpoch();
                progressR++;
            }
            *pRePlay = false;
        }
        qint64 capacity = it.value()- packetsFrom;
        //qDebug()<< "packet count per tick" << capacity << it.value() << packetsFrom << curFileCounter;
        pData->clear();
        pData->append(dataBuffer.mid(packetsFrom *(PACKET_BLOCK_SIZE), capacity *(PACKET_BLOCK_SIZE)));
        pDateTime->setMSecsSinceEpoch(it.key().toMSecsSinceEpoch());
        //
        packetsFrom = it.value();
        //qDebug()<< "<>" << QString::number(it.key().toMSecsSinceEpoch())<<  pRecFiles->at(curFileCounter);
        //progress=int(it - recMap.begin());
        if(QString::number(it.key().toMSecsSinceEpoch()) == recFiles.at(curFileCounter))
        {
            curFileCounter++;
            if(curFileCounter < (uint)recFiles.size())
            {
                qDebug()<< "next file: " << recFiles.at(curFileCounter) << curFileCounter << recFiles.size();
                //curFile.setFileName(PATH_DATA + recFiles.at(curFileCounter));
                curFile.setFileName(dataPath + recFiles.at(curFileCounter));
                ReadData();
                packetsFrom = 0;
            }
        }
        it++;
        progress++;
        return true;
    }
    else
        packetsFrom = 0;
        return false;
}
