#define _CRT_SECURE_NO_WARNINGS
#include "DeviceDemo_1.h"
#include "MsgDef.h"
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
#include <ctime>
//<<-

#define DEVICE_NAME "设备1"

static DataCallback g_dataCallback = nullptr;
static std::map<std::string, double*> g_dataMap;

// 错误信息
static char g_lastErrorMsg[256] = { 0 };

//>>def
static std::mutex g_mutex;  // 用于线程同步
static TY_DEVICE_INFO g_deviceInfo;
static DEVICE_STATUS_CODE g_deviceStatus = DEVICE_STATUS_OFFLINE;
static DataCallback g_callback = nullptr;
static volatile bool g_captureRunning = false;
static std::thread g_captureThread;

// 数据缓存
// 结构体RFScnUpdataStruct大小过大，需要new在堆上分配空间
std::vector<std::shared_ptr<RFScnUpdataStruct>> g_dataHistory;
std::map<ULL, std::shared_ptr<RFScnUpdataStruct>> g_dataFusion;


static inline void clearErrorMsg()
{
    if (g_lastErrorMsg[0] != '\0') {
        memset(g_lastErrorMsg, 0, sizeof(g_lastErrorMsg));      // 清空错误信息
    }
}


/**
 * @brief 初始化设备库，配置默认设备信息
 * 
 * - 初始化设备信息结构体 `g_deviceInfo`
 * - 设置默认设备名称、协议、IP地址和端口
 * 
 * @return true 初始化成功
 * @return false 初始化失败（当前实现始终返回true）
 */
DLL_API bool initLibrary()
{
    clearErrorMsg();
    // 初始化测试数据
#if 0
    double pA[10] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0 };
    double pB[10] = { 10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0 };
    double pC[10] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0 };
    g_dataMap.insert(std::pair<std::string, double*>("测试设备1/参数A", pA));
    g_dataMap.insert(std::pair<std::string, double*>("测试设备1/参数B", pB));
    g_dataMap.insert(std::pair<std::string, double*>("测试设备1/参数C", pC));
    return true;
#endif

    memset(&g_deviceInfo, 0, sizeof(g_deviceInfo));
    strcpy_s(g_deviceInfo.deviceName, DEVICE_NAME);
    strcpy_s(g_deviceInfo.protocol, "DDS");
	g_deviceInfo.searchRangeMs = 500;
	g_deviceInfo.storageDeepthMs = 5000;

    g_deviceStatus = DEVICE_STATUS_OFFLINE;

	

    return true;

}

/**
 * @brief 销毁库资源
 * 
 * - 停止数据采集线程（如果正在运行）
 * - 断开设备连接（如果已连接）
 * 
 * @return true 资源释放成功
 */
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
    if (g_deviceStatus != DEVICE_STATUS_OFFLINE) {
        disconnectDevice();
    }
    return true;

}


DLL_API void getLastErrorMsg(char* errMsg)
{
    strcpy_s(errMsg, 256, g_lastErrorMsg);

}


DLL_API DEVICE_STATUS_CODE getDeviceStatus()
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

DLL_API bool setDeviceInfo(TY_DEVICE_INFO info)
{
    clearErrorMsg();
    g_deviceInfo = info;

    return true;
}

/**
 * @brief 连接到配置的设备（模拟实现）
 * 
 * - 检查设备是否已连接
 * - 模拟连接成功
 * 
 * @return true 连接成功或已连接
 * @return false 连接失败（当前实现始终返回true）
 * @note 实际网络代码需启用 `#if 0` 部分
 */
DLL_API bool connectDevice()
{
    clearErrorMsg();

    //>>
    std::lock_guard<std::mutex> lock(g_mutex);

    if (g_deviceStatus != DEVICE_STATUS_OFFLINE) {
        strcpy_s(g_lastErrorMsg, "Already connected");
        return false;
    }

    g_deviceStatus = DEVICE_STATUS_ONLINE;
    //<<
    return true;
}

/**
 * @brief 断开设备连接
 * 
 * - 关闭dds连接
 * - 更新设备连接状态
 * 
 * @return true 断开成功或未连接
 */
DLL_API bool disconnectDevice()
{
    clearErrorMsg();

    //>>
    std::lock_guard<std::mutex> lock(g_mutex);

    if (g_deviceStatus == DEVICE_STATUS_OFFLINE) {
        strcpy_s(g_lastErrorMsg, "Not connected");
        return false;
    }

    g_deviceStatus = DEVICE_STATUS_OFFLINE;
    //<<

    return true;
}


DLL_API void registerCallback(DataCallback callbackFunc)
{
    clearErrorMsg();

    std::lock_guard<std::mutex> lock(g_mutex);
    g_callback = callbackFunc;
}

/**
 * @brief 数据采集线程函数（模拟数据生成）
 * 
 * - 循环生成模拟雷达数据（`RFScnUpdataStruct`）
 * - 每100ms触发一次回调（`g_callback`）
 * - 数据存入历史队列 `g_dataHistory`
 */
