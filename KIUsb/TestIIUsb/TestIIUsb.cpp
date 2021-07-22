// TestIIUsb.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <windows.h>
#include <iostream>
#include "../IUsb/IUsb.h"

#ifdef DEBUG
#pragma comment(lib,"../IUsb/Debug/IUsb.lib")
#else
#pragma comment(lib,"../IUsb/Release/IUsb.lib")
#endif

int main()
{
//     HMODULE h = LoadLibraryW(L"C:\\Users\\admin\\Downloads\\IUsb\\Release\\IUsb.dll");
//     if (!h)
//     {
//         std::cout << "Load Library IUSB.dll Failed!\n";
//         return 0;
//     }
//     std::cout << "Hello World!\n";
// 
//     FuncDevInfo = (IsDevExists)GetProcAddress(h, "IsDevExists");
//     if (!FuncDevInfo)
//     {
//         std::cout << "Load IsDevExists Function Failed!\n";
//         FreeModule(h);
//         return 0;
//     }
// 
//     FuncDevInfo(L"123456789");

//    IsDevExists(L"123456789");

    PortPhoneArray ppArray;
    ZeroMemory(&ppArray, sizeof(ppArray));
    if (FindAllDevice(&ppArray))
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

    getchar();

    return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
