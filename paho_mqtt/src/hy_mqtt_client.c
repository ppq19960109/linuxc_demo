#include "main.h"
#include "hy_mqtt_client.h"
#include "MQTTClient.h"

#define QOS 1

static void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
}

static int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    int i;
    char *payloadptr;

    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: ");

    payloadptr = message->payload;
    for (i = 0; i < message->payloadlen; i++)
    {
        putchar(*payloadptr++);
    }
    putchar('\n');
    printf("   message payload: %s\n", (char *)message->payload);
    hylinkDispatch(message->payload);

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

static void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}
//------------------------------
#define CTRL_TOPIC "honyar/Dispatch/%s/#"
#define REPORT_TOPIC "honyar/Report/%s/%s/%s"
static MQTTClient client;
static char gateway_mac[18];
static char subscribe_topic[80];
static char publish_topic[100];

int mqtt_client_publish(const char *topicName, char *payload)
{
    int ret;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;

    pubmsg.payload = payload;
    pubmsg.payloadlen = (int)strlen(payload);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    ret = MQTTClient_publishMessage(client, topicName, &pubmsg, &token);
    printf("Waiting for publication of payload %s topic %s\n", payload, topicName);
    return ret;
}

int mqtt_client_subscribe(const char *topicName)
{
    int ret;
    printf("mqtt client subsribe topic:%s\n", topicName);
    ret = MQTTClient_subscribe(client, topicName, QOS);
    if (ret != MQTTCLIENT_SUCCESS)
    {
        fprintf(stderr, "mqtt client subsribe topic fail:%d\n", ret);
    }
    return ret;
}
static int mqtt_Recv(void *recv, unsigned int len)
{
    char *json = (char *)recv;
    if (json == NULL)
        return -1;
    int ret = 0;
    cJSON *root = cJSON_Parse(json);
    if (root == NULL)
    {
        logError("root is NULL\n");
        return -1;
    }

    //Type字段
    cJSON *Type = cJSON_GetObjectItem(root, STR_TYPE);
    if (Type == NULL)
    {
        logError("Type is NULL\n");
        goto fail;
    }

    //Data字段
    cJSON *Data = cJSON_GetObjectItem(root, STR_DATA);
    if (Data == NULL)
    {
        logError("Data is NULL\n");
        goto fail;
    }

    char hyDevId[33] = {0};

    int array_size = cJSON_GetArraySize(Data);
    if (array_size == 0)
        goto fail;
    cJSON *array_sub = cJSON_GetArrayItem(Data, 0);
    getStrForJson(array_sub, STR_DEVICEID, hyDevId);

    sprintf(publish_topic, REPORT_TOPIC, gateway_mac, hyDevId, Type->valuestring);
    char *payload = cJSON_PrintUnformatted(root);
    ret = mqtt_client_publish(publish_topic, payload);
    cJSON_free(payload);

    cJSON_Delete(root);
    return ret;
fail:
    cJSON_Delete(root);
    return -1;
}

//------------------------------
int mqtt_client_close(void)
{
    MQTTClient_unsubscribe(client, subscribe_topic);

    MQTTClient_disconnect(client, 5000);
    MQTTClient_destroy(&client);
    return 0;
}

int mqtt_client_open(const char *serverURI, const char *clientId, const char *username, const char *password)
{
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    registerTransferCb(mqtt_Recv, TRANSFER_MQTT_REPORT);
    if (getNetworkMac(ETH_NAME, gateway_mac, sizeof(gateway_mac), "") == NULL)
    {
        fprintf(stderr, "get mac error\n");
        exit(1);
    }

    MQTTClient_create(&client, serverURI, clientId,
                      MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.username = username;
    conn_opts.password = password;
    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    // sprintf(subscribe_topic, CTRL_TOPIC, gateway_mac);
    // if (mqtt_client_subscribe(subscribe_topic) != MQTTCLIENT_SUCCESS)
    //     printf("mqtt_client_subscribe fail\n");
    // else
    //     printf("mqtt_client_subscribe success\n");

    return rc;
}
