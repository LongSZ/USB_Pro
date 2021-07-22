#include "IUsbInternal.h"

#define HUB_VENDOR 0x1A40
#define HUB_PRODUCT 0x0201

#define ALLOC(dwBytes) GlobalAlloc(GPTR,(dwBytes))
#define FREE(hMem)  GlobalFree((hMem))

wstring A2TT(const string& src)
{
    int  wcsLen = ::MultiByteToWideChar(CP_ACP, NULL, src.c_str(), src.length(), NULL, 0);
    wchar_t* wszString = new  wchar_t[wcsLen + 1];
    ::MultiByteToWideChar(CP_ACP, NULL, src.c_str(), src.length(), wszString, wcsLen);
    wszString[wcsLen] = '\0';

    wstring sRet = wszString;
    delete[] wszString;

    return sRet;
}

string T2AA(const wstring& src)
{
    int  nLen = ::WideCharToMultiByte(CP_ACP, NULL, src.c_str(), src.length(), NULL, 0, NULL, 0);
    char* szString = new  char[nLen + 1];
    ::WideCharToMultiByte(CP_ACP, NULL, src.c_str(), src.length(), szString, nLen, NULL, 0);
    szString[nLen] = '\0';

    string sRet = szString;
    delete[] szString;

    return sRet;
}

wstring UA2TT(const string& src)
{
    int  wcsLen = ::MultiByteToWideChar(CP_UTF8, NULL, src.c_str(), src.length(), NULL, 0);
    wchar_t* wszString = new  wchar_t[wcsLen + 1];
    ::MultiByteToWideChar(CP_UTF8, NULL, src.c_str(), src.length(), wszString, wcsLen);
    wszString[wcsLen] = '\0';

    wstring sRet = wszString;
    delete[] wszString;

    return sRet;
}

string UT2AA(const wstring& src)
{
    int  nLen = ::WideCharToMultiByte(CP_UTF8, NULL, src.c_str(), src.length(), NULL, 0, NULL, 0);
    char* szString = new  char[nLen + 1];
    ::WideCharToMultiByte(CP_UTF8, NULL, src.c_str(), src.length(), szString, nLen, NULL, 0);
    szString[nLen] = '\0';

    string sRet = szString;
    delete[] szString;

    return sRet;
}

