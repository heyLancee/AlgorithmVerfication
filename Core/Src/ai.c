#include "ai.h"
#include "base.h"

ai_handle m_network = AI_HANDLE_NULL;
ai_u8 m_activations[AI_NETWORK_DATA_ACTIVATIONS_SIZE] = {0};
ai_buffer* m_ai_input = NULL;
ai_buffer* m_ai_output = NULL;

void AI_Init(void)
{
    if (m_network != AI_HANDLE_NULL) {
        return;
    }
	ai_error err;
	const ai_handle act_addr[] = { m_activations };
		
	// 构建AI实例
	err = ai_network_create_and_init(&m_network, act_addr, NULL);	// network为实例对象
	if (err.type != AI_ERROR_NONE) {
		printf("E: AI error - type=%d code=%d\r\n", err.type, err.code);
	}
	
	// 数据赋值
	m_ai_input = ai_network_inputs_get(m_network, NULL);
	m_ai_output = ai_network_outputs_get(m_network, NULL);
}

void AI_Run(ai_float *pIn[], ai_float *pOut[]) {
	ai_error err;

    // 更新IO句柄
    for (int i = 0; i < AI_NETWORK_IN_NUM; i++) {
        m_ai_input[i].data = AI_HANDLE_PTR(pIn[i]);
    }
    for (int i = 0; i < AI_NETWORK_OUT_NUM; i++) {
        m_ai_output[i].data = AI_HANDLE_PTR(pOut[i]);
    }

    // 运行推理
    ai_i32 batch = ai_network_run(m_network, m_ai_input, m_ai_output);
    if (batch <= 0) {
        printf("E: AI network run failed - batch=%d\r\n", batch);
        return;
    }
}

void AI_Demo(void)
{
    // 为每个输入准备数据缓冲区
    ai_float* input_data[AI_NETWORK_IN_NUM];  // 输入数据缓冲区数组
    for(int n = 0; n < AI_NETWORK_IN_NUM; n++) {
        // 获取当前输入的大小
        int size = 0;
        if (n < AI_NETWORK_IN_NUM) {  // 首先检查是否在有效范围内
            switch(n) {
                case 0: 
                    #ifdef AI_NETWORK_IN_1_SIZE
                    size = AI_NETWORK_IN_1_SIZE; 
                    #endif
                    break;
                case 1: 
                    #ifdef AI_NETWORK_IN_2_SIZE
                    size = AI_NETWORK_IN_2_SIZE; 
                    #endif
                    break;
                case 2: 
                    #ifdef AI_NETWORK_IN_3_SIZE
                    size = AI_NETWORK_IN_3_SIZE; 
                    #endif
                    break;
                default: 
                    printf("E: Invalid input index %d\r\n", n);
                    break;
            }
        }

        if (size == 0) {
            printf("E: Failed to get size for input %d\r\n", n);
            return;
        }
        
        input_data[n] = (ai_float*)malloc(size * sizeof(ai_float));
        
        // 填充示例数据
        for(int i = 0; i < size; i++) {
            input_data[n][i] = (ai_float)(i + 1 + n * size);
        }
    }
    
    // 为每个输出准备数据缓冲区
    ai_float* output_data[AI_NETWORK_OUT_NUM];
    for(int n = 0; n < AI_NETWORK_OUT_NUM; n++) {
        // 获取当前输出的大小
        int size = 0;
        if (n < AI_NETWORK_OUT_NUM) {
            switch(n) {
                case 0: 
                    #ifdef AI_NETWORK_OUT_1_SIZE
                    size = AI_NETWORK_OUT_1_SIZE; 
                    #endif
                    break;
                case 1: 
                    #ifdef AI_NETWORK_OUT_2_SIZE
                    size = AI_NETWORK_OUT_2_SIZE; 
                    #endif
                    break;
                case 2: 
                    #ifdef AI_NETWORK_OUT_3_SIZE
                    size = AI_NETWORK_OUT_3_SIZE; 
                    #endif
                    break;
                default: 
                    printf("E: Invalid output index %d\r\n", n);
                    break;
            }
        }

        if (size == 0) {
            printf("E: Failed to get size for output %d\r\n", n);
            return;
        }

        output_data[n] = (ai_float*)malloc(size * sizeof(ai_float));
        memset(output_data[n], 0, size * sizeof(ai_float));
    }
    
    // 初始化 AI
    AI_Init();
    
    // 运行推理
    AI_Run(input_data, output_data);
    
    // 打印所有输入数据
    for(int n = 0; n < AI_NETWORK_IN_NUM; n++) {
        int size = 0;
        switch(n) {
            case 0: 
                #ifdef AI_NETWORK_IN_1_SIZE
                size = AI_NETWORK_IN_1_SIZE; 
                #endif
                break;
            case 1: 
                #ifdef AI_NETWORK_IN_2_SIZE
                size = AI_NETWORK_IN_2_SIZE; 
                #endif
                break;
            case 2: 
                #ifdef AI_NETWORK_IN_3_SIZE
                size = AI_NETWORK_IN_3_SIZE; 
                #endif
                break;
            default: 
                size = 0; 
                break;
        }
        
        printf("Input %d[%d]: [", n+1, size);
        for(int i = 0; i < size; i++) {
            printf("%.2f", input_data[n][i]);
            if(i < size - 1) printf(", ");
        }
        printf("]\r\n");
    }
    
    // 打印所有输出数据
    for(int n = 0; n < AI_NETWORK_OUT_NUM; n++) {
        int size = 0;
        switch(n) {
            case 0: 
                #ifdef AI_NETWORK_OUT_1_SIZE
                size = AI_NETWORK_OUT_1_SIZE; 
                #endif
                break;
            case 1: 
                #ifdef AI_NETWORK_OUT_2_SIZE
                size = AI_NETWORK_OUT_2_SIZE; 
                #endif
                break;
            case 2: 
                #ifdef AI_NETWORK_OUT_3_SIZE
                size = AI_NETWORK_OUT_3_SIZE; 
                #endif
                break;
            default: 
                size = 0; 
                break;
        }

        printf("Output %d[%d]: [", n+1, size);
        for(int i = 0; i < size; i++) {
            printf("%.2f", output_data[n][i]);
            if(i < size - 1) printf(", ");
        }
        printf("]\r\n");
    }
    
    // 释放内存
    for(int n = 0; n < AI_NETWORK_IN_NUM; n++) {
        free(input_data[n]);
    }
    for(int n = 0; n < AI_NETWORK_OUT_NUM; n++) {
        free(output_data[n]);
    }
}

