#include <iostream>
#include <conio.h>
#include <windows.h>
#include <winbase.h>
#include <thread>
#include <list>
#include "TcAdsDef.h"
#include "TcAdsApi.h"



void startAdsConnection(std::ofstream* allOutfile);
void _stdcall Callback(AmsAddr*, AdsNotificationHeader*, unsigned long);
long fifoWrite(long val);
long fifoRead();
void writeValuesInPlc(long hWrite, AmsAddr* pAddr);
