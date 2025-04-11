#ifndef __AI_H
#define __AI_H

#include "app_x-cube-ai.h"
#include "base.h"

#define AI_NAME AI_NETWORK_MODEL_NAME

typedef struct {
    ai_float** inputs;
    ai_float** outputs;
} AI_IOBuffer;

void AI_Init(void);
int AI_Run_Inference(TelemetryStruct *pTelemetry);
void AI_DeInit(void);

extern ai_handle m_network;
extern ai_u8 m_activations[AI_NETWORK_DATA_ACTIVATIONS_SIZE];
extern ai_buffer* m_ai_input;
extern ai_buffer* m_ai_output;

#endif // __AI_H