AI_IOBuffer* AI_PrepareIO(TelemetryStruct* pTelemetry) {
    AI_IOBuffer* buffer = (AI_IOBuffer*)malloc(sizeof(AI_IOBuffer));
    
    // 分配输入输出指针数组
    buffer->inputs = (ai_float**)malloc(AI_NETWORK_IN_NUM * sizeof(ai_float*));
    buffer->outputs = (ai_float**)malloc(AI_NETWORK_OUT_NUM * sizeof(ai_float*));
    
    // 为每个输入分配内存
    for(int n = 0; n < AI_NETWORK_IN_NUM; n++) {
        int size = 0;
        if (n < AI_NETWORK_IN_NUM) {
            switch(n) {
                case 0: 
                    #ifdef AI_NETWORK_IN_1_SIZE
                    size = AI_NETWORK_IN_1_SIZE; 
                    #endif
                    break;
                case 1: 
                    #ifdef AI_NETWORK_IN_2_SIZE
                    size = AI_NETWORK_IN_2_SIZE; 
                    #endif
                    break;
                case 2: 
                    #ifdef AI_NETWORK_IN_3_SIZE
                    size = AI_NETWORK_IN_3_SIZE; 
                    #endif
                    break;
                default: 
                    printf("E: Invalid input index %d\r\n", n);
                    break;
            }
        }

        if (size == 0) {
            printf("E: Failed to get size for input %d\r\n", n);
            AI_FreeIO(buffer);  // 释放已分配的内存
            return NULL;
        }
        
        buffer->inputs[n] = (ai_float*)malloc(size * sizeof(ai_float));
        
        // TODO: 从 pTelemetry 中填充输入数据
    }
    
    // 为每个输出分配内存
    for(int n = 0; n < AI_NETWORK_OUT_NUM; n++) {
        int size = 0;
        if (n < AI_NETWORK_OUT_NUM) {
            switch(n) {
                case 0: 
                    #ifdef AI_NETWORK_OUT_1_SIZE
                    size = AI_NETWORK_OUT_1_SIZE; 
                    #endif
                    break;
                case 1: 
                    #ifdef AI_NETWORK_OUT_2_SIZE
                    size = AI_NETWORK_OUT_2_SIZE; 
                    #endif
                    break;
                case 2: 
                    #ifdef AI_NETWORK_OUT_3_SIZE
                    size = AI_NETWORK_OUT_3_SIZE; 
                    #endif
                    break;
                default: 
                    printf("E: Invalid output index %d\r\n", n);
                    break;
            }
        }

        if (size == 0) {
            printf("E: Failed to get size for output %d\r\n", n);
            AI_FreeIO(buffer);  // 释放已分配的内存
            return NULL;
        }
        
        buffer->outputs[n] = (ai_float*)malloc(size * sizeof(ai_float));
        memset(buffer->outputs[n], 0, size * sizeof(ai_float));
    }
    
    return buffer;
}

// 释放IO缓冲区的内存
void AI_FreeIO(AI_IOBuffer* buffer) {
    if (buffer == NULL) return;
    
    if (buffer->inputs != NULL) {
        for(int n = 0; n < AI_NETWORK_IN_NUM; n++) {
            free(buffer->inputs[n]);
        }
        free(buffer->inputs);
    }
    
    if (buffer->outputs != NULL) {
        for(int n = 0; n < AI_NETWORK_OUT_NUM; n++) {
            free(buffer->outputs[n]);
        }
        free(buffer->outputs);
    }
    
    free(buffer);
}