void CUsbInternal::EnumerateHubPorts(HANDLE hHubDevice,ULONG NumPorts, string sPortSerial)
{
    ULONG       index;
    BOOL        success;
    PUSB_NODE_CONNECTION_INFORMATION_EX connectionInfoEx;

    //list ports of root hub
    unsigned int port;
    port=NumPorts;
    for (index=1; index <= port; index++)
    {
        ULONG nBytes;
        nBytes = sizeof(USB_NODE_CONNECTION_INFORMATION_EX) + sizeof(USB_PIPE_INFO) * 30;
        connectionInfoEx = (PUSB_NODE_CONNECTION_INFORMATION_EX)ALLOC(nBytes);
        if (connectionInfoEx == NULL)
            goto end;

        connectionInfoEx->ConnectionIndex = index;
        success = DeviceIoControl(hHubDevice, IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX, connectionInfoEx, nBytes, connectionInfoEx, nBytes, &nBytes, NULL);
        if (!success)
        {
            PUSB_NODE_CONNECTION_INFORMATION    connectionInfo;
            ULONG                               nBytes;

            // Try using IOCTL_USB_GET_NODE_CONNECTION_INFORMATION
            // instead of IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX
            //
            nBytes = sizeof(USB_NODE_CONNECTION_INFORMATION) +
                sizeof(USB_PIPE_INFO) * 30;

            connectionInfo = (PUSB_NODE_CONNECTION_INFORMATION)ALLOC(nBytes);
            connectionInfo->ConnectionIndex = index;

            success = DeviceIoControl(hHubDevice,
                IOCTL_USB_GET_NODE_CONNECTION_INFORMATION,
                connectionInfo,
                nBytes,
                connectionInfo,
                nBytes,
                &nBytes,
                NULL);

            if (!success)
            {
                FREE(connectionInfo);
                FREE(connectionInfoEx);
                continue;
            }

            // Copy IOCTL_USB_GET_NODE_CONNECTION_INFORMATION into
            // IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX structure.
            //
            connectionInfoEx->ConnectionIndex =
                connectionInfo->ConnectionIndex;

            connectionInfoEx->DeviceDescriptor =
                connectionInfo->DeviceDescriptor;

            connectionInfoEx->CurrentConfigurationValue =
                connectionInfo->CurrentConfigurationValue;

            connectionInfoEx->Speed =
                connectionInfo->LowSpeed ? UsbLowSpeed : UsbFullSpeed;

            connectionInfoEx->DeviceIsHub =
                connectionInfo->DeviceIsHub;

            connectionInfoEx->DeviceAddress =
                connectionInfo->DeviceAddress;

            connectionInfoEx->NumberOfOpenPipes =
                connectionInfo->NumberOfOpenPipes;

            connectionInfoEx->ConnectionStatus =
                connectionInfo->ConnectionStatus;

            memcpy(&connectionInfoEx->PipeList[0],
                &connectionInfo->PipeList[0],
                sizeof(USB_PIPE_INFO) * 30);

            FREE(connectionInfo);
        }

        if (connectionInfoEx->ConnectionStatus == DeviceConnected)
        {
            if (connectionInfoEx->DeviceIsHub)
            {
                string extHubName;
                extHubName = GetExternalHubName(hHubDevice,index);
                //cout << "ExternalHubName:" << extHubName.c_str() << "\n";

                if (extHubName.size() != string::npos) {
                    if (connectionInfoEx->DeviceDescriptor.idVendor == HUB_VENDOR &&
                        connectionInfoEx->DeviceDescriptor.idProduct == HUB_PRODUCT)
                    {
                        EnumerateHub(extHubName, " - External Hub", /*sPortSerial + "|" + */std::to_string(index));
                    }
                    else
                    {
                        if (sPortSerial.empty())
                            EnumerateHub(extHubName, " - External Hub", ""/*sPortSerial + "|" + std::to_string(index)*/);
                        else
                            EnumerateHub(extHubName, " - External Hub", sPortSerial + "|" + std::to_string(index));
                    }
                }
            }

            if (connectionInfoEx->DeviceDescriptor.bDeviceClass == 0
                && connectionInfoEx->DeviceDescriptor.bDeviceSubClass == 0
                && !sPortSerial.empty())
            {
                UCHAR nProduct = connectionInfoEx->DeviceDescriptor.iProduct;
                UCHAR nManuf = connectionInfoEx->DeviceDescriptor.iManufacturer;
                UCHAR nSerial = connectionInfoEx->DeviceDescriptor.iSerialNumber;    

                if (nProduct && nManuf /*&& nSerial*/)
                {
                    TCHAR OutBuffPro[128] = {0};
                    TCHAR OutBuffMan[128] = {0};
                    TCHAR OutBuffSerial[128] = {0};
                    GetStringDescriptor(hHubDevice, connectionInfoEx->ConnectionIndex, nProduct, OutBuffPro);
                    GetStringDescriptor(hHubDevice, connectionInfoEx->ConnectionIndex, nManuf, OutBuffMan);
                    GetStringDescriptor(hHubDevice, connectionInfoEx->ConnectionIndex, nSerial, OutBuffSerial);

//                     TCHAR OutBuffNumCfg[10240] = { 0 };
//                     UCHAR nNumCfg = connectionInfoEx->DeviceDescriptor.bNumConfigurations;
//                     GetStringDescriptor(hHubDevice, connectionInfoEx->ConnectionIndex, nNumCfg, OutBuffNumCfg);

                    PortPhoneInfo info;

                    ZeroMemory(&info, sizeof(info));
                    strcpy_s(info.sPid, T2AA(OutBuffPro).c_str());
                    strcpy_s(info.sVid, T2AA(OutBuffMan).c_str());
                    strcpy_s(info.sSerial, T2AA(OutBuffSerial).c_str());
                    info.bConnected = true;

                    string s = sPortSerial + "|" + std::to_string(index);
                    strcpy_s(info.sLevel, s.c_str());

                    if (m_pVecPH)
                        m_pVecPH->push_back(info);

                    //cout <<"ParentPorts:" << info.sLevel <<": Connected  " << info.sPid <<" SerialNumber:" << info.sSerial << "\n";
;
                    OutputDebugString(OutBuffMan);
                    OutputDebugString(OutBuffPro);
                    OutputDebugString(OutBuffSerial);
                }
            }    
        }
        else if (connectionInfoEx->ConnectionStatus == NoDeviceConnected && !sPortSerial.empty())
        {
            PortPhoneInfo info;

            ZeroMemory(&info, sizeof(info));
            info.bConnected = false;
            string s = sPortSerial + "|" + std::to_string(index);
            strcpy_s(info.sLevel, s.c_str());

            if (m_pVecPH)
                m_pVecPH->push_back(info);

            //cout << "ParentPorts:" << info.sLevel << ": NotConnected" << "\n";
        }
    }

end:    
    CloseHandle(hHubDevice);
}

