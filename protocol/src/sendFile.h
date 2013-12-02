#ifndef SENDFILE_H
#define SENDFILE_H

#include <Windows.h>
#include <iostream>

#include "buffer.h"
#include "global.h"

#define PACKET_SIZE 1024
#define TIMEOUT_TIME 1500

DWORD WINAPI sendBufferThread(LPVOID n);
void send_packets(Globals*);
bool enquire_line(Globals*);
bool transmit_packet(Globals*, char*);
bool wait_for_acknowledgement(Globals*);
void sendControlChar(HANDLE *hComm, char);

#endif
