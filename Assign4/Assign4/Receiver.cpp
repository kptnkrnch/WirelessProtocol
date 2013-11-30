#include <windows.h>
#include "packetize.h"
#include "Receiver.h"
#include "sendFile.h"
#include "global.h"

_OVERLAPPED ov;



DWORD WINAPI receiverThread(LPVOID n){
	Globals *globals = (Globals*)n;
	DWORD bytesRead;
	DWORD event;
	char c[2];

	SetCommMask(globals->hComm, EV_RXCHAR);

	while(1){
		WaitForSingleObject(globals->hSem, 0);
		WaitCommEvent(globals->hComm, &event, NULL);
		if(event & EV_RXCHAR){
			ReadFile(globals->hComm, c, 2, &bytesRead, NULL);
			if(c[0] == SYN && c[1] == ENQ){
				sendControlChar(globals->hComm, ACK);
				waitForPackets(globals->hComm, globals->hSem);
			}
			if(c[0] == SYN && c[1] == ACK){
				ReleaseSemaphore(globals->hSem, 0, NULL);
			}
		}
	}
	return 0;
}
/*
void sendACK(HANDLE hComm){
	char c[2];
	c[0] = SYN;
	c[1] = ACK;
	DWORD bytesRead;
	WriteFile(hComm, c, 2, &bytesRead, NULL);
}

void sendNAK(HANDLE hComm){
	char c[2];
	c[0] = SYN;
	c[1] = NAK;
	DWORD bytesRead;
	WriteFile(hComm, c, 2, &bytesRead, NULL);
}*/

void waitForPackets(HANDLE hComm, HANDLE hSem){
	bool timeout = false;
	char c[1024];
	DWORD obj;
	DWORD event;
	DWORD bytesRead;

	ov.Internal = 0;
	ov.InternalHigh = 0;
	ov.Offset = 0;
	ov.OffsetHigh = 0;
	ov.Pointer = 0;
	ov.hEvent = CreateEvent(NULL, true, false, NULL);

	do{
		WaitCommEvent(hComm, &event, &ov); //change to overlapped
		obj = WaitForSingleObject(ov.hEvent, 200);
		if(obj == WAIT_OBJECT_0 && (event & EV_RXCHAR)){
			ReadFile(hComm, c, 2, &bytesRead, NULL);
			if(c[1] == EOT){
				break;
			} else {
				ReadFile(hComm, c+2, 1022, &bytesRead, NULL);
				if(recievePacket(c)){
					sendControlChar(hComm, ACK);
				} else{
					sendControlChar(hComm, NAK);
				}
			}
		} else {
			timeout = true;
		}
	}while(!timeout);

	ReleaseSemaphore(hSem, 0, NULL);
}