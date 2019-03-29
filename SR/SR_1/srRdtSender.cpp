#include "stdafx.h"
#include "srRdtSender.h"
#include "Global.h"
#pragma warning(disable:4996);

FILE *SenderLog = fopen("SenderLog.txt", "w");
srRdtSender::srRdtSender():seqRange(8), winLen(4), base(0), nextseqnum(0)//序号比特数3，序号空间长度为8，窗口大小为8
{
	Window = new deque<sndElem>;//初始化窗口
}


srRdtSender::~srRdtSender()
{
	delete Window;
}

bool srRdtSender::getWaitingState() {
	return (Window->size() == winLen);
}

bool srRdtSender::send(Message& Msg) {
	if (this->getWaitingState()) {
		return false;
	}

	sndElem newElem;
	newElem.mark = false;		//新数据包标记为flase
	newElem.Pkt.acknum = -1;	//忽略该字段
	newElem.Pkt.seqnum = this->nextseqnum;
	newElem.Pkt.checksum = 0;
	memcpy(newElem.Pkt.payload, Msg.data, sizeof(Msg.data));
	newElem.Pkt.checksum = pUtils->calculateCheckSum(newElem.Pkt);

	Window->push_back(newElem);//将新的数据包加入窗口

	printf("发送方发送数据报文：seqnum = %3d\n",newElem.Pkt.seqnum);
	fprintf(SenderLog, "发送方发送数据报文：seqnum = %3d\n", newElem.Pkt.seqnum);
	pns->startTimer(SENDER, Configuration::TIME_OUT, newElem.Pkt.seqnum);
	pns->sendToNetworkLayer(RECEIVER, newElem.Pkt);
	++(this->nextseqnum) %= this->seqRange;

	return true;
}

void srRdtSender::receive(Packet& ackPkt) {
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	int offset = (ackPkt.acknum - base + seqRange) % seqRange;//ack序号与基序号的偏移量

	//检验和正确，并且acknum对应的数据包在窗口内
	if (checkSum == ackPkt.checksum && offset < this->Window->size() && Window->at(offset).mark == false) {
		pns->stopTimer(SENDER, ackPkt.acknum);	
		Window->at(offset).mark = true;			//标记该数据包为已接受
		printf("\n发送方收到ACK，acknum= %3d，并将窗口内对应报文标记\n",ackPkt.acknum);
		fprintf(SenderLog, "\n发送方收到ACK，acknum= %3d，并将窗口内对应报文标记\n", ackPkt.acknum);
		
		printf("窗口信息：base = %3d,nextseqnum = %3d,windowSize = %3d\n",base,nextseqnum,Window->size());
		fprintf(SenderLog, "窗口信息：base = %3d,nextseqnum = %3d,windowSize = %3d\n", base, nextseqnum, Window->size());
		
		if (Window->size()) {//输出窗口内容信息，若不需要调试分析可将次分支注释
			printf("窗口内已经确认的报文序号有：");
			fprintf(SenderLog,"\n窗口内已经确认的报文序号有：");
			for (auto it = Window->begin();it != Window->end();++it){
				if (it->mark) {
					printf("%3d",it->Pkt.seqnum);
					fprintf(SenderLog, "%3d", it->Pkt.seqnum);
				}
			}
			printf("\n\n");
			fprintf(SenderLog, "\n\n");
		}

		
		printf("滑动窗口前，base = %3d,nextseqnum = %3d,windowSize = %3d\n",base,nextseqnum,Window->size());
		fprintf(SenderLog, "滑动窗口前，base = %3d,nextseqnum = %3d,windowSize = %3d\n", base, nextseqnum, Window->size());
		while(Window->size() && Window->begin()->mark == true) {//如果该数据包为窗口最左边的包，则滑动窗口
			++(this->base) %= this->seqRange;  //基序号+1
			Window->pop_front();
		}
		printf("滑动窗口后，base = %3d,nextseqnum = %3d,windowSize = %3d\n", base, nextseqnum, Window->size());
		fprintf(SenderLog, "滑动窗口后，base = %3d,nextseqnum = %3d,windowSize = %3d\n", base, nextseqnum, Window->size());
	}
}

void srRdtSender::timeoutHandler(int seqNum) {
	pns->stopTimer(SENDER,seqNum);

	int offset = (seqNum - base + seqRange) % seqRange;
	printf("\n超时，重新发送超时的数据报文,报文序号：%3d\n",seqNum);
	fprintf(SenderLog, "\n超时，重新发送超时的数据报文：%3d\n",seqNum);
	//如果超时的包在窗口内,则重发数据包
	if (offset < Window->size()) {
		pns->sendToNetworkLayer(RECEIVER, Window->at(offset).Pkt);
		pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);
	}
}