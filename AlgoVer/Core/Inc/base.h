#ifndef BASE_H
#define BASE_H

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "task.h"

#define MAX_QUEUE 100          // 队列最大长度
#define MAX_RECV_BUFFER 1024  // 接收缓冲区大小


typedef struct {
    float timeStep;
    float wx, wy, wz;
    float q0, q1, q2, q3;
    float rx, ry, rz;
    float vx, vy, vz;
    float wboX, wboY, wboZ;
    float qbo0, qbo1, qbo2, qbo3;
    float tx, ty, tz;
    float zAngle;
    float torqueX, torqueY, torqueZ;
} telemetryStruct;

// 从字节数组解析数据到结构体
static inline bool telemetryFromBytes(telemetryStruct* telemetry, const uint8_t* data, size_t length) {
    if (data == NULL || length < sizeof(telemetryStruct)) {
        return false;
    }
    memcpy(telemetry, data, sizeof(telemetryStruct));
    return true;
}

// 将结构体序列化为字节数组
static inline void telemetryToBytes(const telemetryStruct* telemetry, uint8_t* buffer) {
    memcpy(buffer, telemetry, sizeof(telemetryStruct));
}

typedef enum {
    telemetryType = 1,
    faultResultType = 2,
    faultParaType = 3
} CommuDataType;

typedef struct {
    char* header;
    char* tail;
} PackageManager;

// 全局单例
static PackageManager g_package_manager = {NULL, NULL};

static inline char* my_strdup(const char* s) {
    size_t len = strlen(s) + 1;
    char* dup = malloc(len);
    if (dup) {
        memcpy(dup, s, len);
    }
    return dup;
}

// 初始化包管理器
static inline void package_manager_init(const char* header, const char* tail) {
    g_package_manager.header = my_strdup(header);
    g_package_manager.tail = my_strdup(tail);
}

// 清理包管理器
static inline void package_manager_cleanup(void) {
    free(g_package_manager.header);
    free(g_package_manager.tail);
    g_package_manager.header = NULL;
    g_package_manager.tail = NULL;
}

// 验证包的有效性
static inline bool validate_package(const char* package, CommuDataType* dataType) {
    size_t header_len = strlen(g_package_manager.header);
    size_t tail_len = strlen(g_package_manager.tail);
    size_t package_len = strlen(package);
    
    // 检查包长度是否合理
    size_t min_length = header_len + 1 + tail_len;
	
    if (package_len < min_length) {
        return false;
    }
    
    // 验证帧头
    if (strncmp(package, g_package_manager.header, header_len) != 0) {
        return false;
    }
    
    // 验证帧尾
    if (strncmp(package + package_len - tail_len, g_package_manager.tail, tail_len) != 0) {
        return false;
    }
    
    // 提取类型标识符
    *dataType = (CommuDataType)package[header_len];
    
    return true;
}

// 打包数据
static inline char* package_data(const char* data, CommuDataType identifier) {
    if (!g_package_manager.header || !g_package_manager.tail) {
        return NULL;
    }
    
    size_t header_len = strlen(g_package_manager.header);
    size_t data_len = strlen(data);
    size_t tail_len = strlen(g_package_manager.tail);
    
    // 分配内存：帧头 + 标识符(1字节) + 数据 + 帧尾 + 结束符
    char* package = (char*)malloc(header_len + 1 + data_len + tail_len + 1);
    if (!package) {
        return NULL;
    }
    
    // 组装包
    strcpy(package, g_package_manager.header);
    package[header_len] = (char)identifier;
    strcpy(package + header_len + 1, data);
    strcpy(package + header_len + 1 + data_len, g_package_manager.tail);
    
    return package;
}

// 解包数据
static inline char* unpackage_data(const char* package, CommuDataType* dataType) {
    if (!validate_package(package, dataType)) {
        return NULL;
    }
    
    size_t header_len = strlen(g_package_manager.header);
    size_t package_len = strlen(package);
    size_t tail_len = strlen(g_package_manager.tail);
    
    // 计算数据部分的长度
    size_t data_len = package_len - header_len - 1 - tail_len;
    
    // 分配内存并复制数据部分
    char* data = (char*)pvPortMalloc(data_len + 1);
    if (!data) {
        return NULL;
    }
    
    strncpy(data, package + header_len + 1, data_len);
    data[data_len] = '\0';
    
    return data;
}

#endif // BASE_H

