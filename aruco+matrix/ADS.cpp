#include "ADS.h"
#include "WriteFiles.h"

std::ofstream* file;

const int LIMIT = 30;

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

void startAdsConnection(std::ofstream* allOutfile)
{

	long                   nErr, nPort;
	AmsAddr                Addr;
	PAmsAddr               pAddr = &Addr;
	ULONG                  hNotification;
	AdsNotificationAttrib  adsNotificationAttrib;
	
	file = allOutfile;

	// Abrir comunicación de puertos con el PLC.
	nPort = AdsPortOpen();
	nErr = AdsGetLocalAddress(pAddr);
	if (nErr) std::cerr << "Error: AdsGetLocalAddress: " << nErr << '\n';
	pAddr->port = 851;

	// Atributos para la notificación.
	adsNotificationAttrib.cbLength = 4;
	adsNotificationAttrib.nTransMode = ADSTRANS_SERVERONCHA;  // Llama a "Callback" solo cuando haya un cambio en la variable.
	adsNotificationAttrib.nMaxDelay = 0;
	adsNotificationAttrib.nCycleTime = 10; // 1sec ---- Tiempo de refresco para comprobar el PLC.

	// Adquiere el offset de la variable a leer.
	nErr = AdsSyncReadWriteReq(pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(hUser), &hUser, sizeof(szVar), szVar);
	if (nErr) std::cerr << "Error: AdsSyncReadWriteReq: " << nErr << '\n';

	// Adquiere el offset de la variable a escribir.
	nErr = AdsSyncReadWriteReq(pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(hWrite), &hWrite, sizeof(szVarPc), szVarPc);
	if (nErr) std::cerr << "Error: AdsSyncReadWriteReq: " << nErr << '\n';

	// Inicializa la variable de lectura a 0.
	long nData = 0;
	nErr = AdsSyncWriteReq(pAddr, ADSIGRP_SYM_VALBYHND, hUser, sizeof(nData), &nData);
	if (nErr) std::cerr << "Error: AdsSyncWriteReq: " << nErr << '\n';

	// Inicia la transmisión con la variable del PLC.
	nErr = AdsSyncAddDeviceNotificationReq(pAddr, ADSIGRP_SYM_VALBYHND, hUser, &adsNotificationAttrib, Callback, hUser, &hNotification);
	if (nErr) std::cerr << "Error: AdsSyncAddDeviceNotificationReq: " << nErr << '\n';

	std::cout << "Notification: " << hNotification << "\n\n";
	std::cout.flush();
	writeValuesInPlc(hWrite, pAddr);
	
}

// Callback-function
void __stdcall Callback(AmsAddr* pAddr, AdsNotificationHeader* pNotification, ULONG hA)
{
	// Guarda la variable cambiada en el PLC.
	long buf = *(long *)pNotification->data;

	fifoWrite(buf);

	//writeRobotPoseInFile(buf, file);

}

// Esta función se ejecuta en bucle por un hilo, y se encarga
// de ver si hay algún elemento en el buffer. Si lo hay, abre
// una conexión por ADS con el PLC para excribir esa variable.
void writeValuesInPlc(long hWrite, AmsAddr* pAddr) {
	long nErr;

	while (true) {
		if (count > 0) {
			long value = fifoRead();

			nErr = AdsSyncWriteReq(pAddr, ADSIGRP_SYM_VALBYHND, hWrite, sizeof(value), &value);
			if (nErr) std::cerr << "Error: AdsSyncWriteReq: " << nErr << '\n';
		}
	}
	
}

// Coger primer elemento del buffer.
long fifoRead() {
	if (count == 0) return -1;
	count--;
	tail = (tail + 1) % LIMIT;
	std::cout << "PC: " << buffer[tail] << '\n';
	return buffer[tail];
}

// Escribir último elemento de buffer.
long fifoWrite(long val) {
	if (count >= LIMIT) {
		std::cout << "FULL" << '\n';
		return -1;
	}
	buffer[head] = val;
	count++;
	head = (head + 1) % LIMIT;
	std::cout << "ROBOT: " << val << '\n';
	return buffer[head];
}
