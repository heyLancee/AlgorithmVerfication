#ifndef BASE_H
#define BASE_H

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define QUEUE_MAX_SIZE 256          // 队列最大长度
#define LEN_QUEUE_MAX_SIZE 2       // 队列最大长度
#define MAX_RECV_BUFFER 256        // 接收缓冲区大小
#define MAX_SEND_BUFFER 256         // 发送缓冲区大小

// 定义枚举类型
typedef enum {
    TELEMETRY = 1,
    FAULT_RESULT = 2,
    FAULT_PARA = 3
} CommuDataType;

typedef enum {
    FLYWHEEL = 1,
    GYROSCOPES = 2
} ComponentFaultType;

// 故障类型枚举
typedef enum {
    NO_FAULT = 0,
    GYRO_INTERMITTENT_FAULT = 1,
    GYRO_SLOW_FAULT = 2,
    GYRO_MULTI_FAULT = 3,
    FLYWHEEL_PARTIAL_LOSS = 4,
    FLYWHEEL_BIAS = 5,
    FLYWHEEL_COMPREHENSIVE = 6
} FaultType;

// 遥测数据结构
typedef struct {
    double timeStep;
    double wx;  // deg/s
    double wy;
    double wz;
    double q0;
    double q1;
    double q2;
    double q3;
    double rx;
    double ry;
    double rz;
    double vx;
    double vy;
    double vz;
    double wboX;
    double wboY;
    double wboZ;
    double qbo0;
    double qbo1;
    double qbo2;
    double qbo3;
    double tx;   // Nm
    double ty;
    double tz;
    double zAngle;  // deg
} TelemetryStruct;

// 故障参数结构
typedef struct {
    FaultType fault_type;
    double params[2]; // 最多2个参数
    uint8_t param_count;
    
    double fault_start_time;
    double fault_end_time;
    
    // 参数别名
    double f1;          // 间歇故障系数
    double lambda_s;     // 慢变系数
    double k_s;          // 慢变指数
    double lambda_m;     // 乘性系数
    double e;            // 部分失效系数
    double b;            // 偏置系数
    
    int gyro_fault_idx;   // 陀螺故障索引
    int flywheel_fault_idx; // 飞轮故障索引
} FaultParams;

// 数据包管理器
typedef struct {
    uint8_t* header;
    uint8_t header_len;
    uint8_t* tail;
    uint8_t tail_len;
} PackageManager;

// 函数声明
size_t calculate_package_length(const PackageManager *pm, CommuDataType commu_type, FaultType fault_type);
void TelemetryStruct_to_byte_array(const TelemetryStruct* obj, uint8_t* output);
void TelemetryStruct_from_byte_array(TelemetryStruct* obj, const uint8_t* data);
void FaultParams_to_byte_array(const FaultParams* obj, uint8_t* output);
void FaultParams_from_byte_array(FaultParams* obj, const uint8_t* data);
void PackageManager_init(PackageManager* pm, const char* header, const char* tail);
void PackageManager_package(const PackageManager* pm, const void* data, CommuDataType commu_type, uint8_t* output);
int PackageManager_unpackage(const PackageManager* pm, const uint8_t* package, uint16_t package_len, void** output, CommuDataType* output_type);

extern PackageManager *package_manager; // 声明包管理器

#endif // BASE_H