static void captureThreadFunc() 
{
    while (g_captureRunning) 
	{
        // 模拟数据采集

        // 结构体RFScnUpdataStruct大小过大，需要new在堆上分配空间
        std::shared_ptr<RFScnUpdataStruct> newData = std::make_shared<RFScnUpdataStruct>();       
        auto now = std::chrono::system_clock::now();
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
        auto epoch = now_ms.time_since_epoch();
        auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
        long long timestamp = value.count();

        // 填充模拟数据
        newData->scnEquID = 1;
        newData->scnEquCourse = 45.0f;
        auto tt = std::chrono::system_clock::to_time_t(now);
        tm time;
        localtime_s(&time, &tt);
        newData->scnTime[0] = time.tm_year + 1900;
        newData->scnTime[1] = time.tm_mon + 1;
        newData->scnTime[2] = time.tm_mday;
        newData->scnTime[3] = time.tm_hour;
        newData->scnTime[4] = time.tm_min;
        newData->scnTime[5] = time.tm_sec;
        newData->scnTime[6] = (timestamp % 1000);

		printf("time :%d:%d:%d:%d:%d:%d:%d\n", 
			newData->scnTime[0],
			newData->scnTime[1],
			newData->scnTime[2],
			newData->scnTime[3],
			newData->scnTime[4],
			newData->scnTime[5],
			newData->scnTime[6]);
        newData->scnEmtCount = 1;
        EmtStruct& emt = newData->etp2[0];
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
        if (g_callback) 
        {
            g_callback();
        }

        // 模拟采集间隔
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

/**
 * @brief 启动数据采集线程
 * 
 * - 检查采集状态和设备连接状态
 * - 创建线程执行 `captureThreadFunc` 模拟数据采集
 * 
 * @return true 线程启动成功
 * @return false 启动失败（如已运行或未连接）
 * @note 采集周期固定为100ms
 */
DLL_API bool startCapture()
{
    clearErrorMsg();

    //>>
    std::lock_guard<std::mutex> lock(g_mutex);

    if (g_deviceStatus == DEVICE_STATUS_OFFLINE) {
        g_captureRunning = false;
        strcpy_s(g_lastErrorMsg, "Device is not connected");
        return false;
    }

    if (g_captureRunning) {
        g_deviceStatus = DEVICE_STATUS_CAPTURE;
        strcpy_s(g_lastErrorMsg, "Capture is already running");
        return false;
    }

    g_captureRunning = true;
    g_captureThread = std::thread(captureThreadFunc);
    //<<
    g_deviceStatus = DEVICE_STATUS_CAPTURE;
    return true;
}

/**
 * @brief 停止数据采集线程
 * 
 * - 设置停止标志并等待线程结束
 * 
 * @return true 停止成功
 * @return false 停止失败（如未运行）
 */
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
    g_deviceStatus = DEVICE_STATUS_ONLINE;
    return true;
}

/**
 * @brief 获取支持的采集变量名称列表
 * 
 * @param outputVarNames 输出缓冲区指针数组（需预先分配）
 * @param maxSize 缓冲区最大容量
 * @return int 实际返回的变量数量
 * @note 调用者需负责释放 `_strdup` 分配的内存
 */
DLL_API int getCaptureOutputVarNames(char** outputVarNames, int maxSize)
{
    clearErrorMsg();

    const char* varNames[] = {
        "平台Id ", "辐射源Id", "平台敌我属性", "平台类型", "辐射源类型",
        "目标位置", "目标高度", "目标方位", "目标俯仰", "目标斜距",
        "目标径向速度", "目标切向速度", "信号功率码", "辐射源工作模式", "辐射源极化方式",
        "信号调制方式", "天线扫描周期", "波位驻留时间", "相干处理间隔", "信号频率类型",
        "信号频点数", "信号频率中值", "信号频率范围", "频点列表", "信号PRI类型",
        "信号PRI点数", "信号PRI中值 ", "信号PRI范围", "PRI列表", "信号PW类型 ",
        "信号PW点数", " 信号PW中值", "信号PW范围", "信号PW列表", "辐射源工作模式ID"
    };

    int count = (maxSize < static_cast<int>(sizeof(varNames) / sizeof(varNames[0])) ? maxSize : static_cast<int>(sizeof(varNames) / sizeof(varNames[0])));
    if (count > maxSize)
    {
        strcpy(g_lastErrorMsg, "变量数量超过最大值");
        return -1;
    }
    for (int i = 0; i < count; i++) {
        outputVarNames[i] = _strdup(varNames[i]);
    }
    return count;
}

/**
 * @brief 根据变量名获取采集数据值
 * 
 * @param varName 变量名（需与 `getCaptureOutputVarNames` 返回一致）
 * @param valueArray 输出数据数组（需预先分配）
 * @param arraySize 输出数据个数
 * @param maxSize 数组最大容量
 * @return 0 无数据或错误（检查 `g_lastErrorMsg`）
 */
DLL_API DEVICE_GET_ERROR_CODE getCaptureValue(
    const char* varName,       // 变量名 (IN
    unsigned long long msec,   // 时间 (IN)
    double* valueArray,        // 值数组 (OUT)
    int* arraySize,            // 传出数据长度 (OUT)
    int maxSize                // 值数组最大大小 (IN)
)
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
        return DEVICE_GET_ERROR_OTHER;
    }

    // 获取最新的融合数据
    std::shared_ptr<RFScnUpdataStruct> latestData = g_fusionCache;
    if (latestData->scnEmtCount == 0 && !g_dataHistory.empty()) {
        latestData = g_dataHistory.back();
    }

    int count = 0;
    for (int i = 0; i < latestData->scnEmtCount && i < maxSize; i++) {
        const EmtStruct& emt = latestData->etp2[i];

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
            strcpy(g_lastErrorMsg, "变量名不存在");
            return DEVICE_GET_ERROR_OTHER;
        }
        count++;
    }

    *arraySize = count;
    if (count > maxSize)
    {
        strcpy(g_lastErrorMsg, "变量值数组大小不足");
        return DEVICE_GET_ERROR_OTHER;
    }
    return DEVICE_GET_SUCCESS;
}


