#ifndef CPACKET_H
#define CPACKET_H

#include "Const.h"
#include "Packet2.h"
#include <QDebug>

class CPacket
{
public:
    CPacket();
    static void UnPackHeader(CPacketHeader* pHeader, const unsigned char* pBuff);

    static void UnPackControl(CControlPacket2* pPacket, const unsigned char* pBuff);

    //
    CPacketHeader UnPackHeaderD(const unsigned char* pBuff);


private:








};

#endif // CPACKET_H
