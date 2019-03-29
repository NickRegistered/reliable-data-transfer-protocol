#ifndef GBN_RDT_RECEIVER_H
#define GBN_RDT_RECEIVER_H
#include "RdtReceiver.h"
class gbnRdtReceiver:public RdtReceiver
{
private:
	int expectSeqNum;
	Packet lastAckPkt;

public:
	gbnRdtReceiver();
	virtual ~gbnRdtReceiver();
	
public:
	void receive(Packet&);
};

#endif