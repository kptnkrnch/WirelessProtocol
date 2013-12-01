/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: 
--
-- PROGRAM: 
--
-- FUNCTIONS:
-- 
--
--
-- DATE: 11.21.13
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Damien Sathanielle
--
-- PROGRAMMER: Damien Sathanielle
--
-- NOTES:
-- 
----------------------------------------------------------------------------------------------------------------------*/
#include <Windows.h>
#include <iostream>
#include "sendFile.h"
#include "global.h"

extern Stats stats;

using namespace std;

DWORD dwBytesWritten;

Buffer buffer;

extern OVERLAPPED ov;

DWORD WINAPI sendBufferThread(LPVOID n){

	Globals* global = (Globals*)n;
	dwBytesWritten = 0;

	//infinitely wait for the buffer to have at least one packet.
	while (!buffer.is_empty()) {

		send_packets(global);
		Sleep(200);
	}

	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: send_file
--
-- DATE: 11.21.13
--
-- REVISIONS: 11.23.13 - replaced leftover pseudocode
--            11.26.13 - added semaphores and error checking
--            11.27.13 - added buffer and fix ENQ/EOT sending
--            11.28.13 - put in while loop
--
-- DESIGNER: Damien Sathanielle
--
-- PROGRAMMER: Damien Sathanielle
--
-- INTERFACE: void send_file(char* data[]);
--
-- RETURNS: void.
--
-- NOTES: send_file() will constantly run at the start of program. It constantly checks the buffer for packets to
--        send. A maximum of 5 packets at a time will be sent before giving up the line to the reciever to transmit
--        data. As packets are transmited, they are removed from the buffer. 
-- 
----------------------------------------------------------------------------------------------------------------------*/

void send_packets(Globals *global)
{
	int packets_sent = 0;

	// if starting to send data, enquire line
	if(!enquire_line(*(global->hComm), *(global->hSem))) {
		cerr << "could not enquire line" << endl;
		//return;
	}

    // loop through all the data 
    while(!buffer.is_empty() && packets_sent < 5)
    {
		// transmit a packet
		if(!transmit_packet(*(global->hComm), *(global->hSem), buffer.get_packet())) 
			cerr << "error sending packet" << endl;
		else
		{
			packets_sent++;
			stats.totalPacketsSent_++;
		}

		buffer.remove_packet();
    }

    // send end of transmition
    sendControlChar(*(global->hComm), EOT);

    ReleaseSemaphore(*(global->hSem), 1, NULL);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: enquire_line
--
-- DATE: 11.21.13
--
-- REVISIONS: 11.27.13 - added ENQ function
--
-- DESIGNER: Damien Sathanielle
--
-- PROGRAMMER: Damien Sathanielle
--
-- INTERFACE: bool enquire_line();
--
-- RETURNS: bool.
--
-- NOTES: transmits and enquiry for the line and waits for a response
-- 
----------------------------------------------------------------------------------------------------------------------*/

bool enquire_line(HANDLE& hComm, HANDLE& hSem)
{
    // send enq
    sendControlChar(hComm, ENQ);

    // wait for reciever to acknowledge line is free
    return wait_for_acknowledgement(hSem);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: transmit_packet
--
-- DATE: 11.21.13
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Damien Sathanielle
--
-- PROGRAMMER: Damien Sathanielle
--
-- INTERFACE: bool transmit_packet(char* data);
--
-- RETURNS: bool.
--
-- NOTES: Writes a packet to serial port
-- 
----------------------------------------------------------------------------------------------------------------------*/
bool transmit_packet(HANDLE& hComm, HANDLE& hSem, const char* data)
{
	DWORD bytes = 0;
	if (!WriteFile(hComm, data, PACKET_SIZE, &bytes, &ov)) 
	{
		int err = GetLastError();

		if (err == ERROR_IO_PENDING)
			bytes = 1;
	}
	//WriteFile(hComm, packet, 1024, &dwBytesWritten, &ov);
    return wait_for_acknowledgement(hSem);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: wait_for_acknowledgement
--
-- DATE: 11.21.13
--
-- REVISIONS: 11.26.13 - added WaitForSingleObject
--
-- DESIGNER: Damien Sathanielle
--
-- PROGRAMMER: Damien Sathanielle
--
-- INTERFACE: bool wait_for_acknowledgement();
--
-- RETURNS: bool.
--
-- NOTES: waits for an acknowledgement event from recieve function and returns true if an ACK was recieved
-- 
----------------------------------------------------------------------------------------------------------------------*/
bool wait_for_acknowledgement(HANDLE& hSem)
{

    DWORD dwWaitResult = WaitForSingleObject(hSem, TIMEOUT_TIME);
    // wait for ack (return true of ack)
    switch(dwWaitResult)
    {
        // recieved ack
        case WAIT_OBJECT_0:  
			stats.totalNAKsReceived_++;
            return true; 
        // ack not recieved
        case WAIT_ABANDONED: 
            return false;  
    }

	return false;
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendControlChar
--
-- DATE: 11.27.13
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Damien Sathanielle
--
-- PROGRAMMER: Damien Sathanielle
--
-- INTERFACE: bool sendControlChar(HANDLE hComm, char control)
--
-- RETURNS: void.
--
-- NOTES: packetize and sents end of transmition
-- 
----------------------------------------------------------------------------------------------------------------------*/
void sendControlChar(HANDLE& hComm, char control)
{
    char c[2];
    c[0] = SYN;
    c[1] = control;
    WriteFile(hComm, c, 2, &dwBytesWritten, &ov);
}