#include <QCoreApplication>

#include "iusb.h"

#include <Windows.h>
#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    IUSB usb;
    PortPhoneArray ppArray;
    ZeroMemory(&ppArray, sizeof(ppArray));
    if (usb.FindAllDevice(&ppArray))
    {
        for (int i = 0; i < ppArray.nCnt; ++i)
        {
            if (ppArray.ppInfo[i].bConnected)
            {
                std::cout << "Level:" << ppArray.ppInfo[i].sLevel << " Vid:"
                    << ppArray.ppInfo[i].sVid << " Pid:" << ppArray.ppInfo[i].sPid << " SN:" << ppArray.ppInfo[i].sSerial << std::endl;
            }
            else {
                std::cout << "Level:" << ppArray.ppInfo[i].sLevel << " No USB Device Connected!" << std::endl;
            }
        }
    }
    
    return a.exec();
}
