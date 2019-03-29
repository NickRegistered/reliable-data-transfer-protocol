#ifndef GBN_RDT_SENDER_H
#define GBN_RDT_SENDER_H
#include "RdtSender.h"
#include <queue>
using namespace std;
class gbnRdtSender:public RdtSender
{
private:
	const int seqRange;		//序号范围
	const int winLen;		//窗口长度
	int base;				//基序号
	int nextseqnum;			//下一个等待发送包的序号
	int redundancy;			//冗余ack个数

	deque<Packet> *Window;	//使用队列实现GBN中的窗口
public:

	bool getWaitingState();
	bool send(Message&);
	void receive(Packet&);
	void timeoutHandler(int);
public:
	gbnRdtSender();
	virtual ~gbnRdtSender();
};

#endif