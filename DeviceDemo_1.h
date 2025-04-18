#pragma once
#pragma execution_character_set("utf-8")

#ifndef DLL_API
#ifdef DLL_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif
#endif // !DLL_API



extern "C" {
	typedef struct DeviceInfo
	{
		char* deviceName[256];		// 设备名
		char* protocol[128];        // 协议类型：TCP/UDP/DDS...
		char* ip[16];               // 设备IP（IPV4）
		int port;                   // 设备端口号
	}TY_DEVICE_INFO;

	typedef struct DeviceStatus
	{
		bool connection;            // 连接状态: ture-已连接; false-未连接
	}TY_DEVICE_STATUS;

	typedef void (*DataCallback)(void);

	DLL_API bool initLibrary();
	DLL_API bool destroyLibrary();

	// 获取错误信息
	DLL_API void getLastErrorMsg(char* errMsg);

	// 获取设备状态
	DLL_API TY_DEVICE_STATUS getDeviceStatus();

	// 获取/设置 设备信息
	DLL_API TY_DEVICE_INFO getDeviceInfo();
	DLL_API bool setDeviceInfo(TY_DEVICE_INFO info);

	// 连接设备
	DLL_API bool connectDevice();
	// 断开设备
	DLL_API bool disConnectDevice();
	// 注册回调函数
	DLL_API void RegisterCallback(DataCallback callbackFunc);
	// 开始采集:（封装库内部新建一个线程，接收到数据时调用回调函数。注意线程需要与其他接口加锁）
	DLL_API bool startCapture();
	DLL_API bool stopCapture();
	// 获取所有输出变量名
	DLL_API int getCaptureOutputVarNames(char** outputVarNames, int maxSize);
	// 获取输出值
	DLL_API int getCaptureValue(const char* varName, double* valueArray, int maxSize);
	// 融合目标接口
	bool DataFusion(unsigned long long time);
}