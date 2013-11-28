#ifndef SENDFILE_H
#define SENDFILE_H

#include <Windows.h>
#include <iostream>

#include "buffer.h"

#define ENQ 0x05
#define EOT 0x04
#define SYN 0x16

#define PACKET_SIZE 1024

void send_file(char* data[]);
bool enquire_line();
bool transmit_packet(const char* data);
bool wait_for_acknowledgement();
void sendEOT(HANDLE hComm);
bool sendENQ(HANDLE hComm);

Buffer buffer;

#endif