void CUsbInternal::EnumerateHub(string rootHubName, string Msg, string sPortSerial)
{
    BOOL    success;
    HANDLE hHubDevice;
    string deviceName;;
    ULONG nBytes;
    PUSB_NODE_INFORMATION HubInfo;

    HubInfo = (PUSB_NODE_INFORMATION)malloc(sizeof(USB_NODE_INFORMATION));

    {
        deviceName += "\\\\.\\";
        deviceName += rootHubName;

        hHubDevice = CreateFileA(deviceName.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (hHubDevice == INVALID_HANDLE_VALUE)
        {
            return;
        }

        success = DeviceIoControl(hHubDevice, IOCTL_USB_GET_NODE_INFORMATION, HubInfo, sizeof(USB_NODE_INFORMATION), HubInfo, sizeof(USB_NODE_INFORMATION), &nBytes, NULL);
        if (!success)
            return;
    }

    // noew emuerate all ports 
    EnumerateHubPorts(hHubDevice, HubInfo->u.HubInformation.HubDescriptor.bNumberOfPorts,sPortSerial/*nParentPort*/);
}


string CUsbInternal::GetExternalHubName(HANDLE Hub,ULONG ConnectionIndex)
{
    BOOL                        success;
    ULONG                       nBytes;
    USB_NODE_CONNECTION_NAME    extHubName;
    PUSB_NODE_CONNECTION_NAME   extHubNameW;
    string                      extHubNameA;

    extHubNameW = NULL;

    extHubName.ConnectionIndex = ConnectionIndex;
    success = DeviceIoControl(Hub, IOCTL_USB_GET_NODE_CONNECTION_NAME, &extHubName, sizeof(extHubName), &extHubName, sizeof(extHubName), &nBytes,NULL);
    if (!success)
        goto GetExternalHubNameError;


    nBytes = extHubName.ActualLength;
    if (nBytes <= sizeof(extHubName))
        goto GetExternalHubNameError;

    extHubNameW=(PUSB_NODE_CONNECTION_NAME)GlobalAlloc(GPTR,nBytes);
    if (extHubNameW == NULL)
        goto GetExternalHubNameError;

    extHubNameW->ConnectionIndex = ConnectionIndex;
    success = DeviceIoControl(Hub, IOCTL_USB_GET_NODE_CONNECTION_NAME, extHubNameW, nBytes, extHubNameW, nBytes, &nBytes, NULL);
    if (!success)
        goto GetExternalHubNameError;


    extHubNameA = T2AA(extHubNameW->NodeName);

    GlobalFree(extHubNameW);

    return extHubNameA;

GetExternalHubNameError:
    if (extHubNameW != NULL)
    {
        GlobalFree(extHubNameW);
        extHubNameW = NULL;
    }

    return NULL;
}

string CUsbInternal::GetDriverKeyName(HANDLE Hub, ULONG ConnectionIndex)
{
    BOOL                                success;
    ULONG                               nBytes;
    USB_NODE_CONNECTION_DRIVERKEY_NAME  driverKeyName;
    PUSB_NODE_CONNECTION_DRIVERKEY_NAME driverKeyNameW;
    string                              driverKeyNameA;

    driverKeyNameW = NULL;

    driverKeyName.ConnectionIndex = ConnectionIndex;
    success = DeviceIoControl(Hub, IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME, &driverKeyName, sizeof(driverKeyName),  &driverKeyName, sizeof(driverKeyName), &nBytes, NULL);
    if (!success)
    {
        goto GetDriverKeyNameError;
    }

    nBytes = driverKeyName.ActualLength;
    if (nBytes <= sizeof(driverKeyName))
    {
        goto GetDriverKeyNameError;
    }

    driverKeyNameW = (PUSB_NODE_CONNECTION_DRIVERKEY_NAME)malloc(nBytes);
    if (driverKeyNameW == NULL)
    {
        goto GetDriverKeyNameError;
    }

    driverKeyNameW->ConnectionIndex = ConnectionIndex;

    success = DeviceIoControl(Hub, IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME, driverKeyNameW, nBytes, driverKeyNameW, nBytes, &nBytes, NULL);
    if (!success)
    {
        goto GetDriverKeyNameError;
    }
    driverKeyNameA = T2AA(driverKeyNameW->DriverKeyName);
    free(driverKeyNameW);

    return driverKeyNameA;

GetDriverKeyNameError:
    if (driverKeyNameW != NULL)
    {
        free(driverKeyNameW);
        driverKeyNameW = NULL;
    }

    return NULL;
}

string CUsbInternal::GetRootHubName(HANDLE HostController)
{
    BOOL                success;
    ULONG               nBytes;
    USB_ROOT_HUB_NAME   rootHubName;
    PUSB_ROOT_HUB_NAME  rootHubNameW;
    string              rootHubNameA;

    rootHubNameW = NULL;

    success = DeviceIoControl(HostController, IOCTL_USB_GET_ROOT_HUB_NAME, 0, 0, &rootHubName, sizeof(rootHubName), &nBytes, NULL);
    if (!success)
    {
        goto GetRootHubNameError;
    }

    nBytes = rootHubName.ActualLength;

    rootHubNameW =(PUSB_ROOT_HUB_NAME) malloc(nBytes);
    if (rootHubNameW == NULL)
    {
        goto GetRootHubNameError;
    }

    success = DeviceIoControl(HostController, IOCTL_USB_GET_ROOT_HUB_NAME, NULL, 0, rootHubNameW, nBytes, &nBytes, NULL);
    if (!success)
    {
        goto GetRootHubNameError;
    }

    rootHubNameA = T2AA(rootHubNameW->RootHubName);

    free(rootHubNameW);

    return rootHubNameA;

GetRootHubNameError:
    if (rootHubNameW != NULL)
    {
        free(rootHubNameW);
        rootHubNameW = NULL;
    }

    return NULL;
}

string CUsbInternal::GetHCDDriverKeyName(HANDLE HCD)
{
    BOOL                    success;
    ULONG                   nBytes;
    USB_HCD_DRIVERKEY_NAME  driverKeyName;
    PUSB_HCD_DRIVERKEY_NAME driverKeyNameW;
    string                  driverKeyNameA;

    driverKeyNameW = NULL;

    success = DeviceIoControl(HCD, IOCTL_GET_HCD_DRIVERKEY_NAME, &driverKeyName, sizeof(driverKeyName), &driverKeyName, sizeof(driverKeyName), &nBytes, NULL);
    if (!success)
    {
        goto GetHCDDriverKeyNameError;
    }

    nBytes = driverKeyName.ActualLength;
    if (nBytes <= sizeof(driverKeyName))
    {
        goto GetHCDDriverKeyNameError;
    }

    driverKeyNameW =(PUSB_HCD_DRIVERKEY_NAME) ALLOC(nBytes);

    if (driverKeyNameW == NULL)
    {
        goto GetHCDDriverKeyNameError;
    }

    success = DeviceIoControl(HCD, IOCTL_GET_HCD_DRIVERKEY_NAME, driverKeyNameW, nBytes, driverKeyNameW, nBytes, &nBytes, NULL);
    if (!success)
    {
        goto GetHCDDriverKeyNameError;
    }

    driverKeyNameA = T2AA(driverKeyNameW->DriverKeyName);
    FREE(driverKeyNameW);

    return driverKeyNameA;

GetHCDDriverKeyNameError:
    if (driverKeyNameW != NULL)
    {
        FREE(driverKeyNameW);
        driverKeyNameW = NULL;
    }

    return NULL;
}

PSTRING_DESCRIPTOR_NODE CUsbInternal::GetStringDescriptorNode(
    HANDLE  hHubDevice,
    ULONG   ConnectionIndex,
    UCHAR   DescriptorIndex,
    USHORT  LanguageID)
{
    BOOL    success;
    ULONG   nBytes;
    ULONG   nBytesReturned;

    UCHAR   stringDescReqBuf[sizeof(USB_DESCRIPTOR_REQUEST) +
        MAXIMUM_USB_STRING_LENGTH];

    PUSB_DESCRIPTOR_REQUEST stringDescReq;
    PUSB_STRING_DESCRIPTOR  stringDesc;
    PSTRING_DESCRIPTOR_NODE stringDescNode;

    nBytes = sizeof(stringDescReqBuf);

    stringDescReq = (PUSB_DESCRIPTOR_REQUEST)stringDescReqBuf;
    stringDesc = (PUSB_STRING_DESCRIPTOR)(stringDescReq+1);

    // Zero fill the entire request structure
    //
    memset(stringDescReq, 0, nBytes);

    // Indicate the port from which the descriptor will be requested
    //
    stringDescReq->ConnectionIndex = ConnectionIndex;

    //
    // USBHUB uses URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE to process this
    // IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION request.
    //
    // USBD will automatically initialize these fields:
    //     bmRequest = 0x80
    //     bRequest  = 0x06
    //
    // We must inititialize these fields:
    //     wValue    = Descriptor Type (high) and Descriptor Index (low byte)
    //     wIndex    = Zero (or Language ID for String Descriptors)
    //     wLength   = Length of descriptor buffer
    //
    stringDescReq->SetupPacket.wValue = (USB_STRING_DESCRIPTOR_TYPE << 8)
        | DescriptorIndex;

    stringDescReq->SetupPacket.wIndex = LanguageID;

    stringDescReq->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));

    // Now issue the get descriptor request.
    //
    success = DeviceIoControl(hHubDevice,
        IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
        stringDescReq,
        nBytes,
        stringDescReq,
        nBytes,
        &nBytesReturned,
        NULL);

    //
    // Do some sanity checks on the return from the get descriptor request.
    //

    if (!success)
    {
        return NULL;
    }

    if (nBytesReturned < 2)
    {
        return NULL;
    }

    if (stringDesc->bDescriptorType != USB_STRING_DESCRIPTOR_TYPE)
    {
        return NULL;
    }

    if (stringDesc->bLength != nBytesReturned - sizeof(USB_DESCRIPTOR_REQUEST))
    {
        return NULL;
    }

    if (stringDesc->bLength % 2 != 0)
    {
        return NULL;
    }

    //
    // Looks good, allocate some (zero filled) space for the string descriptor
    // node and copy the string descriptor to it.
    //

    stringDescNode = (PSTRING_DESCRIPTOR_NODE)ALLOC(sizeof(STRING_DESCRIPTOR_NODE) +
        stringDesc->bLength);

    if (stringDescNode == NULL)
    {
        return NULL;
    }

    stringDescNode->DescriptorIndex = DescriptorIndex;
    stringDescNode->LanguageID = LanguageID;

    memcpy(stringDescNode->StringDescriptor,
        stringDesc,
        stringDesc->bLength);

    return stringDescNode;
}

