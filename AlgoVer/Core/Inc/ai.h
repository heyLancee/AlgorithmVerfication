#ifndef __AI_H
#define __AI_H

#include "app_x-cube-ai.h"

#define AI_NAME AI_NETWORK_MODEL_NAME

typedef struct {
    ai_float** inputs;
    ai_float** outputs;
} AI_IOBuffer;

void AI_Init(void);
void AI_Run(ai_float *pIn[], ai_float *pOut);
AI_IOBuffer* AI_PrepareIO(telemetryStruct* pTelemetry);
void AI_FreeIO(AI_IOBuffer* buffer) ;

void AI_Demo(void);

ai_handle network;
ai_u8 activations[AI_NETWORK_DATA_ACTIVATIONS_SIZE];
ai_buffer* ai_input;
ai_buffer* ai_output;

#endif // __AI_H

