#ifndef SENDFILE_H
#define SENDFILE_H

#include <Windows.h>
#include <iostream>

#include "buffer.h"
#include "global.h"

#define PACKET_SIZE 1024
#define TIMEOUT_TIME 200

DWORD WINAPI sendBufferThread(LPVOID n);
void send_packets(Globals*);
bool enquire_line(HANDLE&, HANDLE&);
bool transmit_packet(HANDLE&, HANDLE&, const char*);
bool wait_for_acknowledgement(HANDLE&);
void sendControlChar(HANDLE& hComm, char);

#endif
