#ifndef BASE_H
#define BASE_H

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>

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

// 遥测结构序列化
void TelemetryStruct_to_byte_array(const TelemetryStruct* obj, uint8_t* output) {
    double* ptr = (double*)output;
    ptr[0] = obj->timeStep;
    ptr[1] = obj->wx;
    ptr[2] = obj->wy;
    ptr[3] = obj->wz;
    ptr[4] = obj->q0;
    ptr[5] = obj->q1;
    ptr[6] = obj->q2;
    ptr[7] = obj->q3;
    ptr[8] = obj->rx;
    ptr[9] = obj->ry;
    ptr[10] = obj->rz;
    ptr[11] = obj->vx;
    ptr[12] = obj->vy;
    ptr[13] = obj->vz;
    ptr[14] = obj->wboX;
    ptr[15] = obj->wboY;
    ptr[16] = obj->wboZ;
    ptr[17] = obj->qbo0;
    ptr[18] = obj->qbo1;
    ptr[19] = obj->qbo2;
    ptr[20] = obj->qbo3;
    ptr[21] = obj->tx;
    ptr[22] = obj->ty;
    ptr[23] = obj->tz;
    ptr[24] = obj->zAngle;
}

// 遥测结构反序列化
void TelemetryStruct_from_byte_array(TelemetryStruct* obj, const uint8_t* data) {
    const double* ptr = (const double*)data;
    obj->timeStep = ptr[0];
    obj->wx = ptr[1];
    obj->wy = ptr[2];
    obj->wz = ptr[3];
    obj->q0 = ptr[4];
    obj->q1 = ptr[5];
    obj->q2 = ptr[6];
    obj->q3 = ptr[7];
    obj->rx = ptr[8];
    obj->ry = ptr[9];
    obj->rz = ptr[10];
    obj->vx = ptr[11];
    obj->vy = ptr[12];
    obj->vz = ptr[13];
    obj->wboX = ptr[14];
    obj->wboY = ptr[15];
    obj->wboZ = ptr[16];
    obj->qbo0 = ptr[17];
    obj->qbo1 = ptr[18];
    obj->qbo2 = ptr[19];
    obj->qbo3 = ptr[20];
    obj->tx = ptr[21];
    obj->ty = ptr[22];
    obj->tz = ptr[23];
    obj->zAngle = ptr[24];
}

// 获取故障类型对应的参数数量
static uint8_t get_fault_param_count(FaultType fault_type) {
    switch(fault_type) {
        case NO_FAULT: return 0;
        case GYRO_INTERMITTENT_FAULT: return 1;
        case GYRO_SLOW_FAULT: return 2;
        case GYRO_MULTI_FAULT: return 1;
        case FLYWHEEL_PARTIAL_LOSS: return 1;
        case FLYWHEEL_BIAS: return 1;
        case FLYWHEEL_COMPREHENSIVE: return 2;
        default: return 0;
    }
}

// 获取遥测数据结构体编码后长度
size_t get_telemetry_encoded_length(void) {
    return sizeof(TelemetryStruct); // n个double = n*8字节
}

// 获取故障参数结构体编码后长度
size_t get_faultparams_encoded_length(const FaultType fault_type) {
    // 固定部分：2个double(故障开始和结束时间) + 2个int(类型和组件ID) = 16+8 = 24字节
    // 可变部分：参数数组
    return 24 + get_fault_param_count(fault_type) * sizeof(double);
}

// 获取包管理器编码后长度
size_t calculate_package_length(const PackageManager* pm, CommuDataType commu_type, FaultType fault_type) {
    if (pm == NULL) return 0;

    size_t data_len = 0;
    
    // 计算数据部分长度
    switch (commu_type) {
        case TELEMETRY:
            data_len = get_telemetry_encoded_length();
            break;
        case FAULT_PARA: {
            data_len = get_faultparams_encoded_length(fault_type);
            break;
        }
        default:
            return 0; // 无效数据类型
    }

    // 总长度 = 包头 + 类型(4字节) + 数据 + 包尾
    return pm->header_len + sizeof(uint32_t) + data_len + pm->tail_len;
}

// 故障参数结构序列化
void FaultParams_to_byte_array(const FaultParams* obj, uint8_t* output) {
    // 打包时间和类型
    double* dptr = (double*)output;
    dptr[0] = obj->fault_start_time;
    dptr[1] = obj->fault_end_time;
    
    uint32_t* iptr = (uint32_t*)(output + 16); // 跳过2个double
    iptr[0] = (uint32_t)obj->fault_type;
    
    // 确定组件ID
    int component_id = 0;
    if (obj->fault_type == GYRO_INTERMITTENT_FAULT || 
        obj->fault_type == GYRO_SLOW_FAULT || 
        obj->fault_type == GYRO_MULTI_FAULT) {
        component_id = obj->gyro_fault_idx;
    } else if (obj->fault_type == FLYWHEEL_PARTIAL_LOSS || 
               obj->fault_type == FLYWHEEL_BIAS || 
               obj->fault_type == FLYWHEEL_COMPREHENSIVE) {
        component_id = obj->flywheel_fault_idx;
    }
    iptr[1] = (uint32_t)component_id;
    
    // 打包参数
    uint8_t param_count = get_fault_param_count(obj->fault_type);
    if (param_count > 0) {
        memcpy(output + 24, obj->params, param_count * sizeof(double));
    }
}

