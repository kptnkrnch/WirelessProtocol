#include <windows.h>
#include "packetize.h"
#include "Receiver.h"
#include "sendFile.h"
#include "global.h"

extern OVERLAPPED ov;
DWORD bytesRead = 0;
bool flag = false;

bool read(HANDLE& hComm, char* c, int bytesToRead) {
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
			wait = WaitCommEvent(hComm, &ev, &ov);
			if (wait)
				return false;
			obj = WaitForSingleObject(ov.hEvent, INFINITE);
			//if (wait) {
			ClearCommError(hComm, &errors, &cs);
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
				if (cs.cbInQue == 2) {
					ReadFile(hComm, c, cs.cbInQue, &br, &ov);
				} else {
					ReadFile(hComm, c, 1024, &br, &ov);
				}
									//br_total += br;
									//ReadFile(hComm, c, bytesToRead, &bytesRead, &ov);

									//MessageBox(NULL, TEXT("RECV"), TEXT(""), MB_OK);
							//} while (br_total < bytesToRead);

					//}
				return true;
			}
			//}
		//}

        //ResetEvent(ov.hEvent);
			return false;
}

DWORD WINAPI receiverThread(LPVOID n){
        Globals *globals = (Globals*)n;
        DWORD event;
        char c[2];

        DWORD bytesRead = 0;
        
        SetCommMask(*(globals->hComm), EV_RXCHAR | EV_TXEMPTY);

        while(1){
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

                if (!read(*(globals->hComm), c, 2))
					continue;

                if(c[0] == SYN && c[1] == ENQ){
                        //MessageBox(NULL, TEXT("GOT ENQ"), TEXT(""), MB_OK);
                        sendControlChar(*(globals->hComm), ACK);
                        waitForPackets(*(globals->hComm), *(globals->hSem));
                }
                if(c[0] == SYN && c[1] == ACK){
                        //MessageBox(NULL, TEXT("GOT ACK"), TEXT(""), MB_OK);
                        ReleaseSemaphore(*(globals->hSem), 1, NULL);
                }
        }
        return 0;
}

void waitForPackets(HANDLE& hComm, HANDLE& hSem){
        bool timeout = false;
		char c[1024] = {0};
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


                if (!read(hComm, c, 2))
					continue;

                if (c[0] == SYN) {
                        if (c[1] == EOT) {
                                break;
                        }
                        else {

                                //if (read(hComm, c + 2, 1022)) {
									MessageBox(NULL, TEXT("GOT PACKET"), TEXT(""), MB_OK);
									if(recievePacket(c)){
											sendControlChar(hComm, ACK);
									} else{
											sendControlChar(hComm, NAK);
									}
								//}
                        }
                }
                else {
                        //not a packet.
                        MessageBox(NULL, TEXT("Recieved something that wasnt a packet."), TEXT(""), MB_OK);
                }

        }while(!timeout);

        ReleaseSemaphore(hSem, 1, NULL);
}