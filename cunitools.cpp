#include "cunitools.h"

//CUniTools::CUniTools()
//{
//}

CStringTable::CStringTable()
{
}


void CStringTable::AddItem(const QString &text1, const QString &text2)
{
    firstList << text1;
    secondList << text2;
    count = firstList.size();
}

void CStringTable::Clear()
{
    firstList.clear();
    secondList.clear();
    count = 0;
}

QString CStringTable::FirstPart(int index)
{
    return firstList[index];
}

QString CStringTable::SecondPart(int index)
{
    return secondList[index];
}