// 故障参数结构反序列化
void FaultParams_from_byte_array(FaultParams* obj, const uint8_t* data) {
    // 解包时间和类型
    const double* dptr = (const double*)data;
    obj->fault_start_time = dptr[0];
    obj->fault_end_time = dptr[1];
    
    const uint32_t* iptr = (const uint32_t*)(data + 16); // 跳过2个double
    obj->fault_type = (FaultType)iptr[0];
    int component_id = (int)iptr[1];
    
    // 解包参数
    obj->param_count = get_fault_param_count(obj->fault_type);
    if (obj->param_count > 0) {
        memcpy(obj->params, data + 24, obj->param_count * sizeof(double));
    }
    
    // 设置组件ID
    if (obj->fault_type == GYRO_INTERMITTENT_FAULT || 
        obj->fault_type == GYRO_SLOW_FAULT || 
        obj->fault_type == GYRO_MULTI_FAULT) {
        obj->gyro_fault_idx = component_id;
    } else if (obj->fault_type == FLYWHEEL_PARTIAL_LOSS || 
               obj->fault_type == FLYWHEEL_BIAS || 
               obj->fault_type == FLYWHEEL_COMPREHENSIVE) {
        obj->flywheel_fault_idx = component_id;
    }
    
    // 设置参数别名
    switch(obj->fault_type) {
        case GYRO_INTERMITTENT_FAULT:
            obj->f1 = obj->params[0];
            break;
        case GYRO_SLOW_FAULT:
            obj->lambda_s = obj->params[0];
            obj->k_s = obj->params[1];
            break;
        case GYRO_MULTI_FAULT:
            obj->lambda_m = obj->params[0];
            break;
        case FLYWHEEL_PARTIAL_LOSS:
            obj->e = obj->params[0];
            break;
        case FLYWHEEL_BIAS:
            obj->b = obj->params[0];
            break;
        case FLYWHEEL_COMPREHENSIVE:
            obj->e = obj->params[0];
            obj->b = obj->params[1];
            break;
        default:
            break;
    }
}

// 初始化包管理器
void PackageManager_init(PackageManager* pm, const char* header, const char* tail) {
    pm->header_len = strlen(header);
    pm->header = (uint8_t*)malloc(pm->header_len);
    memcpy(pm->header, header, pm->header_len);
    
    pm->tail_len = strlen(tail);
    pm->tail = (uint8_t*)malloc(pm->tail_len);
    memcpy(pm->tail, tail, pm->tail_len);
}

// 封装数据包
void PackageManager_package(const PackageManager* pm, const void* data, CommuDataType commu_type, uint8_t* output) {
    /*
        param:
            pm: 包管理器 [PackageManager]
            data: 要发送的数据 [TelemetryStruct, FaultParams]
            commu_type: 数据类型 [CommuDataType]
            output: 输出的栈内存数据包，需要在外部申请好栈空间 [uint8_t*]
    */
    uint16_t pos = 0;
    
    // 添加包头
    memcpy(output, pm->header, pm->header_len);
    pos += pm->header_len;
    
    // 添加类型
    *((uint32_t*)(output + pos)) = (uint32_t)commu_type;
    pos += 4;
    
    // 添加数据
    uint16_t data_len = 0;
    if (commu_type == TELEMETRY) {
        data_len = sizeof(TelemetryStruct);
        TelemetryStruct_to_byte_array((const TelemetryStruct*)data, output + pos);
    } else if (commu_type == FAULT_PARA) {
        data_len = 24 + get_fault_param_count(((const FaultParams*)data)->fault_type) * sizeof(double);
        FaultParams_to_byte_array((const FaultParams*)data, output + pos);
    }
    pos += data_len;
    
    // 添加包尾
    memcpy(output + pos, pm->tail, pm->tail_len);
    pos += pm->tail_len;
}

// 解包数据包
int PackageManager_unpackage(const PackageManager* pm, const uint8_t* package, uint16_t package_len, void** output, CommuDataType* output_type) {
    // 检查长度
    if (package_len < pm->header_len + 4 + pm->tail_len) {
        return 0; // 数据包太短
    }
    
    // 检查包头包尾
    if (memcmp(package, pm->header, pm->header_len) != 0 || 
        memcmp(package + package_len - pm->tail_len, pm->tail, pm->tail_len) != 0) {
        return 0; // 包头包尾不匹配
    }
    
    // 解包类型
    uint32_t type_value = *((const uint32_t*)(package + pm->header_len));
    *output_type = (CommuDataType)type_value;
    
    // 解包数据
    const uint8_t* data_start = package + pm->header_len + 4;
    uint16_t data_len = package_len - pm->header_len - 4 - pm->tail_len;
    
    if (*output_type == TELEMETRY) {
        if (data_len != sizeof(TelemetryStruct)) {
            return 0; // 数据长度不匹配
        }
        *output = malloc(sizeof(TelemetryStruct));
        TelemetryStruct_from_byte_array((TelemetryStruct*)*output, data_start);
        return 1;
    } else if (*output_type == FAULT_PARA) {
        *output = malloc(sizeof(FaultParams));
        FaultParams_from_byte_array((FaultParams*)*output, data_start);
        return 1;
    }
    
    return 0; // 未知类型
}

#endif // BASE_H

