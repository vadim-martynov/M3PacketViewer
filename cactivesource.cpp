#include "cactivesource.h"


CActiveSource::CActiveSource()
{
    id = 0;

    iType = 0;
    //sourceIp.resize(2);
    for(quint8 i=0; i<NETS_COUNT; i++)
    {
        nets.append(0);
        sourceIp.append(0);
        senderIp.append(0);
    }

    typeList.resize(TYPE_LIST_SIZE);
    view = false;
    offLine = true;
    ws = false;
}

void CActiveSource::Reset()
{
    for(quint8 i=0; i<NETS_COUNT; i++)
    {
        sourceIp[i] = 0;
        senderIp[i] = 0;
        nets[i] = 0;
    }
    for(quint8 i=0; i<TYPE_LIST_SIZE; i++)
    {
        typeList[i] = false;
    }
}


QString CActiveSource::GetIdText()
{
    QString text = QString::number(id);
    return text.rightJustified(4,' ');
}

QString CActiveSource::GetMsText()
{
    return QString::number(ms);
}

void CActiveSource::SetTime()
{
    QDateTime dt(QDateTime::currentDateTime());
    actionTime = dt;
}

bool CActiveSource::CheckTime()
{
    QDateTime dt(QDateTime::currentDateTime());
    return (dt.toMSecsSinceEpoch() - actionTime.toMSecsSinceEpoch() > 1000);
}

quint64 CActiveSource::GetTime()
{
    QDateTime dt(QDateTime::currentDateTime());
    return dt.toMSecsSinceEpoch() - actionTime.toMSecsSinceEpoch();
}

void CActiveSource::swapByteOrder(quint32 &ui)
{
    ui = (ui >> 24) |
         ((ui<<8) & 0x00FF0000) |
         ((ui>>8) & 0x0000FF00) |
         (ui << 24);
}

void CActiveSource::SetSourceIp(quint32 &ip0, quint32 &ip1)
{
    swapByteOrder(ip0);
    swapByteOrder(ip1);
    sourceIp[0]=ip0;
    sourceIp[1]=ip1;
}

void CActiveSource::TypeAnalysis(quint16 key)
{
    iType = I_EMPTY;

    if(typeList[1])
    {
        if(ws)
            iType = I_WS;
        else
        {
            if(typeList[3])
                iType = I_R_MAN;
            else
                iType = I_PHONE;
        }
    }

    if(typeList[2])
        iType = I_RADIO;

    if(typeList[0] && typeList[4])
        iType = I_REC;

    if(typeList[0] && typeList[7])
        iType = I_PHONE;

    if(iType == I_EMPTY)
    {
        iType = key/10000;
        switch (iType)
        {
        case 1: iType = I_RADIO; break;
        case 2: iType = I_PHONE; break;
        case 3: iType = I_REC; break;
        default:          break;
        }
    }
}
