#ifndef CACTIVESOURCE_H
#define CACTIVESOURCE_H

#include <QObject>
#include <QDateTime>
#include <QVector>
#include <QDebug>

#define NETS_COUNT      2
#define TYPE_LIST_SIZE  10

enum
{
    I_WS,
    I_PHONE,
    I_R_MAN,
    I_RADIO,
    I_REC,
    I_EMPTY
};

class CActiveSource
{
public:
    CActiveSource();

    void SetTime();
    quint64 GetTime();
    bool CheckTime();
    void Reset();
    void SetSourceIp(quint32 &ip0, quint32 &ip1);
    QString GetIdText();
    QString GetMsText();
    void TypeAnalysis(quint16 key);

    quint16 id;
    quint8 ms;

    quint8 iType;
    bool view;
    bool offLine;
    bool ws;
    QVector<quint32> sourceIp;
    QVector<quint32> senderIp;
    QVector<bool> typeList;
    QVector<quint8> nets;
    QByteArray byteName;

private:
    QDateTime actionTime;
    void swapByteOrder(quint32 &ui);
};

#endif // CACTIVESOURCE_H
