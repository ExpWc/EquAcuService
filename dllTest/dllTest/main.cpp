#include <iostream>
#include "DeviceDemo_1.h"
using namespace std;

void myCallback(void)
{
	//printf("--->>触发回调\n");   ok
}


int main()
{

	TY_DEVICE_INFO devInfo;

	memset(&devInfo, 0, sizeof(TY_DEVICE_INFO));
	sprintf_s(devInfo.deviceName, "dev1");
	sprintf_s(devInfo.protocol, "DDS");



	initLibrary();
	setDeviceInfo(devInfo);  // 设置设备参数
	TY_DEVICE_INFO  rt = getDeviceInfo();
	cout << "--name：" << devInfo.deviceName << endl;
	cout << "---proto：" << devInfo.protocol << endl;

	connectDevice();
	RegisterCallback(myCallback);

	startCapture();
	// ... 业务处理 ...

	getchar();
	stopCapture();
	disConnectDevice();
	destroyLibrary();

	return 0;
}