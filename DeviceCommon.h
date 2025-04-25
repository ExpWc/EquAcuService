#pragma once
#pragma execution_character_set("utf-8")
#ifndef DLL_API
#ifdef DLL_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif
#endif // !DLL_API

#define STRING_CHAR_SIZE 256				   // 默认字符串长度

enum DEVICE_GET_ERROR_CODE {
	DEVICE_GET_SUCCESS = 0,
	DEVICE_GET_ERROR_XX = -1,
	DEVICE_GET_ERROR_XXX = -2,
	DEVICE_GET_ERROR_OTHER = -3,
};

enum DEVICE_STATUS_CODE
{
	DEVICE_STATUS_OFFLINE = 0,                // 离线
	DEVICE_STATUS_ONLINE,                     // 在线-空闲状态
	DEVICE_STATUS_CAPTURE,                    // 采集中
	DEVICE_STATUS_ERROR,
};

extern "C" {
	typedef struct DeviceInfo
	{
		char deviceName[STRING_CHAR_SIZE];		// 设备名
		char protocol[128];                     // 协议类型：TCP/UDP/DDS...
		char ip[16];                            // 设备IP（IPV4）
		int port;                               // 设备端口号
		int searchRangeMs;                      // 默认500ms
		int storageDeepthMs;                    // 融合数据存储深度，ms
	}TY_DEVICE_INFO;

	typedef void (*DataCallback)(void);
}