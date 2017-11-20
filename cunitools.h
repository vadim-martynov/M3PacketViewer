#ifndef CUNITOOLS_H
#define CUNITOOLS_H

#include <QObject>
#include <QStringList>

//class CUniTools
//{
//public:
//    CUniTools();
//};


class CStringTable
{
public:
    CStringTable();

    void AddItem(const QString &text1, const QString &text2);
    QString FirstPart(int index);
    QString SecondPart(int index);
    void Clear();
    quint16 count;

private:
    QStringList firstList;
    QStringList secondList;
};


union Bits_quint16
{
   struct
    {
     quint8 bit1: 1;
     quint8 bit2: 1;
     quint8 bit3: 1;
     quint8 bit4: 1;
     quint8 bit5: 1;
     quint8 bit6: 1;
     quint8 bit7: 1;
     quint8 bit8: 1;
     quint8 bit9: 1;
     quint8 bit10: 1;
     quint8 bit11: 1;
     quint8 bit12: 1;
     quint8 bit13: 1;
     quint8 bit14: 1;
     quint8 bit15: 1;
     quint8 bit16: 1;
    };
   quint16 nData;
} ;

enum SelectModes
{
    SELECT_ON_OFF,
    SELECT_YES_NO,
    SELECT_DEVICE,
    SELECT_VOL_R,
    SELECT_VOL_D
} ;

enum TerminalDevices
{
    DEV_MIC             = 1,
    DEV_HANDSET,
    DEV_HEADSET_1,
    DEV_HEADSET_2,
    DEV_LS_1,
    DEV_LS_2,
    DEV_LS_3,
    DEV_LS_4,
} ;







#endif // CUNITOOLS_H