PSTRING_DESCRIPTOR_NODE CUsbInternal::GetStringDescriptorsNodeByLang (
    HANDLE  hHubDevice,
    ULONG   ConnectionIndex,
    UCHAR   DescriptorIndex,
    ULONG   NumLanguageIDs,
    USHORT  *LanguageIDs,
    PSTRING_DESCRIPTOR_NODE StringDescNodeTail)
{
    ULONG i;

    for (i=0; i<NumLanguageIDs; i++)
    {
        StringDescNodeTail->Next = GetStringDescriptorNode(hHubDevice,
            ConnectionIndex,
            DescriptorIndex,
            *LanguageIDs);

        if (StringDescNodeTail->Next)
        {
            StringDescNodeTail = StringDescNodeTail->Next;
        }

        LanguageIDs++;
    }

    return StringDescNodeTail;
}

bool CUsbInternal::GetStringDescriptor (HANDLE hHubDevice, ULONG ConnectionIndex,UCHAR DescriptorIndex, TCHAR *outBuff)
{
    PSTRING_DESCRIPTOR_NODE supportedLanguagesString;
    PSTRING_DESCRIPTOR_NODE stringDescNodeTail;
    ULONG                   numLanguageIDs;
    USHORT                  *languageIDs;

    supportedLanguagesString = GetStringDescriptorNode(hHubDevice,
        ConnectionIndex,
        0,
        0);

    if (supportedLanguagesString == NULL)
        return false;

    numLanguageIDs = (supportedLanguagesString->StringDescriptor->bLength - 2) / 2;
    languageIDs = (USHORT*)&supportedLanguagesString->StringDescriptor->bString[0];    
    stringDescNodeTail = supportedLanguagesString;

    if (DescriptorIndex)
    {
        stringDescNodeTail = GetStringDescriptorsNodeByLang(hHubDevice,
            ConnectionIndex,
            DescriptorIndex,
            numLanguageIDs,
            languageIDs,
            stringDescNodeTail);

        wcscpy(outBuff,stringDescNodeTail->StringDescriptor->bString);
        return true;
    }

    return false;
}

