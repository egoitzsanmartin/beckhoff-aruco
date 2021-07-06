#include "ADS.h"
#include "WriteFiles.h"

using namespace std::chrono;

char szVar1[] = { "MAIN.Var1" };
long hVar1;
bool nData = false;

long anterior = NULL;
bool* endProgram;
bool started = false;

std::condition_variable* cVar;

// Callback-function
void __stdcall Callback(AmsAddr* pAddr, AdsNotificationHeader* pNotification, ULONG hA)
{
	nData = pNotification->data[0];
	//std::cout << nData << std::endl;
	//nData += 1;
	if (nData == true) {
		cVar->notify_all();
		started = true;
	}
	else if(nData == false && started == true) {
		*endProgram = true;
	}

	unsigned long time = pNotification->nTimeStamp / 10000; //Convertir de 100ns a ms

	//unsigned long time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

	//std::cout << nData << std::endl;

	if (anterior != NULL) {
		std::cout << "miliseconds since last message: " << time - anterior << std::endl;
	}

	anterior = time;

}

void mainADS(std::condition_variable* condVar, bool* end)
{

	long                   nErr, nPort;
	AmsAddr                Addr;
	PAmsAddr               pAddr = &Addr;
	ULONG                  hNotification;
	AdsNotificationAttrib  adsNotificationAttrib;

	cVar = condVar;
	endProgram = end;

	// Abrir comunicación de puertos con el PLC.
	nPort = AdsPortOpen();
	nErr = AdsGetLocalAddress(pAddr);
	if (nErr) std::cerr << "Error: AdsGetLocalAddress: " << nErr << '\n';
	pAddr->port = 851;

	// Atributos para la notificación.
	adsNotificationAttrib.cbLength = 1;
	adsNotificationAttrib.nTransMode = ADSTRANS_SERVERONCHA;  // Llama a "Callback" solo cuando haya un cambio en la variable.
	adsNotificationAttrib.nMaxDelay = 0;
	adsNotificationAttrib.nCycleTime = 0; //  1 mili ---- Tiempo de refresco para comprobar el PLC.

	// Adquiere el offset de la variable a leer.
	nErr = AdsSyncReadWriteReq(pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(hVar1), &hVar1, sizeof(szVar1), szVar1);
	if (nErr) std::cerr << "Error: AdsSyncReadWriteReq: " << nErr << '\n';

	// Inicia la transmisión con la variable del PLC.
	nErr = AdsSyncAddDeviceNotificationReq(pAddr, ADSIGRP_SYM_VALBYHND, hVar1, &adsNotificationAttrib, Callback, hVar1, &hNotification);
	if (nErr) std::cerr << "Error: AdsSyncAddDeviceNotificationReq: " << nErr << '\n';

	std::cout << "Notification: " << hNotification << "\n\n";
	std::cout.flush();
	
}