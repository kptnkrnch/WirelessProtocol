#include <windows.h>
#include "packetize.h"
#include "Receiver.h"
#include "sendFile.h"
#include "global.h"

OVERLAPPED ov;
DWORD bytesRead = 0;

void read(HANDLE hComm, char* c, int bytesToRead) {

	DWORD obj;
	DWORD event = 0;
	DWORD bytesTransferred = 0;
	DWORD br = 0;
	bool wait;

	wait = WaitCommEvent(hComm, &event, &ov);
	obj = WaitForSingleObject(ov.hEvent, INFINITE);
	

	if (obj == WAIT_OBJECT_0 && event & EV_RXCHAR) {
		
		
		if (GetOverlappedResult(hComm, &ov, &bytesTransferred, TRUE)) {

			do {
				GetOverlappedResult(hComm, &ov, &bytesTransferred, TRUE);
				ReadFile(hComm, c + br, bytesToRead - br, &br, &ov);

				//ReadFile(hComm, c, bytesToRead, &bytesRead, &ov);

				MessageBox(NULL, TEXT("RECV"), TEXT(""), MB_OK);
			} while (br < bytesToRead);

		}
	}

	ResetEvent(ov.hEvent);
}

DWORD WINAPI receiverThread(LPVOID n){
	Globals *globals = (Globals*)n;
	DWORD event;
	char c[2];

	DWORD bytesRead = 0;
	
	ov.Internal = 0;
	ov.InternalHigh = 0;
	ov.Offset = 0;
	ov.OffsetHigh = 0;
	ov.Pointer = 0;
	ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	
	SetCommMask(globals->hComm, EV_RXCHAR);

	while(1){
		//WaitForSingleObject(globals->hSem, 0);
		/*WaitCommEvent(globals->hComm, &event, &ov);
		WaitForSingleObject(ov.hEvent, INFINITE);
		if(event & EV_RXCHAR){
			ReadFile(globals->hComm, c, 2, &bytesRead, &ov);
			if(c[0] == SYN && c[1] == ENQ){
				MessageBox(NULL, TEXT("GOT ENQ"), TEXT(""), MB_OK);
				sendControlChar(globals->hComm, ACK);
				waitForPackets(globals->hComm, globals->hSem);
			}
			if(c[0] == SYN && c[1] == ACK){
				MessageBox(NULL, TEXT("GOT ACK"), TEXT(""), MB_OK);
				ReleaseSemaphore(globals->hSem, 1, NULL);
			}
		}*/

		read(globals->hComm, c, 2);
		if(c[0] == SYN && c[1] == ENQ){
			MessageBox(NULL, TEXT("GOT ENQ"), TEXT(""), MB_OK);
			sendControlChar(globals->hComm, ACK);
			waitForPackets(globals->hComm, globals->hSem);
		}
		if(c[0] == SYN && c[1] == ACK){
			MessageBox(NULL, TEXT("GOT ACK"), TEXT(""), MB_OK);
			ReleaseSemaphore(globals->hSem, 1, NULL);
		}
	}
	return 0;
}

void waitForPackets(HANDLE hComm, HANDLE hSem){
	bool timeout = false;
	char c[1024];
	DWORD obj;
	DWORD event;


	do{
		//GET TYPE OF PACKET
		/*WaitCommEvent(hComm, &event, &ov);
		obj = WaitForSingleObject(ov.hEvent, INFINITE);
		if(obj == WAIT_OBJECT_0 && (event & EV_RXCHAR)){
			ReadFile(hComm, c, 2, &bytesRead, &ov);
			if(c[1] == EOT){
				break;
			} else {

				//READ REST OF PACKET
				ReadFile(hComm, c+2, 1022, &bytesRead, NULL);
				MessageBox(NULL, TEXT("GOT PACKET"), TEXT(""), MB_OK);
				if(recievePacket(c)){
					sendControlChar(hComm, ACK);
				} else{
					sendControlChar(hComm, NAK);
				}


			}
		} else {
			timeout = true;
		}*/


		read(hComm, c, 2);
		if (c[0] == SYN) {
			if (c[1] == EOT) {
				break;
			}
			else {

				read(hComm, c + 2, 1022);
				MessageBox(NULL, TEXT("GOT PACKET"), TEXT(""), MB_OK);
				if(recievePacket(c)){
					sendControlChar(hComm, ACK);
				} else{
					sendControlChar(hComm, NAK);
				}
			}
		}
		else {
			//not a packet.
			MessageBox(NULL, TEXT("Recieved something that wasnt a packet."), TEXT(""), MB_OK);
		}

	}while(!timeout);

	ReleaseSemaphore(hSem, 1, NULL);
}