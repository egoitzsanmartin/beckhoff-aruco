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

char szVar[] = { "MAIN.ROBOTVar" };
char szVarPc[] = { "MAIN.PCVar" };
long hUser, hWrite;

long buffer[LIMIT];
int tail = -1;
int head = 0;
int count = 0;
