#include <iostream>
#include <conio.h>
#include <windows.h>
#include <winbase.h>
#include "C:\TwinCAT\AdsApi\TcAdsDll\Include\TcAdsDef.h"
#include "C:\TwinCAT\AdsApi\TcAdsDll\Include\TcAdsApi.h"

using namespace std;

void _stdcall Callback(AmsAddr*, AdsNotificationHeader*, unsigned long);

void main()
{ 
  long                   nErr, nPort; 
  AmsAddr                Addr; 
  PAmsAddr               pAddr = &Addr; 
  ULONG                  hNotification, hUser; 
  AdsNotificationAttrib  adsNotificationAttrib;
  char					 szVar []={"MAIN.PLCVar"};

  // open communication port on the ADS router
  nPort = AdsPortOpen();
  nErr = AdsGetLocalAddress(pAddr);
  if (nErr) cerr << "Error: AdsGetLocalAddress: " << nErr << '\n';
  pAddr->port = 851;

  // set the attributes of the notification
  adsNotificationAttrib.cbLength = 4;
  adsNotificationAttrib.nTransMode = ADSTRANS_SERVERONCHA;
  adsNotificationAttrib.nMaxDelay = 0;
  adsNotificationAttrib.nCycleTime = 10000000; // 1sec 

  // get handle
  nErr = AdsSyncReadWriteReq(pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(hUser), &hUser, sizeof(szVar), szVar);
  if (nErr) cerr << "Error: AdsSyncReadWriteReq: " << nErr << '\n';

  // initiate the transmission of the PLC-variable 
  nErr = AdsSyncAddDeviceNotificationReq(pAddr, ADSIGRP_SYM_VALBYHND, hUser, &adsNotificationAttrib, Callback, hUser, &hNotification);

  if (nErr) cerr << "Error: AdsSyncAddDeviceNotificationReq: " << nErr << '\n';
  cout << "Notification: " << hNotification << "\n\n";
  cout.flush();

  // wait for user intraction (keystroke)
  while (true) {
	  printf("Hello\n");
	  Sleep(500);
  }

  // finish the transmission of the PLC-variable 
  nErr = AdsSyncDelDeviceNotificationReq(pAddr, hNotification);
  if (nErr) cerr << "Error: AdsSyncDelDeviceNotificationReq: " << nErr << '\n';

  // release handle
  nErr = AdsSyncWriteReq(pAddr, ADSIGRP_SYM_RELEASEHND, 0, sizeof(hUser), &hUser); 
  if (nErr) cerr << "Error: AdsSyncWriteReq: " << nErr << '\n';

  // Close the communication port
  nErr = AdsPortClose();
  if (nErr) cerr << "Error: AdsPortClose: " << nErr << '\n';
}

// Callback-function
void __stdcall Callback(AmsAddr* pAddr, AdsNotificationHeader* pNotification, ULONG hUser)
{
  int                     nIndex; 
  static ULONG            nCount = 0; 
  SYSTEMTIME              SystemTime, LocalTime; 
  FILETIME                FileTime; 
  LARGE_INTEGER           LargeInteger; 
  TIME_ZONE_INFORMATION   TimeZoneInformation; 

  cout << ++nCount << ". Call:\n"; 

  // print (to screen)) the value of the variable 
  cout << "Value: " << *(ULONG *)pNotification->data << '\n'; 
  cout << "Notification: " << pNotification->hNotification << '\n';

  // Convert the timestamp into SYSTEMTIME
  LargeInteger.QuadPart = pNotification->nTimeStamp;
  FileTime.dwLowDateTime = (DWORD)LargeInteger.LowPart;
  FileTime.dwHighDateTime = (DWORD)LargeInteger.HighPart;
  FileTimeToSystemTime(&FileTime, &SystemTime);

  // Convert the time value Zeit to local time
  GetTimeZoneInformation(&TimeZoneInformation);
  SystemTimeToTzSpecificLocalTime(&TimeZoneInformation, &SystemTime, &LocalTime);

  // print out the timestamp
  cout << LocalTime.wHour << ":" << LocalTime.wMinute << ":" << LocalTime.wSecond << '.' << LocalTime.wMilliseconds << 
       " den: " << LocalTime.wDay << '.' << LocalTime.wMonth << '.' << LocalTime.wYear << '\n';

  // Größe des Buffers in Byte
  cout << "SampleSize: " << pNotification->cbSampleSize << '\n';
 
  // 32-Bit Variable (auch Zeiger), die beim AddNotification gesetzt wurde // (siehe main)
  cout << "hUser: " << hUser << '\n';

  // Print out the ADS-address of the sender
  cout << "ServerNetId: ";
  for (nIndex = 0; nIndex < 6; nIndex++)
    cout << (int)pAddr->netId.b[nIndex] << ".";
  cout << "\nPort: " << pAddr->port << "\n\n";
  cout.flush();
}
