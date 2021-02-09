#ifndef _SCENE_H_
#define _SCENE_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include "scene_add.h"
    int sceneReport(void *req, unsigned int len);
    int sceneHyDispatch(cJSON *DataArray);

    int localScene(const int devid, const char *serviceid, const int serviceid_len, const char *request, char **response, int *response_len);

#ifdef __cplusplus
}
#endif
#endif
