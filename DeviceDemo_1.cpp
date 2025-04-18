#define _CRT_SECURE_NO_WARNINGS
#include "DeviceDemo_1.h"
#include "msgDef.h"
#include <Map>
#include <string>

//>>-
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <mutex>
#include <vector>
#include <algorithm>
#include <chrono>
#include <math.h>

#pragma comment(lib, "ws2_32.lib")
//<<

#define DEVICE_NAME "设备1"

static DataCallback g_dataCallback = nullptr;
static std::map<std::string, double*> g_dataMap;
static char g_errorMsg[256];
//static TY_DEVICE_INFO g_deviceInfo;

//>>def
std::mutex g_mutex;  // 用于线程同步
SOCKET g_socket = INVALID_SOCKET;
TY_DEVICE_INFO g_deviceInfo;
TY_DEVICE_STATUS g_deviceStatus = { false };
DataCallback g_callback = nullptr;
bool g_captureRunning = false;
std::thread g_captureThread;

// 数据缓存
std::vector<RFScnUpdataStruct> g_dataHistory;
RFScnUpdataStruct g_fusionCache;

// 错误信息
char g_lastErrorMsg[256] = { 0 };
//><<

//>>api
// 初始化Winsock
bool initWinsock() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		strcpy_s(g_lastErrorMsg, "WSAStartup failed");
		return false;
	}
	return true;
}

//<<

static inline void clearErrorMsg()
{
    if (g_errorMsg[0] != '\0') {
        memset(g_errorMsg, 0, sizeof(g_errorMsg));      // 清空错误信息
    }
}


DLL_API bool initLibrary()
{
    clearErrorMsg();
    // 初始化测试数据
#if 0
    double pA[10] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0 };
    double pB[10] = { 10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0 };
    double pC[10] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0 };
    g_dataMap.insert(std::pair<std::string, double*>("测试设备1/设备参数A", pA));
    g_dataMap.insert(std::pair<std::string, double*>("测试设备1/设备参数B", pB));
    g_dataMap.insert(std::pair<std::string, double*>("测试设备1/设备参数C", pC));
    return true;
#endif
	memset(&g_deviceInfo, 0, sizeof(g_deviceInfo));
	strcpy_s(g_deviceInfo.deviceName, "DefaultDevice");
	strcpy_s(g_deviceInfo.protocol, "TCP");
	strcpy_s(g_deviceInfo.ip, "127.0.0.1");
	g_deviceInfo.port = 8080;

	return true;

}


DLL_API bool destroyLibrary()
{
    clearErrorMsg();
#if 0
    for (auto& array : g_dataMap)
    {
        delete[] array.second;
    }
    return true;
#endif
	if (g_captureRunning) {
		stopCapture();
	}
	if (g_deviceStatus.connection) {
		disConnectDevice();
	}
	return true;

}

DLL_API void getLastErrorMsg(char* errMsg)
{
    //strcpy(errMsg, g_errorMsg);
	strcpy_s(errMsg, 256, g_lastErrorMsg);

}

DLL_API DeviceStatus getDeviceStatus()
{
    clearErrorMsg();
   // return DeviceStatus();

	std::lock_guard<std::mutex> lock(g_mutex);
	return g_deviceStatus;
}

DLL_API DeviceInfo getDeviceInfo()
{
    clearErrorMsg();

	std::lock_guard<std::mutex> lock(g_mutex);
	return g_deviceInfo;

   // return DeviceInfo();
}

DLL_API bool setDeviceInfo(DeviceInfo info)
{
    clearErrorMsg();


    return true;
}


DLL_API bool connectDevice()
{
    clearErrorMsg();

	//>>
	std::lock_guard<std::mutex> lock(g_mutex);

	if (g_deviceStatus.connection) {
		strcpy_s(g_lastErrorMsg, "Already connected");
		return true;
	}

#if 0
	if (!initWinsock()) {
		return false;
	}

	g_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_socket == INVALID_SOCKET) {
		strcpy_s(g_lastErrorMsg, "Socket creation failed");
		WSACleanup();
		return false;
	}

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(g_deviceInfo.port);
	inet_pton(AF_INET, g_deviceInfo.ip, &serverAddr.sin_addr);

	if (connect(g_socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		strcpy_s(g_lastErrorMsg, "Connection failed");
		closesocket(g_socket);
		WSACleanup();
		return false;
	}
#endif
	g_deviceStatus.connection = true;
	//<<
    return true;
}

