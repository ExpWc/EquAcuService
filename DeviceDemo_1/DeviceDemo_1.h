#pragma once
#include "DeviceCommon.h"

extern "C" {
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
	// 获取输出值-返回值：不同错误码
	DLL_API DEVICE_GET_ERROR_CODE getCaptureValue(
		const char* varName,       // 变量名 (IN
		unsigned long long msec,   // 时间 (IN)
		double* valueArray,        // 值数组 (OUT)
		int maxSize,               // 传出数据长度 (OUT)
		int* arraySize             // 值数组最大大小 (IN)
	);
	// 数据融合接口
	bool DataFusion();
}