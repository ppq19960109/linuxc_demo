#include "cJSON.h"
#include "sqlite3.h"
#include "log.h"

const char Command[] = {"Dispatch", "Report"};
const char test_json[] = {"{\
                            \"Command\" : \"Dispatch\", \
                            \"FrameNumber\" : \"00\",\
                            \"Type\" : \"Ctrl\",\
                            \"Data\" : [\
                                {\
                                    \"DeviceId\" : \"123456787654310\",\
                                    \"Key\" : \"Switch\",\
                                    \"Value\" : \"1\"\
                                }\
                            ]} "};

void protlcol_init()
{
}

int read_from_bottom(const char *json)
{
    cJSON *root = cJSON_Parse(test_json);
    log_debug("%s\n",cJSON_Print(root));
    return 0;
}

int write_to_bottom(void *ptr)
{

    return 0;
}

int read_from_cloud(void *ptr)
{

    return 0;
}

int write_to_cloud(void *ptr)
{

    return 0;
}

int cloud_to_bottom()
{
    return 0;
}

int bottom_to_cloud()
{
    return 0;
}