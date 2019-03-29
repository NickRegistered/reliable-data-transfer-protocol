#ifndef SR_RDT_RECEIVER_H
#define SR_RDT_RECEIVER_H
#include "RdtReceiver.h"
#include <queue>
using namespace std;

struct rcvElem {
	bool mark;
	Message Msg;
};

class srRdtReceiver:public RdtReceiver
{
private:
	const int seqRange; //序号范围
	const int winLen;	//窗口长度
	int base;			//窗口基序号

	Packet AckPkt;
	deque<rcvElem> *Window;//接收缓冲窗口
public:
	srRdtReceiver();
	virtual ~srRdtReceiver();
public:
	void receive(Packet&);
};

#endif