#include <string>
#include <queue>

class Buffer
{
    public:
        Buffer(){}

        // adds a packet to the queue
        virtual void add_packet(const char* packet){ buff.push(packet); }

        // retrieves a packet and pops it off the buffer
        virtual char* get_packet() 
        {
            if(!buff.empty())
                return buff.front();
            else
                std::cerr << "buffer empty" << std::endl;
            return NULL;
        }
        // removes packet from queue
        virtual void remove_packet() { buff.pop();}
        // checks to see if buffer is empty
        virtual bool is_empty() { return buff.empty(); }

    private:
        std::queue<char*> buff; // buffer for packets
}