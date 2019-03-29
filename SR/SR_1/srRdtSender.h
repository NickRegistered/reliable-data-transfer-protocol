#ifndef SR_RDT_SENDER_H
#define SR_RDT_SENDER_H
#include "RdtSender.h"
#include <queue>
using namespace std;

struct sndElem {
	bool mark;		//已经确认的标记
	Packet Pkt;		
};

class srRdtSender:public RdtSender
{
private:
	const int seqRange;	//序号范围
	const int winLen;	//窗口长度
	int base;			//基序号
	int nextseqnum;		//下一个等待发送包的序号

	deque<sndElem> *Window;//使用队列实现SR协议中的窗口
public:
	bool getWaitingState();
	bool send(Message&);
	void receive(Packet&);
	void timeoutHandler(int);
public:
	srRdtSender();
	virtual ~srRdtSender();
};

#endif

