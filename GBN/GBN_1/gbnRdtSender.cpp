#include "stdafx.h"
#include "gbnRdtSender.h"
#include "Global.h"
#pragma warning(disable:4996)

FILE *SenderLog = fopen("SenderLog.txt", "w");
gbnRdtSender::gbnRdtSender():seqRange(8),winLen(4),base(0),nextseqnum(0)//序号比特数3，序号空间长度为8，窗口大小为4
{
	Window = new deque<Packet>;//初始化窗口
}


gbnRdtSender::~gbnRdtSender()
{
	delete Window;
}

bool gbnRdtSender::getWaitingState() {
	return (Window->size() == winLen);//当窗口长度达到限定时等待
}

bool gbnRdtSender::send(Message& Msg) {
	if (this->getWaitingState()) {
		return false;
	}

	Packet newPacket;
	newPacket.acknum = -1;//忽略该字段
	newPacket.seqnum = this->nextseqnum;
	newPacket.checksum = 0;
	memcpy(newPacket.payload, Msg.data, sizeof(Msg.data));
	newPacket.checksum = pUtils->calculateCheckSum(newPacket);

	Window->push_back(newPacket);//将新的包加入窗口
	
	printf("发送方发送数据报文,seqnum = %3d\n",newPacket.seqnum);
	fprintf(SenderLog, "发送方发送数据报文,seqnum = %3d\n", newPacket.seqnum);
	if (this->base == this->nextseqnum){
		pns->startTimer(SENDER, Configuration::TIME_OUT, newPacket.seqnum);
	}
	pns->sendToNetworkLayer(RECEIVER, newPacket);
	++(this->nextseqnum) %= this->seqRange;

	return true;
}

void gbnRdtSender::receive(Packet &ackPkt) {
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//校验和正确，序号为已发送并等待确认的包(约定当接收方没收到期望的序号时，ack为最近接收到的序号)
	if (checkSum == ackPkt.checksum && ackPkt.acknum != (this->base + 7) % 8 ) {
		
		pns->stopTimer(SENDER, base);//停止计时器

		printf("\n发送方收到ACK：acknum = %3d\n滑动窗口前:base=%3d,nextseqnum=%3d,windowSize=%3d", ackPkt.acknum, base, nextseqnum, Window->size());
		fprintf(SenderLog,"\n发送方收到ACK：acknum = %3d\n滑动窗口前:base=%3d,nextseqnum=%3d,windowSize=%3d", ackPkt.acknum, base, nextseqnum, Window->size());
		//滑动窗口
		while (base != (ackPkt.acknum + 1) % seqRange)
		{
			Window->pop_front();
			++base %= seqRange;			
		}
		printf("\n滑动窗口后:base=%3d,nextseqnum=%3d,windowSize=%3d\n", base, nextseqnum, Window->size());
		fprintf(SenderLog,"\n滑动窗口后:base=%3d,nextseqnum=%3d,windowSize=%3d\n\n", base, nextseqnum, Window->size());
		//如果还有未发送的包，则重启计时器
		if (Window->size()) {
			pns->startTimer(SENDER, Configuration::TIME_OUT, Window->front().seqnum);//以第一个包的序号开启计时器
		}		
	}
}

void gbnRdtSender::timeoutHandler(int seqNum) {
	
	printf("\n超时，重新发送窗口内数据报文，窗口信息:base=%3d,nextseqnum=%3d,windowSize=%3d\n", base, nextseqnum, Window->size());
	fprintf(SenderLog,"\n超时，重新发送窗口内数据报文，窗口信息:base=%3d,nextseqnum=%3d,windowSize=%3d\n", base, nextseqnum, Window->size());
	pns->stopTimer(SENDER, seqNum);
	for (deque<Packet>::iterator itor = Window->begin();itor != Window->end();++itor) {
		pns->sendToNetworkLayer(RECEIVER, *itor);
	}
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);
	
}