#ifndef RECEIVER_H
#define RECEIVER_H

#include <Windows.h>

DWORD WINAPI receiverThread(LPVOID n);
void waitForPackets(HANDLE& hComm, HANDLE& hSem);

#endif