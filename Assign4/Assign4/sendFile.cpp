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

using namespace std;

DWORD dwBytesWritten;
DWORD dwWaitResult; 

extern HANDLE hSem;
extern HANDLE hComm;
Buffer buffer;

DWORD WINAPI sendBufferThread(LPVOID n){

	//infinitely wait for the buffer to have at least one packet.
	while (!buffer.is_empty()) {

		send_packets();

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
void send_packets()
{
	int packets_sent = 0;

	// if starting to send data, enquire line
	if(!enquire_line())
		cerr << "could not enquire line" << endl;

    // loop through all the data
    while(!buffer.is_empty() && packets_sent < 5)
    {
		// transmit a packet
		if(!transmit_packet(buffer.get_packet()))
			cerr << "error sending packet" << endl;
		packets_sent++;
		buffer.remove_packet();
    }
    // send end of transmition
    sendEOT(hComm);
    ReleaseSemaphore(hSem, 1, NULL);
	//delay after we send EOT
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
bool enquire_line()
{
    // send enq
    if (!sendENQ(hComm))
        return false;

    // wait for reciever to acknowledge line is free
    return wait_for_acknowledgement();
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
bool transmit_packet(const char* data)
{
    // write data to port
    return WriteFile(hComm, data,(DWORD) PACKET_SIZE, &dwBytesWritten, NULL);
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
bool wait_for_acknowledgement()
{

    dwWaitResult = WaitForSingleObject(hSem,INFINITE);
    // wait for ack (return true of ack)
    switch(dwWaitResult)
    {
        // recieved ack
        case WAIT_OBJECT_0:            
            return true; 
        // ack not recieved
        case WAIT_ABANDONED: 
            return false;  
    }

	return false;
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendEOT
--
-- DATE: 11.27.13
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Damien Sathanielle
--
-- PROGRAMMER: Damien Sathanielle
--
-- INTERFACE: void sendEOT(HANDLE hComm)
--
-- RETURNS: void.
--
-- NOTES: packetize and sents end of transmition
-- 
----------------------------------------------------------------------------------------------------------------------*/
void sendEOT(HANDLE hComm)
{
    char c[2];
    c[0] = SYN;
    c[1] = EOT;
    DWORD bytesRead;
    WriteFile(hComm, c, 2, &bytesRead, NULL);
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendENQ
--
-- DATE: 11.27.13
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Damien Sathanielle
--
-- PROGRAMMER: Damien Sathanielle
--
-- INTERFACE: bool sendENQ(HANDLE hComm)
--
-- RETURNS: bool.
--
-- NOTES: packetize and sents enquiry for line
-- 
----------------------------------------------------------------------------------------------------------------------*/
bool sendENQ(HANDLE hComm)
{
    char c[2];
    c[0] = SYN;
    c[1] = ENQ;
    DWORD bytesRead;
    return WriteFile(hComm, c, 2, &bytesRead, NULL);
}