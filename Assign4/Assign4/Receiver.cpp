#include <windows.h>
#include "packetize.h"
#define SYN 0x16
#define ENQ 0x05
#define ACK 0x06
#define NAK 0x15
#define EOT 0x04

HANDLE hComm;
HANDLE hSem;

_OVERLAPPED ov;

void sendACK(HANDLE hComm);
void sendNAK(HANDLE hComm);
void waitForPackets(HANDLE hComm);



DWORD WINAPI receiverThread(LPVOID n){
	DWORD bytesRead;
	DWORD event;
	char c[2];

	SetCommMask(hComm, EV_RXCHAR);

	while(1){
		WaitForSingleObject(hSem, 0);
		WaitCommEvent(hComm, &event, NULL);
		if(event & EV_RXCHAR){
			ReadFile(hComm, c, 2, &bytesRead, NULL);
			if(c[0] == SYN && c[1] == ENQ){
				sendACK(hComm);
				waitForPackets(hComm);
			}
			if(c[0] == SYN && c[1] == ACK){
				ReleaseSemaphore(hSem, 0, NULL);
			}
		}
	}
	return 0;
}

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
}

void waitForPackets(HANDLE hComm){
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
				if(!recievePacket(c)){
					sendNAK(hComm);
				} else{
					sendACK(hComm);
				}
			}
		} else {
			timeout = true;
		}
	}while(!timeout);

	ReleaseSemaphore(hSem, 0, NULL);
}