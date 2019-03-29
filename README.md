# reliable-data-transfer-protocol
数据可靠传输协议，包括SR、GBN和基于GBN的TCP协议

&emsp;项目使用VS2017开发,请在stdafx.h头文件第16行的“#pragma comment(lib,"F:\\计网实验\\rdt-win-student\\lib\\netsimlib.lib")”
代码处将路径改为您的netsimlib.lib所在路径。

&emsp;netsimlib.lib模拟了网络环境，随机发生丢包乱序，具体使用方法请参阅模块二 可靠传输协议设计.pdf

&emsp;StopWait是本实验的一个Demo，实现了停等协议。需要在此Demo的框架下设计不同的Sender类（继承RdtSender）和Receiver类（继承RdtReceiver）
并实现这两个类提供给网络环境和上层应用的接口。

&emsp;关于SR、GBN和TCP协议请参阅《计算机网络》
