#include <windows.h>
#include "packetize.h"
#include "Receiver.h"
#include "sendFile.h"
#include "global.h"
#include "Terminal.h"

extern OVERLAPPED ov;
extern Stats stats;
extern int recv_control;
DWORD bytesRead = 0;
bool flag = false;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: read
--
-- DATE: 11.21.13
--
-- REVISIONS: 11.27.13 - changed to use overlapped struct
--            11.27.13 - changed to return a bool rather than void
--            11.28.13 - change to read in all bytes rather than one byte at a time
--
-- DESIGNER: Cory Thomas
--
-- PROGRAMMERS: Cory Thomas, Joshua Campbell, Jordan Marling
--
-- INTERFACE: bool read(HANDLE&, char*, int);
--
-- RETURNS: bool
--
-- NOTES: read() will be called whenever somehting needs to be read form the comm port. The function will wait till data
-- 	  comes in through the comm port and then store the data into a char array. If the data if properly read, the 
--        function will return true. The function will return false if an incorrect number of bytes came through the 
--        comm port.
-- 
----------------------------------------------------------------------------------------------------------------------*/

bool read(HANDLE *hComm, char* c, int bytesToRead, bool &timeout) {
	COMSTAT cs;
	DWORD obj;
	DWORD errors;
	DWORD ev = 0;
	DWORD bytesTransferred = 0;
	DWORD br = 0;
	DWORD br_total = 0;
	bool wait;
	bool complete = false;

	//while (!complete) {
	wait = WaitCommEvent(*hComm, &ev, &ov);
	if (wait)
		return false;
	obj = WaitForSingleObject(ov.hEvent, TIMEOUT_TIME);
	//if (wait) {
	ClearCommError(*hComm, &errors, &cs);
	if (obj == WAIT_OBJECT_0 && ev & EV_RXCHAR && (cs.cbInQue == 2 || cs.cbInQue >= 1024)) {
		//if ((ev & EV_RXCHAR) && cs.cbInQue) {


		//if (GetOverlappedResult(hComm, &ov, &bytesTransferred, TRUE)) {

		//do {
		//if (cs.cbInQue == 2 && flag == false) {
		//GetOverlappedResult(hComm, &ov, &bytesTransferred, TRUE);
		//ReadFile(hComm, c, cs.cbInQue, &br, &ov);
		//flag = true;
		//complete = true;
		//} else if (cs.cbInQue == 1024 && flag == true) {
		/*ReadFile(hComm, c, 2, &br, &ov);
		flag = false;
		complete = true;*/
		//}

		if (cs.cbInQue >= bytesToRead)
			ReadFile(*hComm, c, bytesToRead, &br, &ov);

		/*if (cs.cbInQue >= 2 && bytesToRead == 2) {
			ReadFile(*hComm, c, bytesToRead, &br, &ov);
		} else {
			ReadFile(*hComm, c, bytesToRead, &br, &ov);
			
		}*/

		//br_total += br;
		//ReadFile(hComm, c, bytesToRead, &bytesRead, &ov);

		//MessageBox(NULL, TEXT("RECV"), TEXT(""), MB_OK);
		//} while (br_total < bytesToRead);

		//}
		return true;
	}
	timeout = obj == WAIT_TIMEOUT;
	//}
	//}

	ResetEvent(ov.hEvent);
	return false;
}

DWORD WINAPI receiverThread(LPVOID n){
	Globals *globals = (Globals*)n;
	DWORD event;
	char c[2];
	bool timeout;

	DWORD bytesRead = 0;

	SetCommMask(*(globals->hComm), EV_RXCHAR | EV_TXEMPTY);

	while(1){
		//timeout = false;
		c[0] = 0;
		c[1] = 0;
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

		if (!read(globals->hComm, c, 2, timeout))
			continue;

		if(c[0] == SYN && c[1] == ENQ){
			//MessageBox(NULL, TEXT("GOT ENQ"), TEXT(""), MB_OK);
			sendControlChar(globals->hComm, ACK);
			stats.totalRequests_++;
			UpdateStats();
			waitForPackets(globals);
		}
		else if(c[0] == SYN && c[1] == ACK){
			//MessageBox(NULL, TEXT("GOT ACK"), TEXT(""), MB_OK);
			globals->gotAck = true;
			ReleaseSemaphore(*(globals->hSem), 1, NULL);
			stats.totalACKsReceived_++;
			UpdateStats();
		}
		else if(c[0] == SYN && c[1] == NAK){
			globals->gotAck = false;
			ReleaseSemaphore(*(globals->hSem), 1, NULL);
			stats.totalNAKsReceived_++;
			UpdateStats();
		}
	}
	return 0;
}

void waitForPackets(Globals *globals){
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


		if (read(globals->hComm, c, 2, timeout)) {

			if (c[0] == SYN) {
				if (c[1] == EOT) {

					recv_control = 0;

					//MessageBox(NULL, TEXT("Got EOT"), TEXT(""), MB_OK);
					globals->gotAck = true;
					ReleaseSemaphore(*(globals->hSem), 1, NULL);
					return;
				}
				else if (c[1] == SOT1 || c[1] == SOT2) {
					ReadFile(*(globals->hComm), c + 2, 1022, &bytesRead, &ov);
					//if (read(globals->hComm, c + 2, 1022, timeout)) {
					//MessageBox(NULL, TEXT("GOT PACKET"), TEXT(""), MB_OK);
					if(recievePacket(globals, c)){
						//MessageBox(NULL, TEXT("Got packet"), TEXT(""), MB_OK);
						sendControlChar(globals->hComm, ACK);
					} else{
						sendControlChar(globals->hComm, NAK);
					}
					//}
				}
			}
			else {
				//not a packet.
				MessageBox(NULL, TEXT("Recieved something that wasnt a packet."), TEXT(""), MB_OK);
			}
		}

	}while(!timeout);

	if (timeout) {
		stats.totalTimeouts_++;
		UpdateStats();
		ReleaseSemaphore(*(globals->hSem), 1, NULL);
	}
}