static unsigned long long convertToTimestamp(const unsigned int timeArray[7]) {
    // 提取时间字段（假设 timeArray 是 UTC 时间）
    unsigned int year = timeArray[0];
    unsigned int month = timeArray[1];
    unsigned int day = timeArray[2];
    unsigned int hour = timeArray[3];
    unsigned int minute = timeArray[4];
    unsigned int second = timeArray[5];
    unsigned int millisecond = timeArray[6];

    // 转换为 tm 结构（UTC 基准）
    struct tm timeStruct = {};
    timeStruct.tm_year = year - 1900;
    timeStruct.tm_mon = month - 1;
    timeStruct.tm_mday = day;
    timeStruct.tm_hour = hour;
    timeStruct.tm_min = minute;
    timeStruct.tm_sec = second;
    timeStruct.tm_isdst = 0; // 禁用夏令时

    // 计算 UTC 时间戳（秒级）
    time_t utcSeconds = timegm(&timeStruct); // 使用 timegm 而非 mktime（需平台兼容）
    // 若平台不支持 timegm，手动调整时区偏移（如东八区：-8*3600）
    // time_t utcSeconds = mktime(&timeStruct) - 8*3600; 

    // 合并毫秒
    unsigned long long timestamp = 
        static_cast<unsigned long long>(utcSeconds) * 1000ULL + 
        millisecond;

    return timestamp;
}

/**
 * @brief 基于时间戳进行数据融合
 * 
 * - 合并该时间点前的所有数据，按 `scnEmtId` 去重
 * @return true 融合成功
 * @return false 融合失败（无数据或超时）
 * @note 融合结果存储在 `g_fusionCache` 中
 */
DLL_API bool dataFusion()
{
    clearErrorMsg();
    //>>
    std::lock_guard<std::mutex> lock(g_mutex);

    if (g_dataHistory.empty()) {
        strcpy_s(g_lastErrorMsg, "No data available for fusion");
        return false;
    }

    // 1. 取当前最新的采集数据
	 RFScnUpdataStruct* closestData = g_dataHistory.back();
    if (!closestData) {
        strcpy_s(g_lastErrorMsg, "No matching data found within ±500ms range");
        return false;
    }

    // 2. 融合该时间点之前的所有数据
    std::shared_ptr<RFScnUpdataStruct> newFusionCache = std::make_shared<RFScnUpdataStruct>();
    std::map<UINT32, EmtStruct> emtMap;  // 用于去重，key为scnEmtId

    for (auto& data : g_dataHistory)  //融合历史数据
	{
        // 合并辐射源数据
        for (int i = 0; i < data->scnEmtCount; i++) 
		{
            EmtStruct& emt = data->etp2[i];
            emtMap[emt.scnEmtId] = emt;  // 自动去重，保留最新的
        }   
    }

    // 填充融合后的数据
    newFusionCache->scnEquID = closestData->scnEquID;
    newFusionCache->scnEquCourse = closestData->scnEquCourse;
    memcpy(newFusionCache->scnTime, closestData->scnTime, sizeof(closestData->scnTime));
    newFusionCache->scnEmtCount = 0;

    // 将map中的数据复制到融合缓存中
    for (auto& pair : emtMap) 
	{
        if (newFusionCache->scnEmtCount < 32) 
		{
            newFusionCache->etp2[newFusionCache->scnEmtCount++] = pair.second;
        }
    }
	ULL currentTime = convertToTimestamp(newFusionCache->scnTime);
	g_dataFusion[currentTime,newFusionCache];

	
	
    //<<
    return true;
}
