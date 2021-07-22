#ifndef IUSB_H
#define IUSB_H

#include <QtCore/qglobal.h>

#if defined(IUSB_LIBRARY)
#  define IUSB_EXPORT Q_DECL_EXPORT
#else
#  define IUSB_EXPORT Q_DECL_IMPORT
#endif

#define MAX_PORT_CNT  181

struct IUSB_EXPORT PortPhoneInfo { 
    bool bConnected;

    // 层级
    char sLevel[32];

    char sVid[32];
    char sPid[32];
    char sSerial[64];  
};

typedef struct PortPhoneInfo *PPortPhoneInfo;

struct IUSB_EXPORT PortPhoneArray {
    int nCnt;
    PortPhoneInfo ppInfo[MAX_PORT_CNT];
};


class IUSB_EXPORT IUSB
{
public:
    IUSB();
    
    bool FindAllDevice(PortPhoneArray *ppArray);
    
private:
    void PPCopy(PortPhoneInfo& info, const PortPhoneInfo& rinfo);
};

#endif // IUSB_H
