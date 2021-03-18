#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "logFunc.h"
#include "cJSON.h"
#include "hylinkRecv.h"
#include "hylinkSend.h"

#include "scene.h"
#include "action_list.h"

#define CMD_DATA_LEN (1024)
#define CMD_TYPE_LOCAL_SCENE "LocalScene" //本地场景指令

/*由于本地场景下发的控制指令，无法携带会话相关属性，所有只能新开接口*/
static int hylink_scene_send(char *pCmd, int cmdLen)
{
    logWarn("local scene cmd:%s\n", pCmd);
    cJSON *root = cJSON_Parse(pCmd);

    //command字段
    cJSON *Command = cJSON_GetObjectItem(root, STR_COMMAND);
    if (Command == NULL)
    {
        logError("Command is NULL\n");
        goto fail;
    }

    if (0 == strcmp(Command->valuestring, CMD_TYPE_LOCAL_SCENE))
    {
        /*场景处理*/
        scene_query(pCmd,
                    CMD_DATA_LEN,
                    NULL,
                    0,
                    NULL);
    }
    else
    {
        hylinkDispatch(pCmd, strlen(pCmd), 1);
    }
    cJSON_Delete(root);
    return 0;
fail:
    cJSON_Delete(root);
    return -1;
}

static int scene_adapter_register_report(void *data)
{
    logError("local scene scene_adapter_register_report:%s\n", data);
    return hylinkDispatch(data, strlen(data), 0);
}

int scene_adapter_open(void)
{
    hy_scene_command_register(hylink_scene_send);
    scene_report_reg(scene_adapter_register_report);
    return scene_init("./scene_info.db", 0);
}
int scene_adapter_close(void)
{
    return scene_destroy();
}
int scene_adapter_reset(void)
{
    return scene_clean();
}
int scene_adapter_cmd(char *inputBuff, int inputSize)
{
    logWarn("local scene scene_adapter_cmd:%s\n", inputBuff);
    return scene_query(inputBuff, inputSize, NULL, 0, NULL);
}
int scene_adapter_report(char *inputBuff, int inputSize)
{
    logWarn("local scene scene_adapter_report:%s\n", inputBuff);
    return scene_event(inputBuff);
}
