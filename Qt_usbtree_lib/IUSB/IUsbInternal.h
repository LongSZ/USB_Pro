#ifndef _IUSBINTERNAL_H
#define _IUSBINTERNAL_H

#include <vector>
#include <string>
#include <windows.h>
#include <basetyps.h>
#include <stdio.h>
#include <tchar.h>
#include <devguid.h>
#include <initguid.h>
#include <iostream>
#include <devguid.h>
#include <InitGuid.h>
#include <SetupAPI.h>
#include <WinIoCtl.h>
#include <WinUser.h>

#include <basetyps.h>
#include <usb100.h>
#include <usbioctl.h>
#include <usbiodef.h>

using namespace std;

#pragma comment(lib, "User32.lib")

#include "iusb.h"

#define MAX_HCD 10

typedef struct _STRING_DESCRIPTOR_NODE
{
    struct _STRING_DESCRIPTOR_NODE* Next;
    UCHAR                           DescriptorIndex;
    USHORT                          LanguageID;
    USB_STRING_DESCRIPTOR           StringDescriptor[0];
} STRING_DESCRIPTOR_NODE, * PSTRING_DESCRIPTOR_NODE;

class CUsbInternal
{
public:
    CUsbInternal();
    ~CUsbInternal();
    
    bool GetPortUsbDevices(vector<PortPhoneInfo>& vecPH);
    
private:
    bool EnumerateHostControllers();

    string GetDriverKeyName(HANDLE Hub, ULONG ConnectionIndex);
    string GetHCDDriverKeyName(HANDLE HCD);
    string GetRootHubName(HANDLE HostController);

    bool GetStringDescriptor(HANDLE hHubDevice, ULONG ConnectionIndex, UCHAR DescriptorIndex, TCHAR* outBuff);
    string GetExternalHubName(HANDLE Hub, ULONG ConnectionIndex);
    void EnumerateHub(string HubName, string Msg,string sPortSerial);
    void EnumerateHubPorts(HANDLE hHubDevice, ULONG NumPorts, string sPortSerial);

    PSTRING_DESCRIPTOR_NODE GetStringDescriptorNode(
        HANDLE  hHubDevice,
        ULONG   ConnectionIndex,
        UCHAR   DescriptorIndex,
        USHORT  LanguageID);

    PSTRING_DESCRIPTOR_NODE GetStringDescriptorsNodeByLang(
            HANDLE  hHubDevice,
            ULONG   ConnectionIndex,
            UCHAR   DescriptorIndex,
            ULONG   NumLanguageIDs,
            USHORT* LanguageIDs,
            PSTRING_DESCRIPTOR_NODE StringDescNodeTail);
    
    std::vector<PortPhoneInfo>* m_pVecPH;
};

#endif