DLL_API bool disConnectDevice()
{
    clearErrorMsg();

	//>>
	std::lock_guard<std::mutex> lock(g_mutex);

	if (!g_deviceStatus.connection) {
		strcpy_s(g_lastErrorMsg, "Not connected");
		return true;
	}

	closesocket(g_socket);
	WSACleanup();
	g_socket = INVALID_SOCKET;
	g_deviceStatus.connection = false;
	//<<
    return true;
}

DLL_API void RegisterCallback(DataCallback callbackFunc)
{
    clearErrorMsg();

	std::lock_guard<std::mutex> lock(g_mutex);
	g_callback = callbackFunc;
}
// 模拟数据采集线程
void captureThreadFunc() {
	while (g_captureRunning) {
		// 模拟数据采集
		RFScnUpdataStruct newData;
		auto now = std::chrono::system_clock::now();
		auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
		auto epoch = now_ms.time_since_epoch();
		auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
		long long timestamp = value.count();

		// 填充模拟数据
		newData.scnEquID = 1;
		newData.scnEquCourse = 45.0f;
		auto tt = std::chrono::system_clock::to_time_t(now);
		tm time;
		localtime_s(&time, &tt);
		newData.scnTime[0] = time.tm_year + 1900;
		newData.scnTime[1] = time.tm_mon + 1;
		newData.scnTime[2] = time.tm_mday;
		newData.scnTime[3] = time.tm_hour;
		newData.scnTime[4] = time.tm_min;
		newData.scnTime[5] = time.tm_sec;
		newData.scnTime[6] = (timestamp % 1000);

		newData.scnEmtCount = 1;
		EmtStruct& emt = newData.etp2[0];
		memset(&emt, 0, sizeof(EmtStruct));
		emt.scnPlatId = 1001;
		emt.scnEmtId = 2001;
		emt.scnPlatForce = 1;
		emt.scnPlatType = 6;
		emt.scnEmtType = 2;
		emt.scnTgtPos[0] = 116404000;
		emt.scnTgtPos[1] = 39915000;
		emt.scnTgtHight = 5000.0f;
		emt.scnTgtAzim = 45.0f;
		emt.scnTgtElev = 30.0f;
		emt.scnTgtSdist = 10000.0f;
		emt.scnTgtRadSpd = 200.0f;
		emt.scnTgtTagSpd = 50.0f;
		emt.scnSgnPowCode = -80.0f;
		emt.scnEmtWkMod = 1;
		emt.scnEmtPlorMod = 3;
		emt.scnSgnFM = 3;
		emt.scnAntScanProd = 5.0f;
		emt.scnBeamDwlTm = 2.0f;
		emt.scnIntfDur = 1.0f;
		emt.scnSgnRFTyp = 1;
		emt.scnSgnRFCnt = 1;
		emt.scnSgnRFMed = 3000.0f;
		emt.scnSgnRFRng = 100.0f;
		emt.scnSgnRFList[0] = 3000.0f;
		emt.scnSgnPRITyp = 1;
		emt.scnSgnPRICnt = 1;
		emt.scnSgnPRIMed = 1000.0f;
		emt.scnSgnPRIRng = 100.0f;
		emt.scnSgnPRIList[0] = 1000.0f;
		emt.scnSgnPWTyp = 1;
		emt.scnSgnPWCnt = 1;
		emt.scnSgnPWMed = 10.0f;
		emt.scnSgnPWRange = 2.0f;
		emt.scnSgnPWList[0] = 10.0f;
		emt.scnEmtWkModID = 1;

		// 添加到历史数据
		{
			std::lock_guard<std::mutex> lock(g_mutex);
			g_dataHistory.push_back(newData);
		}

		// 触发回调
		if (g_callback) {
			g_callback();
		}

		// 模拟采集间隔
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}


DLL_API bool startCapture()
{
    clearErrorMsg();

	//>>
	std::lock_guard<std::mutex> lock(g_mutex);

	if (g_captureRunning) {
		strcpy_s(g_lastErrorMsg, "Capture is already running");
		return false;
	}

	if (!g_deviceStatus.connection) {
		strcpy_s(g_lastErrorMsg, "Device is not connected");
		return false;
	}

	g_captureRunning = true;
	g_captureThread = std::thread(captureThreadFunc);
	//<<
   
    return true;
}

DLL_API bool stopCapture()
{
    clearErrorMsg();
	//>>
	{
		std::lock_guard<std::mutex> lock(g_mutex);
		if (!g_captureRunning) {
			strcpy_s(g_lastErrorMsg, "Capture is not running");
			return false;
		}
		g_captureRunning = false;
	}

	if (g_captureThread.joinable()) {
		g_captureThread.join();
	}
	//<<
    return true;
}

DLL_API int getCaptureOutputVarNames(char** outputVarNames, int maxSize)
{
    clearErrorMsg();
#if 0
    int varNum = g_dataMap.size();
    if (varNum > maxSize)
    {
        strcpy(g_errorMsg, "变量数量超过最大值");
        return -1;
    }
    int i = 0;
    for (auto it = g_dataMap.begin(); it != g_dataMap.end(); ++it)
    {
        strcpy(outputVarNames[i], it->first.c_str());
        i++;
    }
    return varNum;
#endif

	//>>
	const char* varNames[] = {
		"scnPlatId", "scnEmtId", "scnPlatForce", "scnPlatType", "scnEmtType",
		"scnTgtPos", "scnTgtHight", "scnTgtAzim", "scnTgtElev", "scnTgtSdist",
		"scnTgtRadSpd", "scnTgtTagSpd", "scnSgnPowCode", "scnEmtWkMod", "scnEmtPlorMod",
		"scnSgnFM", "scnAntScanProd", "scnBeamDwlTm", "scnIntfDur", "scnSgnRFTyp",
		"scnSgnRFCnt", "scnSgnRFMed", "scnSgnRFRng", "scnSgnRFList", "scnSgnPRITyp",
		"scnSgnPRICnt", "scnSgnPRIMed", "scnSgnPRIRng", "scnSgnPRIList", "scnSgnPWTyp",
		"scnSgnPWCnt", "scnSgnPWMed", "scnPWRange", "scnSgnPWList", "scnEmtWkModID"
	};

	//int count = std::min(maxSize, static_cast<int>(sizeof(varNames) / sizeof(varNames[0])));
	int count = (maxSize < static_cast<int>(sizeof(varNames) / sizeof(varNames[0]))? maxSize: static_cast<int>(sizeof(varNames) / sizeof(varNames[0])));

	for (int i = 0; i < count; i++) {
		outputVarNames[i] = _strdup(varNames[i]);
	}
	return count;
	//<<
}

DLL_API int getCaptureValue(const char* varName, double* valueArray, int maxSize)
{
    clearErrorMsg();
#if 0
	auto it = g_dataMap.find(varName);
    if (it == g_dataMap.end())
    {
        strcpy(g_errorMsg, "变量名不存在");
        return -1;
    }
    double* data = it->second;
    int size = sizeof(data) / sizeof(double);
    if (size > maxSize)
    {
        strcpy(g_errorMsg, "变量值数组大小不足");
        return -1;
    }
    for (int i = 0; i < size; i++)
    {
        valueArray[i] = data[i];
    }
    return size;
#endif

	if (g_dataHistory.empty()) {
		strcpy_s(g_lastErrorMsg, "No data available");
		return 0;
	}

	// 获取最新的融合数据
	RFScnUpdataStruct latestData = g_fusionCache;
	if (latestData.scnEmtCount == 0 && !g_dataHistory.empty()) {
		latestData = g_dataHistory.back();
	}

	int count = 0;
	for (int i = 0; i < latestData.scnEmtCount && i < maxSize; i++) {
		const EmtStruct& emt = latestData.etp2[i];

		if (strcmp(varName, "scnPlatId") == 0) {
			valueArray[i] = emt.scnPlatId;
		}
		else if (strcmp(varName, "scnEmtId") == 0) {
			valueArray[i] = emt.scnEmtId;
		}
		else if (strcmp(varName, "scnPlatForce") == 0) {
			valueArray[i] = emt.scnPlatForce;
		}
		else if (strcmp(varName, "scnPlatType") == 0) {
			valueArray[i] = emt.scnPlatType;
		}
		else if (strcmp(varName, "scnEmtType") == 0) {
			valueArray[i] = emt.scnEmtType;
		}
		else if (strcmp(varName, "scnTgtPos") == 0) {
			// 只返回经度
			valueArray[i] = emt.scnTgtPos[0];
		}
		else if (strcmp(varName, "scnTgtHight") == 0) {
			valueArray[i] = emt.scnTgtHight;
		}
		else if (strcmp(varName, "scnTgtAzim") == 0) {
			valueArray[i] = emt.scnTgtAzim;
		}
		else if (strcmp(varName, "scnTgtElev") == 0) {
			valueArray[i] = emt.scnTgtElev;
		}
		else if (strcmp(varName, "scnTgtSdist") == 0) {
			valueArray[i] = emt.scnTgtSdist;
		}
		else if (strcmp(varName, "scnTgtRadSpd") == 0) {
			valueArray[i] = emt.scnTgtRadSpd;
		}
		else if (strcmp(varName, "scnTgtTagSpd") == 0) {
			valueArray[i] = emt.scnTgtTagSpd;
		}
		else if (strcmp(varName, "scnSgnPowCode") == 0) {
			valueArray[i] = emt.scnSgnPowCode;
		}
		else if (strcmp(varName, "scnEmtWkMod") == 0) {
			valueArray[i] = emt.scnEmtWkMod;
		}
		else if (strcmp(varName, "scnEmtPlorMod") == 0) {
			valueArray[i] = emt.scnEmtPlorMod;
		}
		else if (strcmp(varName, "scnSgnFM") == 0) {
			valueArray[i] = emt.scnSgnFM;
		}
		else if (strcmp(varName, "scnAntScanProd") == 0) {
			valueArray[i] = emt.scnAntScanProd;
		}
		else if (strcmp(varName, "scnBeamDwlTm") == 0) {
			valueArray[i] = emt.scnBeamDwlTm;
		}
		else if (strcmp(varName, "scnIntfDur") == 0) {
			valueArray[i] = emt.scnIntfDur;
		}
		else if (strcmp(varName, "scnSgnRFTyp") == 0) {
			valueArray[i] = emt.scnSgnRFTyp;
		}
		else if (strcmp(varName, "scnSgnRFCnt") == 0) {
			valueArray[i] = emt.scnSgnRFCnt;
		}
		else if (strcmp(varName, "scnSgnRFMed") == 0) {
			valueArray[i] = emt.scnSgnRFMed;
		}
		else if (strcmp(varName, "scnSgnRFRng") == 0) {
			valueArray[i] = emt.scnSgnRFRng;
		}
		else if (strcmp(varName, "scnSgnRFList") == 0) {
			// 只返回第一个频点
			valueArray[i] = emt.scnSgnRFList[0];
		}
		else if (strcmp(varName, "scnSgnPRITyp") == 0) {
			valueArray[i] = emt.scnSgnPRITyp;
		}
		else if (strcmp(varName, "scnSgnPRICnt") == 0) {
			valueArray[i] = emt.scnSgnPRICnt;
		}
		else if (strcmp(varName, "scnSgnPRIMed") == 0) {
			valueArray[i] = emt.scnSgnPRIMed;
		}
		else if (strcmp(varName, "scnSgnPRIRng") == 0) {
			valueArray[i] = emt.scnSgnPRIRng;
		}
		else if (strcmp(varName, "scnSgnPRIList") == 0) {
			// 只返回第一个PRI值
			valueArray[i] = emt.scnSgnPRIList[0];
		}
		else if (strcmp(varName, "scnSgnPWTyp") == 0) {
			valueArray[i] = emt.scnSgnPWTyp;
		}
		else if (strcmp(varName, "scnSgnPWCnt") == 0) {
			valueArray[i] = emt.scnSgnPWCnt;
		}
		else if (strcmp(varName, "scnSgnPWMed") == 0) {
			valueArray[i] = emt.scnSgnPWMed;
		}
		else if (strcmp(varName, "scnPWRange") == 0) {
			valueArray[i] = emt.scnSgnPWRange;
		}
		else if (strcmp(varName, "scnSgnPWList") == 0) {
			// 只返回第一个PW值
			valueArray[i] = emt.scnSgnPWList[0];
		}
		else if (strcmp(varName, "scnEmtWkModID") == 0) {
			valueArray[i] = emt.scnEmtWkModID;
		}
		else {
			strcpy_s(g_lastErrorMsg, "Unknown variable name");
			return 0;
		}
		count++;
	}

	return count;
}

bool DataFusion(unsigned long long time)
{
    clearErrorMsg();
	//>>
	std::lock_guard<std::mutex> lock(g_mutex);

	if (g_dataHistory.empty()) {
		strcpy_s(g_lastErrorMsg, "No data available for fusion");
		return false;
	}

	// 1. 找到时间误差在±500ms范围内最接近的数据
	RFScnUpdataStruct* closestData = nullptr;
	long long minDiff = LLONG_MAX;

	for (auto& data : g_dataHistory) {
		// 计算数据时间戳（毫秒）
		tm tmTime = { 0 };
		tmTime.tm_year = data.scnTime[0] - 1900;
		tmTime.tm_mon = data.scnTime[1] - 1;
		tmTime.tm_mday = data.scnTime[2];
		tmTime.tm_hour = data.scnTime[3];
		tmTime.tm_min = data.scnTime[4];
		tmTime.tm_sec = data.scnTime[5];
		time_t tt = mktime(&tmTime);
		auto tp = std::chrono::system_clock::from_time_t(tt);
		auto tp_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(tp);
		long long dataTime = tp_ms.time_since_epoch().count() + data.scnTime[6];

		long long diff = llabs(static_cast<long long>(time) - dataTime);
		if (diff <= 500 && diff < minDiff) {
			minDiff = diff;
			closestData = &data;
		}
	}

	if (!closestData) {
		strcpy_s(g_lastErrorMsg, "No matching data found within ±500ms range");
		return false;
	}

	// 2. 融合该时间点之前的所有数据
	RFScnUpdataStruct newFusionCache;
	memset(&newFusionCache, 0, sizeof(RFScnUpdataStruct));
	std::map<UINT32, EmtStruct> emtMap;  // 用于去重，key为scnEmtId

	// 计算closestData的时间戳
	tm tmTime = { 0 };
	tmTime.tm_year = closestData->scnTime[0] - 1900;
	tmTime.tm_mon = closestData->scnTime[1] - 1;
	tmTime.tm_mday = closestData->scnTime[2];
	tmTime.tm_hour = closestData->scnTime[3];
	tmTime.tm_min = closestData->scnTime[4];
	tmTime.tm_sec = closestData->scnTime[5];
	time_t tt = mktime(&tmTime);
	auto tp = std::chrono::system_clock::from_time_t(tt);
	auto tp_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(tp);
	long long closestTime = tp_ms.time_since_epoch().count() + closestData->scnTime[6];

	for (auto& data : g_dataHistory) {
		// 计算当前数据时间戳
		tm tmCurrent = { 0 };
		tmCurrent.tm_year = data.scnTime[0] - 1900;
		tmCurrent.tm_mon = data.scnTime[1] - 1;
		tmCurrent.tm_mday = data.scnTime[2];
		tmCurrent.tm_hour = data.scnTime[3];
		tmCurrent.tm_min = data.scnTime[4];
		tmCurrent.tm_sec = data.scnTime[5];
		time_t ttCurrent = mktime(&tmCurrent);
		auto tpCurrent = std::chrono::system_clock::from_time_t(ttCurrent);
		auto tpCurrent_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(tpCurrent);
		long long currentTime = tpCurrent_ms.time_since_epoch().count() + data.scnTime[6];

		if (currentTime <= closestTime) {
			// 合并辐射源数据
			for (int i = 0; i < data.scnEmtCount; i++) {
				EmtStruct& emt = data.etp2[i];
				emtMap[emt.scnEmtId] = emt;  // 自动去重，保留最新的
			}
		}
	}

	// 填充融合后的数据
	newFusionCache.scnEquID = closestData->scnEquID;
	newFusionCache.scnEquCourse = closestData->scnEquCourse;
	memcpy(newFusionCache.scnTime, closestData->scnTime, sizeof(closestData->scnTime));
	newFusionCache.scnEmtCount = 0;

	// 将map中的数据复制到融合缓存中
	for (auto& pair : emtMap) {
		if (newFusionCache.scnEmtCount < 32) {
			newFusionCache.etp2[newFusionCache.scnEmtCount++] = pair.second;
		}
	}

	g_fusionCache = newFusionCache;
	//<<
	return true;
}
