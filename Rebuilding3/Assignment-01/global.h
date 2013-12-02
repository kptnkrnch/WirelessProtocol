#ifndef GLOBAL_H
#define GLOBAL_H

#include "Windows.h"

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

	void SetTotalPacketsSent(int totalPacketsSent){ //1
		totalPacketsSent_ += totalPacketsSent; 
	}
	void SetTotalPacketsReceived(int totalPacketsReceived){ //2
		totalPacketsReceived_ += totalPacketsReceived; 
	}
	void SetTotalErrors(int totalErrors){ //3
		totalErrors_ += totalErrors; 
	}
	void SetTotalACKsReceived(int totalACKsReceived){ //4
		totalACKsReceived_ += totalACKsReceived; 
	}
	void SetTotalACKsSent(int totalACKsSent){ //5
		totalACKsSent_ += totalACKsSent; 
	}
	void SetTotalNAKsReceived(int totalNAKsReceived){ //6
		totalNAKsReceived_ += totalNAKsReceived; 
	}
	void SetTotalNAKsSent(int totalNAKsSent){ //7
		totalNAKsSent_ += totalNAKsSent; 
	}
	void SetTotalTimeouts(int totalTimeouts){ //8
		totalTimeouts_ += totalTimeouts; 
	}
	void SetTotalPadding(int totalPadding){ //9
		totalPadding_ += totalPadding; 
	}
	void SetTotalRequests(int totalRequests){ //10
		totalRequests_ += totalRequests; 
	}
	void SetElapsedTime(int elapsedTime){ //11
		elapsedTime_ += elapsedTime; 
	}
	void SetUsefulBitsSent(int usefulBitsSent){ //12
		usefulBitsSent_ += usefulBitsSent; 
	}
	void SetUsefulBitsReceived(int usefulBitsReceived){ //13
		usefulBitsReceived_ += usefulBitsReceived; 
	}
	void SetTotalResponseTime(int totalResponseTime){ //14
		totalResponseTime_ += totalResponseTime; 
	}

	int GetTotalPacketsSent(){ return totalPacketsSent_; } //15
	int GetTotalPacketsReceived(){ return totalPacketsReceived_; } //16
	int GetTotalErrors(){ return totalErrors_; }//17
	int GetTotalACKsReceived(){ return totalACKsReceived_; }//18
	int GetTotalACKsSent(){ return totalACKsSent_; }//19
	int GetTotalNAKsReceived(){ return totalNAKsReceived_; }//20
	int GetTotalNAKsSent(){ return totalNAKsSent_; }//21
	int GetTotalTimeouts(){ return totalTimeouts_; }//22
	int GetTotalPadding(){ return totalPadding_; }//23
	int GetTotalRequests(){ return totalRequests_; }//24
	int GetElapsedTime(){ return elapsedTime_; }//25
	int GetUsefulBitsSent(){ return usefulBitsSent_; }//26
	int GetUsefulBitsReceived(){ return usefulBitsReceived_; }//27
	int GetTotalResponseTime(){ return totalResponseTime_; }//28

	int SendProtocolEfficiency() const { //29
		if (((((totalPacketsSent_ * (1024 - 48)) - totalPadding_)) == 0) || (totalPacketsSent_ * 1024) == 0) {
			return 0;
		} else {
			return ( (int)(100*((double)((totalPacketsSent_ * (1024 - 48)) - totalPadding_)) / (totalPacketsSent_ * 1024) ));
		}
	}
	int ReceiveProtocolEfficiency() const { //30
		if (((((totalPacketsReceived_ * (1024 - 48)) - totalPadding_)) == 0) || (totalPacketsReceived_ * 1024) == 0) {
			return 0;
		} else {
			return ( (int)(100*((double)((totalPacketsReceived_ * (1024 - 48)) - totalPadding_)) / (totalPacketsReceived_ * 1024) )); 
		}
	}
	int EffectiveSendBPS() const { return ((int)(1024 * SendProtocolEfficiency())); }//31
	int EffectiveReceiveBPS() const { return ((int)(1024 * ReceiveProtocolEfficiency())); }//32
	int NumPacketsSent() const { return totalPacketsSent_; }//33
	int BitErrorRate() const { //34
		if ((totalPacketsSent_ + totalPacketsReceived_) == 0 || totalErrors_ == 0) {
			return 0;
		} else {
			return (totalErrors_/(totalPacketsSent_ + totalPacketsReceived_));
		}
	}
	int NumACKsReceived() const { return totalACKsReceived_; }//35
	int NumACKsSent() const { return totalACKsSent_; }//36
	int NumNAKsReceived() const { return totalNAKsReceived_; }//37
	int NumNAKsSent() const { return totalNAKsSent_; }//38
	int TotalBitsSent() const { return (1024 * totalPacketsSent_); }//39
	int TotalBitsReceived() const { return (1024 * totalPacketsReceived_); }//40
	int ResponseTime() const { //41
		if (totalResponseTime_ == 0 || totalPacketsReceived_ == 0) {
			return 0;
		} else {
			return totalResponseTime_/totalPacketsReceived_; 
		}
	}
	int ElapsedTransferTime() const { return elapsedTime_; }//42
	int TotalTimeouts() const { return totalTimeouts_; }//43
	int TotalRequests() const { return totalRequests_; }//44
	int AverageSendPadding() const { //45
		if (totalPacketsSent_ == 0 || totalPadding_ == 0) {
			return 0;
		} else {
			return (totalPacketsSent_/totalPadding_); 
		}
	}
	int AverageReceivePadding() const { //46
		if (totalPacketsReceived_ == 0 || totalPadding_ == 0) {
			return 0;
		} else {
			return (totalPacketsReceived_/totalPadding_); 
		}
	}
	int PacketsSentPerSecond() const { //47
		if (totalPacketsSent_ == 0 || elapsedTime_ == 0) {
			return 0;
		} else {
			return (totalPacketsSent_/elapsedTime_); 
		}
	}
	int PacketsReceivedPerSecond() const { //48
		if (totalPacketsReceived_ == 0 || elapsedTime_ == 0) {
			return 0;
		} else {
			return (totalPacketsReceived_/elapsedTime_); 
		}
	}


