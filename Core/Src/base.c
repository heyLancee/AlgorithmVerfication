#include "base.h"

// ?????????????
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


// ???????
void TelemetryStruct_to_byte_array(const TelemetryStruct* obj, uint8_t* output) {
    if(obj == NULL || output == NULL) {
        printf("Invalid input parameters!\n");
        return;
    }

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

// ????????
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

// ??????????????
size_t get_telemetry_encoded_length(void) {
    return sizeof(TelemetryStruct); // n?double = n*8??
}

// ??????????????
size_t get_faultparams_encoded_length(const FaultType fault_type) {
    // ????:2?double(?????????) + 2?int(?????ID) = 16+8 = 24??
    // ????:????
    return 24 + get_fault_param_count(fault_type) * sizeof(double);
}

// ???????????
size_t calculate_package_length(const PackageManager* pm, CommuDataType commu_type, FaultType fault_type) {
    if (pm == NULL) return 0;

    size_t data_len = 0;
    
    // ????????
    switch (commu_type) {
        case TELEMETRY:
            data_len = get_telemetry_encoded_length();
            break;
        case FAULT_PARA: {
            data_len = get_faultparams_encoded_length(fault_type);
            break;
        }
        default:
            return 0; // ??????
    }

    // ??? = ?? + ??(4??) + ?? + ??
    return pm->header_len + sizeof(uint32_t) + data_len + pm->tail_len;
}

// ?????????
void FaultParams_to_byte_array(const FaultParams* obj, uint8_t* output) {
    // ???????
    double* dptr = (double*)output;
    dptr[0] = obj->fault_start_time;
    dptr[1] = obj->fault_end_time;
    
    uint32_t* iptr = (uint32_t*)(output + 16); // ??2?double
    iptr[0] = (uint32_t)obj->fault_type;
    
    // ????ID
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
    
    // ????
    uint8_t param_count = get_fault_param_count(obj->fault_type);
    if (param_count > 0) {
        memcpy(output + 24, obj->params, param_count * sizeof(double));
    }
}

// ??????????
void FaultParams_from_byte_array(FaultParams* obj, const uint8_t* data) {
    // ???????
    const double* dptr = (const double*)data;
    obj->fault_start_time = dptr[0];
    obj->fault_end_time = dptr[1];
    
    const uint32_t* iptr = (const uint32_t*)(data + 16); // ??2?double
    obj->fault_type = (FaultType)iptr[0];
    int component_id = (int)iptr[1];
    
    // ????
    obj->param_count = get_fault_param_count(obj->fault_type);
    if (obj->param_count > 0) {
        memcpy(obj->params, data + 24, obj->param_count * sizeof(double));
    }
    
    // ????ID
    if (obj->fault_type == GYRO_INTERMITTENT_FAULT || 
        obj->fault_type == GYRO_SLOW_FAULT || 
        obj->fault_type == GYRO_MULTI_FAULT) {
        obj->gyro_fault_idx = component_id;
    } else if (obj->fault_type == FLYWHEEL_PARTIAL_LOSS || 
               obj->fault_type == FLYWHEEL_BIAS || 
               obj->fault_type == FLYWHEEL_COMPREHENSIVE) {
        obj->flywheel_fault_idx = component_id;
    }
    
    // ??????
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

// ???????
void PackageManager_init(PackageManager* pm, const char* header, const char* tail) {
    pm->header_len = strlen(header);
    pm->header = (uint8_t*)malloc(pm->header_len);
    memcpy(pm->header, header, pm->header_len);
    
    pm->tail_len = strlen(tail);
    pm->tail = (uint8_t*)malloc(pm->tail_len);
    memcpy(pm->tail, tail, pm->tail_len);
}

// ?????
void PackageManager_package(const PackageManager* pm, const void* data, CommuDataType commu_type, uint8_t* output) {
    /*
        param:
            pm: ???? [PackageManager]
            data: ?????? [TelemetryStruct, FaultParams]
            commu_type: ???? [CommuDataType]
            output: ?????????,??????????? [uint8_t*]
    */
    uint16_t pos = 0;
    
    // ????
    memcpy(output, pm->header, pm->header_len);
    pos += pm->header_len;
    
    // ????
    *((uint32_t*)(output + pos)) = (uint32_t)commu_type;
    pos += 4;
    
    // ????
    uint16_t data_len = 0;
    if (commu_type == TELEMETRY) {
        TelemetryStruct* pTelemetry = (TelemetryStruct*)data;
        data_len = sizeof(TelemetryStruct);
        TelemetryStruct_to_byte_array(pTelemetry, output + pos);
    } else if (commu_type == FAULT_PARA) {
        data_len = 24 + get_fault_param_count(((const FaultParams*)data)->fault_type) * sizeof(double);
        FaultParams_to_byte_array((const FaultParams*)data, output + pos);
    }
    pos += data_len;
    
    // ????
    memcpy(output + pos, pm->tail, pm->tail_len);
    pos += pm->tail_len;
}

// ?????
int PackageManager_unpackage(const PackageManager* pm, const uint8_t* package, uint16_t package_len, void** output, CommuDataType* output_type) {
    // ????
    if (package_len < pm->header_len + 4 + pm->tail_len) {
        printf("packet length is shorter than expected length %d\n", pm->header_len + 4 + pm->tail_len);
        return 0; // ?????
    }
    
    // ??????
    if (memcmp(package, pm->header, pm->header_len) != 0 || 
        memcmp(package + package_len - pm->tail_len, pm->tail, pm->tail_len) != 0) {
        printf("header or tail mismatch\n");
        return 0; // ???????
    }
    
    // ????
    uint32_t type_value = *((const uint32_t*)(package + pm->header_len));
    *output_type = (CommuDataType)type_value;
    
    // ????
    const uint8_t* data_start = package + pm->header_len + 4;
    uint16_t data_len = package_len - pm->header_len - 4 - pm->tail_len;
    
    if (*output_type == TELEMETRY) {
        if (data_len != get_telemetry_encoded_length()) {
            printf("telemetry struct data length mismatch\n");
            return 0; // ???????
        }
        *output = malloc(sizeof(TelemetryStruct));
        TelemetryStruct_from_byte_array((TelemetryStruct*)*output, data_start);
        return 1;
    } else if (*output_type == FAULT_PARA) {
        if (data_len != get_faultparams_encoded_length(((const FaultParams*)data_start)->fault_type)) {
            printf("fault params struct data length mismatch\n");
            return 0; // ???????
        }
        *output = malloc(sizeof(FaultParams));
        FaultParams_from_byte_array((FaultParams*)*output, data_start);
        return 1;
    }
    
    return 0; // ????
}
