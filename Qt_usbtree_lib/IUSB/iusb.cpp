#include "iusb.h"
#include "IUsbInternal.h"

IUSB::IUSB()
{
}

bool IUSB::FindAllDevice(PortPhoneArray *ppArray)
{
    CUsbInternal uii;
    vector<PortPhoneInfo> vecPhone;
    
    if (NULL == ppArray)
        return false;
    
    uii.GetPortUsbDevices(vecPhone);
    
     if (vecPhone.empty())
        return false;
    
     ppArray->nCnt = vecPhone.size();
      
     int i = 0;
     vector<PortPhoneInfo>::iterator it = vecPhone.begin();
     while (it != vecPhone.end() && i < MAX_PORT_CNT)
     {
        //cout << "Node:" << it->sLevel << "Name:"  << it->sVid<< it->sPid << "SN: " << it->sSerial<< endl;
        PPCopy(ppArray->ppInfo[i] , *it);
        ++i;
        ++it;
     }
    
     return i > 0;
}

void IUSB::PPCopy(PortPhoneInfo &info, const PortPhoneInfo &rinfo)
{
    info.bConnected = rinfo.bConnected;
   
    if (rinfo.sLevel[0] != 0)
        strcpy_s(info.sLevel, rinfo.sLevel);

    if (rinfo.sPid[0] != 0)
        strcpy_s(info.sPid, rinfo.sPid);

    if (rinfo.sVid[0] != 0)
        strcpy_s(info.sVid, rinfo.sVid);

    if (rinfo.sSerial[0] != 0)
        strcpy_s(info.sSerial, rinfo.sSerial);
}