private:
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



int HandleStats(CRITICAL_SECTION& section, Stats& stats, int cmd, int value);/* {
	EnterCriticalSection(&section);
	switch(cmd) {
	case 1:
		stats.SetTotalPacketsSent(value);
	break;
	case 2:
		stats.SetTotalPacketsReceived(value);
	break;
	case 3:
		stats.SetTotalErrors(value);
	break;
	case 4:
		stats.SetTotalACKsReceived(value);
	break;
	case 5:
		stats.SetTotalACKsSent(value);
	break;
	case 6:
		stats.SetTotalNAKsReceived(value);
	break;
	case 7:
		stats.SetTotalNAKsSent(value);
	break;
	case 8:
		stats.SetTotalTimeouts(value);
	break;
	case 9:
		stats.SetTotalPadding(value);
	break;
	case 10:
		stats.SetTotalRequests(value);
	break;
	case 11:
		stats.SetElapsedTime(value);
	break;
	case 12:
		stats.SetUsefulBitsSent(value);
	break;
	case 13:
		stats.SetUsefulBitsReceived(value);
	break;
	case 14:
		stats.SetTotalResponseTime(value);
	break;
	case 15:
		return stats.GetTotalPacketsSent();
	break;
	case 16:
		return stats.GetTotalPacketsReceived();
	break;
	case 17:
		return stats.GetTotalErrors();
	break;
	case 18:
		return stats.GetTotalACKsReceived();
	break;
	case 19:
		return stats.GetTotalACKsSent();
	break;
	case 20:
		return stats.GetTotalNAKsReceived();
	break;
	case 21:
		return stats.GetTotalNAKsSent();
	break;
	case 22:
		return stats.GetTotalTimeouts();
	break;
	case 23:
		return stats.GetTotalPadding();
	break;
	case 24:
		return stats.GetTotalRequests();
	break;
	case 25:
		return stats.GetElapsedTime();
	break;
	case 26:
		return stats.GetUsefulBitsSent();
	break;
	case 27:
		return stats.GetUsefulBitsReceived();
	break;
	case 28:
		return stats.GetTotalResponseTime();
	break;
	case 29:
		return stats.SendProtocolEfficiency();
	break;
	case 30:
		return stats.ReceiveProtocolEfficiency();
	break;
	case 31:
		return stats.EffectiveSendBPS();
	break;
	case 32:
		return stats.EffectiveReceiveBPS();
	break;
	case 33:
		return stats.NumPacketsSent();
	break;
	case 34:
		return stats.BitErrorRate();
	break;
	case 35:
		return stats.NumACKsReceived();
	break;
	case 36:
		return stats.NumACKsSent();
	break;
	case 37:
		return stats.NumNAKsReceived();
	break;
	case 38:
		return stats.NumNAKsSent();
	break;
	case 39:
		return stats.TotalBitsSent();
	break;
	case 40:
		return stats.TotalBitsReceived();
	break;
	case 41:
		return stats.ResponseTime();
	break;
	case 42:
		return stats.ElapsedTransferTime();
	break;
	case 43:
		return stats.TotalTimeouts();
	break;
	case 44:
		return stats.TotalRequests();
	break;
	case 45:
		return stats.AverageSendPadding();
	break;
	case 46:
		return stats.AverageReceivePadding();
	break;
	case 47:
		return stats.PacketsSentPerSecond();
	break;
	case 48:
		return stats.PacketsReceivedPerSecond();
	break;
	default:
	break;
	}
	LeaveCriticalSection(&section);
	return 0;
}*/
#endif