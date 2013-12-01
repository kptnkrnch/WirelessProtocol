#ifndef SENDFILE_H
#define SENDFILE_H

#include <Windows.h>
#include <iostream>

#include "buffer.h"
#include "global.h"

#define ENQ 0x05
#define EOT 0x04
#define SYN 0x16

#define PACKET_SIZE 1024
#define TIMEOUT_TIME INFINITE

DWORD WINAPI sendBufferThread(LPVOID n);
void send_packets(Globals*);
bool enquire_line(HANDLE&, HANDLE&);
bool transmit_packet(HANDLE&, HANDLE&, const char*);
bool wait_for_acknowledgement(HANDLE&);
void sendControlChar(HANDLE& hComm, char);

#endif
