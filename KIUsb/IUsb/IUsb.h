#ifndef _IUSB_H
#define _IUSB_H

#if defined(IUSB_EXPORTS)
#	if defined(_MSC_VER)
#		define IUSB_API __declspec(dllexport)
#	else
#		define UILIB_API
#	endif
#else
#	if defined(_MSC_VER)
#		define IUSB_API __declspec(dllimport)
#	else
#		define IUSB_API 
#	endif
#endif
#define IUSB_COMDAT __declspec(selectany)

#define MAX_PORT_CNT  181

typedef struct PortPhoneInfo
{ 
    bool bConnected;

    // ²ã¼¶
    char sLevel[32];

    char sVid[16];
    char sPid[16];
    char sSerial[64];  
}PortPhoneInfo, * PPortPhoneInfo;

struct PortPhoneArray {
    int nCnt;
    PortPhoneInfo ppInfo[MAX_PORT_CNT];
};

IUSB_API bool FindAllDevice(PortPhoneArray* ppArray);

#endif
