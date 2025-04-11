#include "ai.h"
#include "base.h"

static  ai_handle m_network = AI_HANDLE_NULL;

AI_ALIGNED(32)
static ai_u8 m_activations[AI_NETWORK_DATA_ACTIVATIONS_SIZE] = {0};

AI_ALIGNED(32)
static ai_float in_data[AI_NETWORK_IN_1_SIZE];

AI_ALIGNED(32)
static ai_float out_data[AI_NETWORK_OUT_1_SIZE];

ai_buffer* m_ai_input = NULL;
ai_buffer* m_ai_output = NULL;

// 初始化 AI 模型
void AI_Init(void) {
    if (m_network != AI_HANDLE_NULL) {
        return; // 已经初始化
    }

    ai_error err;

    // 创建并初始化 AI 网络
    const ai_handle act_addr[] = { m_activations };
    err = ai_network_create_and_init(&m_network, act_addr, NULL);
    if (err.type != AI_ERROR_NONE) {
        printf("AI 初始化失败: type=%d, code=%d\n", err.type, err.code);
        return;
    }

    // 获取输入/输出缓冲区
    m_ai_input = ai_network_inputs_get(m_network, NULL);
    m_ai_output = ai_network_outputs_get(m_network, NULL);

    if (m_ai_input == NULL || m_ai_output == NULL) {
        printf("获取输入/输出缓冲区失败！\n");
        ai_network_destroy(m_network);
        m_network = AI_HANDLE_NULL;
        return;
    }

    printf("AI 初始化成功！\n");
}

// 准备输入数据并运行推理
int AI_Run_Inference(TelemetryStruct *pTelemetry) {
    if (m_network == AI_HANDLE_NULL || m_ai_input == NULL || m_ai_output == NULL) {
        printf("AI 未初始化！\n");
        return -1;
    }

    m_ai_input[0].data = AI_HANDLE_PTR(in_data);
    m_ai_output[0].data = AI_HANDLE_PTR(out_data);

    // 填充输入数据（示例：四元数 + 角速度）
    in_data[0] = pTelemetry->q0;
    in_data[1] = pTelemetry->q1;
    in_data[2] = pTelemetry->q2;
    in_data[3] = pTelemetry->q3;
    in_data[4] = pTelemetry->wx;
    in_data[5] = pTelemetry->wy;
    in_data[6] = pTelemetry->wz;
    in_data[7] = pTelemetry->rx;
    in_data[8] = pTelemetry->ry;
    in_data[9] = pTelemetry->rz;

    // 运行推理
    ai_i32 batch = ai_network_run(m_network, &m_ai_input[0], &m_ai_output[0]);
    if (batch <= 0) {
        ai_error err = ai_network_get_error(m_network);
        printf("推理失败: batch=%d, type=%d, code=%d\n", batch, err.type, err.code);
        return -1;
    }

    // 获取输出结果（假设单输出）
    float* output_data = (float*)m_ai_output[0].data;
    printf("推理结果: %.2f, %.2f, %.2f\n", output_data[0], output_data[1], output_data[2]);

    return 0;
}

// 释放资源
void AI_DeInit(void) {
    if (m_network != AI_HANDLE_NULL) {
        // 释放手动分配的内存
        if (m_ai_input != NULL && m_ai_input[0].data != NULL) {
            free(m_ai_input[0].data);
        }
        if (m_ai_output != NULL && m_ai_output[0].data != NULL) {
            free(m_ai_output[0].data);
        }

        ai_network_destroy(m_network);
        m_network = AI_HANDLE_NULL;
        m_ai_input = NULL;
        m_ai_output = NULL;
        printf("AI 资源已释放！\n");
    }
}