#ifndef _SCENE_ADAPTER_H_
#define _SCENE_ADAPTER_H_

#ifdef __cplusplus
extern "C"
{
#endif

int scene_adapter_open(void);
int scene_adapter_close(void);
int scene_adapter_reset(void);
int scene_adapter_cmd(char *inputBuff, int inputSize);
int scene_adapter_report(char *inputBuff, int inputSize);

#ifdef __cplusplus
}
#endif
#endif