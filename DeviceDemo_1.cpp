#define _CRT_SECURE_NO_WARNINGS
#include "DeviceDemo_1.h"
#include <Map>
#include <string>

#define DEVICE_NAME "设备1"

static DataCallback g_dataCallback = nullptr;
static std::map<std::string, double*> g_dataMap;
static char g_errorMsg[256];
static TY_DEVICE_INFO g_deviceInfi;

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
    double pA[10] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0 };
    double pB[10] = { 10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0 };
    double pC[10] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0 };
    g_dataMap.insert(std::pair<std::string, double*>("测试设备1/设备参数A", pA));
    g_dataMap.insert(std::pair<std::string, double*>("测试设备1/设备参数B", pB));
    g_dataMap.insert(std::pair<std::string, double*>("测试设备1/设备参数C", pC));
    return true;
}

DLL_API bool destroyLibrary()
{
    clearErrorMsg();
    for (auto& array : g_dataMap)
    {
        delete[] array.second;
    }
    return true;
}

DLL_API void getLastErrorMsg(char* errMsg)
{
    strcpy(errMsg, g_errorMsg);
}

DLL_API DeviceStatus getDeviceStatus()
{
    clearErrorMsg();
    return DeviceStatus();
}

DLL_API DeviceInfo getDeviceInfo()
{
    clearErrorMsg();
    return DeviceInfo();
}

DLL_API bool setDeviceInfo(DeviceInfo info)
{
    clearErrorMsg();


    return true;
}

DLL_API bool connectDevice()
{
    clearErrorMsg();
    return true;
}

DLL_API bool disConnectDevice()
{
    clearErrorMsg();
    return true;
}

DLL_API void RegisterCallback(DataCallback callbackFunc)
{
    clearErrorMsg();
    g_dataCallback = callbackFunc;
}

DLL_API bool startCapture()
{
    clearErrorMsg();

    /**功能**/
    //1.从dds中采集后缓存原始数据
    //2.触发回调函数
    return true;
}

DLL_API bool stopCapture()
{
    clearErrorMsg();
    return true;
}

DLL_API int getCaptureOutputVarNames(char** outputVarNames, int maxSize)
{
    clearErrorMsg();
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
}

DLL_API int getCaptureValue(const char* varName, double* valueArray, int maxSize)
{
    clearErrorMsg();
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
}

bool DataFusion(unsigned long long time)
{
    clearErrorMsg();
    return true;
}
