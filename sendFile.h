#ifndef SENDFILE_H
#define SENDFILE_H

#include <iostream>

#define ENQ 0x05
#define EOT 0x04

#define PACKET_SIZE 1024

void send_file(char* data[]);
bool enquire_line();
bool transmit_packet(char* data);
bool wait_for_acknowledgement();
void sendEOT(HANDLE hComm);
bool sendENQ(HANDLE hComm);

#endif