CUsbInternal::CUsbInternal()
{
    m_pVecPH = NULL;
}

CUsbInternal::~CUsbInternal()
{
    m_pVecPH = NULL;
}

bool CUsbInternal::GetPortUsbDevices(vector<PortPhoneInfo>& vecPH)
{
    m_pVecPH = &vecPH;
    return EnumerateHostControllers();
}

bool CUsbInternal::EnumerateHostControllers()
{
    WCHAR       HCName[16];
    HANDLE      hHCDev;
    int         HCNum;
    string      driverKeyName;
    string       rootHubName;

    for (HCNum = 0; HCNum < MAX_HCD; HCNum++)
    {
        wsprintf(HCName, _T("\\\\.\\HCD%d"), HCNum);
        hHCDev = CreateFile(HCName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (hHCDev == INVALID_HANDLE_VALUE)
        {
            //return false;
            continue;
        }

        driverKeyName = (char*)GetHCDDriverKeyName(hHCDev).c_str();
        if ((int)driverKeyName.length() > 0)
        {
            rootHubName=GetRootHubName(hHCDev).c_str();
            if((int)rootHubName.size() > 0)
            {
                cout << "NodeName:" << rootHubName.c_str() << "\n";
                //EnumerateHub(rootHubName, "Root Hub",std::to_string(HCNum));
                EnumerateHub(rootHubName, "Root Hub","");
            }
            else
            {
                CloseHandle(hHCDev);
                return false;
            }
        }
        else
        {
            CloseHandle(hHCDev);
            return false;
        }

        CloseHandle(hHCDev);
    }

    return true;
}
