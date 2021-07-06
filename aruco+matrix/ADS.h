#include <iostream>
#include <conio.h>
#include <windows.h>
#include <winbase.h>
#include <thread>
#include <list>
#include <mutex>
#include "TcAdsDef.h"
#include "TcAdsApi.h"



void mainADS(std::condition_variable* condVar, bool* end);
void __stdcall Callback(AmsAddr* pAddr, AdsNotificationHeader* pNotification, ULONG hA);
