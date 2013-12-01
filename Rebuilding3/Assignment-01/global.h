#ifndef GLOBAL_H
#define GLOBAL_H

#include <Windows.h>

#define NUL  0x00
#define EOT  0x04
#define ENQ  0x05
#define ACK  0x06
#define SOT1 0x11
#define SOT2 0x12
#define NAK  0x15
#define SYN  0x16

class Stats {
public:
	Stats():totalPacketsSent_(0),
			totalPacketsReceived_(0),
			totalErrors_(0),
			totalACKsReceived_(0),
			totalACKsSent_(0),
			totalNAKsReceived_(0),
			totalNAKsSent_(0),
			totalTimeouts_(0),
			totalPadding_(0),
			totalRequests_(0),
			elapsedTime_(0),
			usefulBitsSent_(0),
			usefulBitsReceived_(0),
			totalResponseTime_(0) {}

	double SendProtocolEfficiency() const { 
		if (((((totalPacketsSent_ * (1024 - 48)) - totalPadding_)) == 0) || (totalPacketsSent_ * 1024) == 0) {
			return 0;
		} else {
			return ( ((double)((totalPacketsSent_ * (1024 - 48)) - totalPadding_)) / (totalPacketsSent_ * 1024) );
		}
	}
	double ReceiveProtocolEfficiency() const { 
		if (((((totalPacketsReceived_ * (1024 - 48)) - totalPadding_)) == 0) || (totalPacketsReceived_ * 1024) == 0) {
			return 0;
		} else {
			return ( ((double)((totalPacketsReceived_ * (1024 - 48)) - totalPadding_)) / (totalPacketsReceived_ * 1024) ); 
		}
	}
	int EffectiveSendBPS() const { return ((int)(1024 * SendProtocolEfficiency())); }
	int EffectiveReceiveBPS() const { return ((int)(1024 * ReceiveProtocolEfficiency())); }
	int NumPacketsSent() const { return totalPacketsSent_; }
	int BitErrorRate() const { 
		if ((totalPacketsSent_ + totalPacketsReceived_) == 0 || totalErrors_ == 0) {
			return 0;
		} else {
			return (totalErrors_/(totalPacketsSent_ + totalPacketsReceived_));
		}
	}
	int NumACKsReceived() const { return totalACKsReceived_; }
	int NumACKsSent() const { return totalACKsSent_; }
	int NumNAKsReceived() const { return totalNAKsReceived_; }
	int NumNAKsSent() const { return totalNAKsSent_; }
	int TotalBitsSent() const { return (1024 * totalPacketsSent_); }
	int TotalBitsReceived() const { return (1024 * totalPacketsReceived_); }
	int ResponseTime() const { 
		if (totalResponseTime_ == 0 || totalPacketsReceived_ == 0) {
			return 0;
		} else {
			return totalResponseTime_/totalPacketsReceived_; 
		}
	}
	int ElapsedTransferTime() const { return elapsedTime_; }
	int TotalTimeouts() const { return totalTimeouts_; }
	int TotalRequests() const { return totalRequests_; }
	int AverageSendPadding() const { 
		if (totalPacketsSent_ == 0 || totalPadding_ == 0) {
			return 0;
		} else {
			return (totalPacketsSent_/totalPadding_); 
		}
	}
	int AverageReceivePadding() const { 
		if (totalPacketsReceived_ == 0 || totalPadding_ == 0) {
			return 0;
		} else {
			return (totalPacketsReceived_/totalPadding_); 
		}
	}
	int PacketsSentPerSecond() const { 
		if (totalPacketsSent_ == 0 || elapsedTime_ == 0) {
			return 0;
		} else {
			return (totalPacketsSent_/elapsedTime_); 
		}
	}
	int PacketsReceivedPerSecond() const { 
		if (totalPacketsReceived_ == 0 || elapsedTime_ == 0) {
			return 0;
		} else {
			return (totalPacketsReceived_/elapsedTime_); 
		}
	}
	int totalPacketsSent_;
	int totalPacketsReceived_;
	int totalErrors_;
	int totalACKsReceived_;
	int totalACKsSent_;
	int totalNAKsReceived_;
	int totalNAKsSent_;
	int totalTimeouts_;
	int totalPadding_;
	int totalRequests_;
	int elapsedTime_;
	int usefulBitsSent_;
	int usefulBitsReceived_;
	int totalResponseTime_;
};

typedef struct Globals {

	HANDLE* hSem;
	HANDLE* hComm;
	OVERLAPPED ov;

	bool gotAck;

} Globals;

#endif