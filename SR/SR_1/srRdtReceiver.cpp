#include "stdafx.h"
#include "Global.h"
#include "srRdtReceiver.h"
#pragma warning(disable:4996);

FILE *ReceiverLog = fopen("ReceiverLog.txt", "w");
srRdtReceiver::srRdtReceiver() :seqRange(8), winLen(4),base(0)
{
	AckPkt.acknum = 0;
	AckPkt.checksum = 0;
	AckPkt.seqnum = -1;
	int i;
	for (i = 0;i < Configuration::PAYLOAD_SIZE;++i) {
		AckPkt.payload[i] = '.';
	}
	AckPkt.checksum = pUtils->calculateCheckSum(AckPkt);

	Window = new deque<rcvElem>;//初始化窗口

	for (i = 0;i < winLen;++i){
		rcvElem blankElem;//生成未标记的数据包占满窗口
		blankElem.mark = false;
		Window->push_back(blankElem);
	}
}


srRdtReceiver::~srRdtReceiver()
{
	delete Window;
}

void srRdtReceiver::receive(Packet& Pkt) {

	Packet dataPacket;
	dataPacket.acknum &= base;
	dataPacket.seqnum = Window->size();
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(Pkt);

	int offset = (Pkt.seqnum - base + seqRange) % seqRange;
	//如果校验和正确
	if (checkSum == Pkt.checksum) {
		printf("接收方收到数据报文,seqnum = %3d\n", Pkt.seqnum);
		fprintf(ReceiverLog, "接收方收到数据报文,seqnum = %3d\n", Pkt.seqnum);
		//如果数据包应该位于窗口内则将其信息缓存
		if (offset < winLen) {
			printf("将数据报文缓存,与base的距离offset = %3d\n", offset);
			fprintf(ReceiverLog, "将数据报文缓存,与base的距离offset = %3d\n", offset);
			Window->at(offset).mark = true;
			memcpy(Window->at(offset).Msg.data, Pkt.payload, sizeof(Pkt.payload));
		}

		//发送确认ACK
		AckPkt.acknum = Pkt.seqnum;//确认号为数据包序号
		AckPkt.checksum = pUtils->calculateCheckSum(AckPkt);
		
		printf("接收方发送ACK，acknum = %3d\n", AckPkt.acknum);
		fprintf(ReceiverLog, "接收方发送ACK，acknum = %3d\n", AckPkt.acknum);
		pns->sendToNetworkLayer(SENDER, AckPkt);

		if (Window->size()) {//输出窗口内报文信息，当不需要调试分析时可以将该分支注释
			printf("窗口内已经缓存的报文序号有：");
			fprintf(ReceiverLog, "\n窗口内已经缓存的报文序号有：");
			for (auto it = Window->begin();it != Window->end();++it) {
				int seq = (it - Window->begin() + base) % seqRange;
				if (it->mark) {
					printf("%3d",seq);
					fprintf(ReceiverLog, "%3d", seq);
				}
			}
			printf("\n");
			fprintf(ReceiverLog, "\n");
		}
		


		//当窗口最左边的信息已经确认时，向上层递交,同时滑动窗口
		printf("\n滑动窗口前:base = %3d\n", base);
		fprintf(ReceiverLog, "\n滑动窗口前:base = %3d\n", base);
		while (Window->begin()->mark == true) {
			pns->delivertoAppLayer(RECEIVER, Window->begin()->Msg);
			Window->pop_front();

			//添加未标记的元素占满窗口
			rcvElem blankElem;
			blankElem.mark = false;
			Window->push_back(blankElem);

			++(this->base) %= seqRange;
		}
		printf("滑动窗口后:base = %3d\n\n", base);
		fprintf(ReceiverLog, "滑动窗口后:base = %3d\n\n", base);
	}
	else
		printf("接收方收到了校验错误的数据报文\n\n");
		fprintf(ReceiverLog, "接收方收到了校验错误的数据报文\n\n");
}