#include "crecorder.h"

CRecorder::CRecorder()
{

    active = false;
    counter = 0;
    buffer.clear();


}

void CRecorder::Activate()
{
    active = true;
}


void CRecorder::AddData(const QByteArray &data, const QByteArray &sender, QChar net)
{
    if(active)
    {
        buffer.append(net);
        buffer.append(sender);
        buffer.append(data);
        counter++;
    }
}

bool CRecorder::AddTime()
{
    bool ret = false;
    if(active)
    {
        QDateTime dt(QDateTime::currentDateTime());
        timeMap.insert(dt, counter);
        if(counter > BLOCK_SIZE) ret = true;
    }
    //qDebug()<<"--"<< n << counter << ret;
    return ret;
}

quint64 CRecorder::GetMapSize()
{
    return timeMap.count();
}

void CRecorder::ShowRecState()
{
    //qDebug()<<"Rec: "<< counter << timeMap.size() << timeMap.lastKey().toMSecsSinceEpoch();
}

void CRecorder::WriteData()
{
    active = false;
#if QT_VERSION >= QT_VERSION_CHECK(5,2,0)
    QString fileName = QString::number(timeMap.lastKey().toMSecsSinceEpoch());
#else
    QList<QDateTime> list = timeMap.keys();
    QString fileName = QString::number(list.last().toMSecsSinceEpoch());
#endif
    QFile f(PATH_DATA + fileName);
       f.open(QIODevice::WriteOnly);
       f.write(buffer);
       f.close();

    qDebug()<< "file: " << fileName << buffer.size()/41 <<counter <<timeMap.size();
    buffer.clear();
    counter = 0;
}

QMap<QDateTime, quint64> *CRecorder::GetTimeMap()
{
    if(active)  WriteData();
    return &timeMap;
}

void CRecorder::ClearTimeMap()
{
    timeMap.clear();
}

