// SR_1.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "srRdtSender.h"
#include "srRdtReceiver.h"

int main(int argc, char** argv[])
{
	RdtSender *ps = new srRdtSender();
	RdtReceiver * pr = new srRdtReceiver();
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("input.txt");
	pns->setOutputFile("output.txt");
	pns->start();

	delete ps;
	delete pr;
	delete pUtils;									//指向唯一的工具类实例，只在main函数结束前delete
	delete pns;										//指向唯一的模拟网络环境类实例，只在main函数结束前delete
	system("pause");
	return 0;
}
