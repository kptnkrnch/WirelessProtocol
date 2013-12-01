/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE:     packetize.cpp
--
-- PROGRAM:         name
--
-- FUNCTIONS:
--                  bool recievePacket(unsigned char data[PACKET_SIZE])
--					void readFile(fstream &is)
--					void packetize(unsigned char data[DATA_SIZE])
--
-- DATE:            November 18, 2013
--
-- REVISIONS:       November 20, 2013        Implemented recieving the packet.
--
--                  November 25, 2013        Fixed bug in recieve packet to encode and check for CRC.
--
-- DESIGNER:        Jordan Marling
--
-- PROGRAMMER:      Jordan Marling
--
-- NOTES:           This file handles all packet conversions. It can take a stream of data and packetize it
--					and recieve packets and pull data out of them.
--
----------------------------------------------------------------------------------------------------------------------*/

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>

#include "Buffer.h"
#include "crc.h"
#include "packetize.h"

using namespace std;

int send_control;
int recv_control = SOT1;

extern Buffer buffer;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:		recievePacket
--
-- DATE:			November 20, 2013
--
-- REVISIONS:		November 25, 2013		Fixed CRC calculation error.
--
-- DESIGNER:		Jordan Marling
--
-- PROGRAMMER:		Jordan Marling
--
-- INTERFACE:		bool recievePacket(unsigned char data[PACKET_SIZE])
--							data: the packet as a character array.
--
-- RETURNS:			if the packet was good or not. If the control characters are correct or not.
--
-- NOTES: 			This function takes in a packet and checks to see if the CRC value is good. It takes the
--					data out of the packet and sends it to be displayed.
--
----------------------------------------------------------------------------------------------------------------------*/
bool recievePacket(char data[]) {
	
	if (data[0] != SYN) {
		cerr << "packet didnt start with SYN" << endl;
		return false;
	}
	
	if (data[1] != recv_control) {
		cerr << "packet wasn't sequenced properly." << endl;
		return false;
	}
	
	recv_control = recv_control == SOT1 ? SOT2 : SOT1;
	
	char *packet_data = &data[2];
	short first, second;
	
	first = (unsigned short)data[PACKET_SIZE - 2];
	second = (unsigned short)data[PACKET_SIZE - 1];
	
	
	unsigned short crc = ((first << 8) & 0xff00) + second;
	unsigned short crc_check = crc16(packet_data, DATA_SIZE);
	
	if (crc_check != crc) {
		cerr << "CRC fail. Got " << hex << setw(4) << setfill('0') << crc << " and expected " << crc16(packet_data, DATA_SIZE) << endl;
		return false;
	}
	
	int last_char = 0;
	
	for(int i = 0; i < DATA_SIZE; i++) {
		if (packet_data[i] != (char)0) {
			last_char = i;
			cout << packet_data[i];
		}
	}
	
	if (last_char < DATA_SIZE - 1) {
		packet_data[last_char] = '\0';
	}
	
	return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:		readFile
--
-- DATE:			November 20, 2013
--
-- REVISIONS:		
--
-- DESIGNER:		Jordan Marling
--
-- PROGRAMMER:		Jordan Marling
--
-- INTERFACE:		void readFile(fstream &is)
--							is: the inputstream to the data that is being sent.
--
-- RETURNS:			void
--
-- NOTES: 			This function reads in data from an input stream and packetizes it.
--
----------------------------------------------------------------------------------------------------------------------*/
void readFile(fstream &is) {
	
	char data[DATA_SIZE];
	int tmp;
	int count = 0;
	
	send_control = SOT1;
	
	while ((tmp = is.get()) >= 0) {
		
		data[count++] = (unsigned char)tmp;
		
		if (count >= DATA_SIZE) {
			packetize(data);
			count = 0;
		}
		
	}
	
	for(;count < DATA_SIZE; count++) {
		
		data[count] = (unsigned char)0;
		
	}
	
	packetize(data);
	
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION:		packetize
--
-- DATE:			November 18, 2013
--
-- REVISIONS:		November 25, 2013		Fixed CRC calculation error.
--
-- DESIGNER:		Jordan Marling
--
-- PROGRAMMER:		Jordan Marling
--
-- INTERFACE:		void packetize(unsigned char data[DATA_SIZE])
--							data: the text data to be put into a packet.
--
-- RETURNS:			void
--
-- NOTES: 			This function takes in data and puts it into a packet. The packet is as follows:
--					
--					SYN|SOT1/SOT2|DATA|CRC
--					
--					SOT1 and SOT2 are switched back and forth to enable sequencing to allow the
--					machine recieving know if it has already recieved this packet.
--					
--					The packet in bytes:
--
--					SYN - 1|SOT1/SOT2 - 1|DATA - 1020| CRC - 2
--
----------------------------------------------------------------------------------------------------------------------*/
void packetize(char data[DATA_SIZE]) {

	char *packet = (char*)malloc(PACKET_SIZE);
	
	//Set the first to chars to describe what type of packet it is.
	packet[0] = SYN;
	packet[1] = send_control;
	
	//swap the SOT1 and SOT2 to enable sequencing.
	send_control = send_control == SOT1 ? SOT2 : SOT1;
	
	//load the data into the packet.
	for(int i = 0; i < DATA_SIZE; ++i) {
		packet[i + 2] = data[i];
	}
	
	//get the crc value
	unsigned short crc = crc16(data, DATA_SIZE);
	unsigned short first, second;
	
	//take out each part of it so we can store it in 2 characters.
	first = (crc & 0xff00) >> 8;
	second = crc & 0x00ff;
	
	packet[PACKET_SIZE - 2] = (char)first;
	packet[PACKET_SIZE - 1] = (char)second;
	
	//send the packet.
	
	//for(int i = 0; i < PACKET_SIZE; i++) {
	//	cout << (char)packet[i];
	//}
	
	buffer.add_packet(packet);

}
