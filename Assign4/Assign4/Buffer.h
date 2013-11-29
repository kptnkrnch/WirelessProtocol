#ifndef BUFFER_H
#define BUFFER_H

#include <Windows.h>
#include <iostream>
#include <string>
#include <queue>

class Buffer
{
    public:

        Buffer(){}

        // adds a packet to the queue
        void add_packet(const char* packet){ 
			buff.push(packet);
		}

        // retrieves a packet and pops it off the buffer
        const char* get_packet() 
        {
            if(!buff.empty())
                return buff.front();
            else
                std::cerr << "buffer empty" << std::endl;
            return NULL;
        }

        // removes packet from queue
        void remove_packet() { 

			if (!buff.empty()) {
				buff.pop();
			}
		}

        // checks to see if buffer is empty
        bool is_empty() { return buff.empty(); }

    private:
        std::queue<const char*> buff; // buffer for packets
};

#endif