#include "base.h"


int main() {
    PackageManager pm;
    PackageManager_init(&pm, "SSSSSSSS", "EEEEEEEE");
    
    // 测试遥测数据
    TelemetryStruct telemetry_data;
    memset(&telemetry_data, 0, sizeof(TelemetryStruct));
    telemetry_data.timeStep = 1.0;
    telemetry_data.wx = 0.5;
    telemetry_data.wy = 0.6;
    telemetry_data.wz = 0.7;
    telemetry_data.zAngle = 45.0;

    // 打印遥测测试数据内容
    printf("TelemetryStruct content:\n");
    printf("timeStep: %f\n", telemetry_data.timeStep);
    printf("wx: %f\n", telemetry_data.wx);
    printf("wy: %f\n", telemetry_data.wy);
    printf("wz: %f\n", telemetry_data.wz);
    printf("zAngle: %f\n", telemetry_data.zAngle);
    
    // 计算下telemetry_package需要的长度，不要直接给1024
    uint16_t telemetry_package_len = calculate_package_length(&pm, TELEMETRY, NO_FAULT);
    uint8_t telemetry_package[telemetry_package_len];
    PackageManager_package(&pm, &telemetry_data, TELEMETRY, telemetry_package);

    printf("Telemetry package length: %d\n", telemetry_package_len);
    printf("-------------------------------------------\n");

    void* unpacked_data = NULL;
    CommuDataType unpacked_type;
    if (PackageManager_unpackage(&pm, telemetry_package, telemetry_package_len, &unpacked_data, &unpacked_type)) {
        if (unpacked_type == TELEMETRY) {
            TelemetryStruct* ts = (TelemetryStruct*)unpacked_data;
            // 打印解包后的数据内容
            printf("Unpacked TelemetryStruct content:\n");
            printf("timeStep: %f\n", ts->timeStep);
            printf("wx: %f\n", ts->wx);
            printf("wy: %f\n", ts->wy);
            printf("wz: %f\n", ts->wz);
            printf("zAngle: %f\n", ts->zAngle);
        }

        if (unpacked_data != NULL) {
            free(unpacked_data); // 释放内存 
        }
    }

    printf("\n");
    printf("--------------------故障参数测试-----------------------\n");

    // 测试故障参数
    FaultParams fault_params;
    memset(&fault_params, 0, sizeof(FaultParams));
    fault_params.fault_type = NO_FAULT;
    fault_params.params[0] = 0.5;
    fault_params.param_count = 0;
    fault_params.gyro_fault_idx = 1;
    fault_params.fault_start_time = 100.0;
    fault_params.fault_end_time = 200.0;

    // 打印故障参数测试数据内容
    printf("FaultParams content:\n");
    printf("fault_type: %d\n", fault_params.fault_type);
    printf("params[0]: %f\n", fault_params.params[0]);
    printf("param_count: %d\n", fault_params.param_count);
    printf("gyro_fault_idx: %d\n", fault_params.gyro_fault_idx);
    printf("fault_start_time: %f\n", fault_params.fault_start_time);
    printf("fault_end_time: %f\n", fault_params.fault_end_time);
    
    uint16_t fault_package_len = calculate_package_length(&pm, FAULT_PARA, fault_params.fault_type);
    uint8_t fault_package[fault_package_len];
    PackageManager_package(&pm, &fault_params, FAULT_PARA, fault_package);

    printf("Fault package length: %d\n", fault_package_len);
    printf("-------------------------------------------\n");
    
    if (PackageManager_unpackage(&pm, fault_package, fault_package_len, &unpacked_data, &unpacked_type)) {
        if (unpacked_type == FAULT_PARA) {
            FaultParams* fp = (FaultParams*)unpacked_data;
            // 打印解包后的数据内容
            printf("Unpacked FaultParams content:\n");
            printf("fault_type: %d\n", fp->fault_type);
            printf("params[0]: %f\n", fp->params[0]);
            printf("param_count: %d\n", fp->param_count);
            printf("gyro_fault_idx: %d\n", fp->gyro_fault_idx);
            printf("fault_start_time: %f\n", fp->fault_start_time);
            printf("fault_end_time: %f\n", fp->fault_end_time);
        }

        if (unpacked_data!= NULL) {
            free(unpacked_data); // 释放内存 
        }
    }
    
    return 0;
}
