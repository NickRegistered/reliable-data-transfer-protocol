#include "stdafx.h"
#include "Global.h"
#include "gbnRdtReceiver.h"
#pragma warning(disable:4996)

FILE *ReceiverLog = fopen("ReceiverLog.txt", "w");

gbnRdtReceiver::gbnRdtReceiver():expectSeqNum(0)
{
	lastAckPkt.acknum = (expectSeqNum + 7) % 8;//约定以希望接收到的序号前一个数表示nck
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//忽略该字段
	for (int i = 0; i < Configuration::PAYLOAD_SIZE;i++) {
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}


gbnRdtReceiver::~gbnRdtReceiver()
{

}

void gbnRdtReceiver::receive(Packet &packet) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(packet);

	//如果校验和正确，同时收到的包序号为希望序号
	if (checkSum == packet.checksum && this->expectSeqNum == packet.seqnum) {
		printf("\n接收方收到报文，seqnum = %3d\n",packet.seqnum);
		fprintf(ReceiverLog, "\n接收方收到报文，seqnum = %3d\n", packet.seqnum);
		//抽取Message，向上层递交
		Message Msg;
		memcpy(Msg.data, packet.payload, sizeof(packet.payload));
		pns->delivertoAppLayer(RECEIVER, Msg);

		lastAckPkt.acknum = packet.seqnum;//确认序号为数据包序号
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		printf("\n接收方发送ACK，acknum = %3d\n", lastAckPkt.acknum);
		fprintf(ReceiverLog, "\n接收方发送ACK，acknum = %3d\n", lastAckPkt.acknum);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);

		++(this->expectSeqNum) %= 8;
	}
	else {
		if (checkSum != packet.checksum) {
			printf("\n接收方收到了校验错误的数据报文\n");
			fprintf(ReceiverLog, "\n接收方收到了校验错误的数据报文\n");		
		}
		else {
			printf("\n接收方收到了失序的数据报文,其序号为%3d\n",packet.seqnum);
			fprintf(ReceiverLog, "\n接收方收到了失序的数据报文,其序号为%3d\n", packet.seqnum);		
		}
		printf("\n接收方重新发送ack，acknum = %3d\n",lastAckPkt.acknum);
		fprintf(ReceiverLog,"\n接收方重新发送ack，acknum = %3d\n", lastAckPkt.acknum);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送上次的确认报文

	}
}