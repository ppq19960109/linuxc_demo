#ifndef _SCENE_ADD_H_
#define _SCENE_ADD_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include "cJSON.h"
#include "scene_database.h"

int addtriggers(const char *localSceneId, cJSON *triggersItems);
int addconditions(const char *localSceneId, cJSON *conditionsItems);
int addActions(const char *localSceneId, cJSON *actions);

#ifdef __cplusplus
}
#endif
#endif
