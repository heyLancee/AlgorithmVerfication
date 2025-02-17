#ifndef BASE_H
#define BASE_H

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "task.h"

#define QUEUE_MAX_SIZE 100          // 队列最大长度
#define LEN_QUEUE_MAX_SIZE 10       // 队列最大长度
#define MAX_RECV_BUFFER 256        // 接收缓冲区大小
#define PACKAGE_BUFFER_SIZE 256
#define PACKAGE_BUFFER_COUNT 2  // 缓冲区池大小


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

// 缓冲区池结构
typedef struct {
    char buffers[PACKAGE_BUFFER_COUNT][PACKAGE_BUFFER_SIZE];
    bool used[PACKAGE_BUFFER_COUNT];
} BufferPool;

// 全局缓冲区池
static BufferPool g_buffer_pool = {{{0}}, {false}};

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

// 从缓冲区池获取可用缓冲区
static inline char* get_buffer(void) {
    for(int i = 0; i < PACKAGE_BUFFER_COUNT; i++) {
        if(!g_buffer_pool.used[i]) {
            g_buffer_pool.used[i] = true;
            return g_buffer_pool.buffers[i];
        }
    }
    return NULL;  // 所有缓冲区都在使用中
}

// 释放缓冲区回池
static inline void release_buffer(char** buffer) {
    if (buffer == NULL || *buffer == NULL) {
        return;
    }
    for(int i = 0; i < PACKAGE_BUFFER_COUNT; i++) {
        if(g_buffer_pool.buffers[i] == *buffer) {
            g_buffer_pool.used[i] = false;
            *buffer = NULL;
            return;
        }
    }
}

// 修改后的打包函数
static inline char* package_data(const char* data, CommuDataType identifier) {
    if (!g_package_manager.header || !g_package_manager.tail) {
        return NULL;
    }
    
    size_t header_len = strlen(g_package_manager.header);
    size_t data_len = strlen(data);
    size_t tail_len = strlen(g_package_manager.tail);
    size_t total_len = header_len + 1 + data_len + tail_len + 1;
    
    // 检查总长度是否超出缓冲区
    if(total_len > PACKAGE_BUFFER_SIZE) {
        return NULL;
    }
    
    // 从池中获取缓冲区
    char* package = get_buffer();
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

// 修改后的解包函数
static inline char* unpackage_data(const char* package, CommuDataType* dataType) {
    if (!validate_package(package, dataType)) {
        return NULL;
    }
    
    size_t header_len = strlen(g_package_manager.header);
    size_t package_len = strlen(package);
    size_t tail_len = strlen(g_package_manager.tail);
    size_t data_len = package_len - header_len - 1 - tail_len;
    
    // 检查数据长度是否合法
    if(data_len + 1 > PACKAGE_BUFFER_SIZE) {
        return NULL;
    }
    
    // 从池中获取缓冲区
    char* data = get_buffer();
    if (!data) {
        return NULL;
    }
    
    // 复制数据部分
    strncpy(data, package + header_len + 1, data_len);
    data[data_len] = '\0';
    
    return data;
}

#endif // BASE_H

