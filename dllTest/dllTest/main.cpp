#include <iostream>
#include "DeviceDemo_1.h"
using namespace std;

void myCallback(void)
{
	//printf("--->>�����ص�\n");   ok
}


int main()
{

	TY_DEVICE_INFO devInfo;

	memset(&devInfo, 0, sizeof(TY_DEVICE_INFO));
	sprintf_s(devInfo.deviceName, "dev1");
	sprintf_s(devInfo.protocol, "DDS");



	initLibrary();
	setDeviceInfo(devInfo);  // �����豸����
	TY_DEVICE_INFO  rt = getDeviceInfo();
	cout << "--name��" << devInfo.deviceName << endl;
	cout << "---proto��" << devInfo.protocol << endl;

	connectDevice();
	RegisterCallback(myCallback);

	startCapture();
	// ... ҵ���� ...

	getchar();
	stopCapture();
	disConnectDevice();
	destroyLibrary();

	return 0;
}