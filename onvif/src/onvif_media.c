#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "soapH.h"
#include "soapStub.h"
#include "onvif_server.h"

#define ONVIF_FRAME_WIDTH 1920
#define ONVIF_FRAME_HEIGHT 1080

#define ONVIF_TIME_OUT 10
//----
#define PROFILE_NUM 1
#define PROFILE_NAME "MediaProfile_Name"
#define PROFILE_TOKEN "MediaProfile000"
#define PROFILE_VIDEO_SOURCECONFIG_NAME "SourceConfig_Name"
#define PROFILE_VIDEO_SOURCECONFIG_TOKEN "000"
#define PROFILE_VIDEO_SOURCECONFIG_SOURCETOKEN "000"
//----
#define VIDEO_ENCODE_NUM 1
#define VIDEO_ENCODE_NAME "EncoderConfig_Name"
#define VIDEO_ENCODE_TOKEN "000"
//--
#define VIDEO_SOURCE_CONFIG_NUM 1
#define VIDEO_SOURCE_CONFIG_NAME "SourceConfig_Name"
#define VIDEO_SOURCE_CONFIG_TOKEN "000"
#define VIDEO_SOURCE_CONFIG_SOURCETOKEN "000"
//------
#define H264_PROFILE tt__H264Profile__High
#define H264_FRAMERATE 25
#define H264_GOVLENGTH 60

/** Web service operation '__trt__GetServiceCapabilities' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetServiceCapabilities(struct soap *soap, struct _trt__GetServiceCapabilities *trt__GetServiceCapabilities, struct _trt__GetServiceCapabilitiesResponse *trt__GetServiceCapabilitiesResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetVideoSources' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSources(struct soap *soap, struct _trt__GetVideoSources *trt__GetVideoSources, struct _trt__GetVideoSourcesResponse *trt__GetVideoSourcesResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetAudioSources' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSources(struct soap *soap, struct _trt__GetAudioSources *trt__GetAudioSources, struct _trt__GetAudioSourcesResponse *trt__GetAudioSourcesResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetAudioOutputs' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputs(struct soap *soap, struct _trt__GetAudioOutputs *trt__GetAudioOutputs, struct _trt__GetAudioOutputsResponse *trt__GetAudioOutputsResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__CreateProfile' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__CreateProfile(struct soap *soap, struct _trt__CreateProfile *trt__CreateProfile, struct _trt__CreateProfileResponse *trt__CreateProfileResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
static int get_profile(struct soap *soap, int id, struct tt__Profile *Profile)
{
    //<profiles><name>和<profiles><token>
    Profile->Name = (char *)soap_malloc(soap, sizeof(char) * 32);
    memset(Profile->Name, '\0', sizeof(char) * 32);

    strcpy(Profile->Name, PROFILE_NAME);
    Profile->token = (char *)soap_malloc(soap, sizeof(char) * 32);
    memset(Profile->token, '\0', sizeof(char) * 32);
    strcpy(Profile->token, PROFILE_TOKEN);
    Profile->fixed = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
    *(Profile->fixed) = xsd__boolean__true_;

    //<VideoSourceConfiguration><name>和<VideoSourceConfiguration><token>
    Profile->VideoSourceConfiguration = (struct tt__VideoSourceConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoSourceConfiguration));
    memset(Profile->VideoSourceConfiguration, 0, sizeof(struct tt__VideoSourceConfiguration));
    Profile->VideoSourceConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * 32);
    memset(Profile->VideoSourceConfiguration->Name, '\0', sizeof(char) * 32);
    strcpy(Profile->VideoSourceConfiguration->Name, PROFILE_VIDEO_SOURCECONFIG_NAME);
    Profile->VideoSourceConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * 32);
    memset(Profile->VideoSourceConfiguration->token, '\0', sizeof(char) * 32);
    strcpy(Profile->VideoSourceConfiguration->token, PROFILE_VIDEO_SOURCECONFIG_TOKEN);
    Profile->VideoSourceConfiguration->SourceToken = (char *)soap_malloc(soap, sizeof(char) * 32);
    memset(Profile->VideoSourceConfiguration->SourceToken, '\0', sizeof(char) * 32);
    strcpy(Profile->VideoSourceConfiguration->SourceToken, PROFILE_VIDEO_SOURCECONFIG_SOURCETOKEN);
    Profile->VideoSourceConfiguration->UseCount = 1;
    //<VideoSourceConfiguration><Bounds>
    Profile->VideoSourceConfiguration->Bounds = (struct tt__IntRectangle *)soap_malloc(soap, sizeof(struct tt__IntRectangle));
    memset(Profile->VideoSourceConfiguration->Bounds, 0, sizeof(struct tt__IntRectangle));
    Profile->VideoSourceConfiguration->Bounds->x = 0;
    Profile->VideoSourceConfiguration->Bounds->y = 0;
    Profile->VideoSourceConfiguration->Bounds->width = ONVIF_FRAME_WIDTH;
    Profile->VideoSourceConfiguration->Bounds->height = ONVIF_FRAME_HEIGHT;

    //<VideoEncoderConfiguration>
    Profile->VideoEncoderConfiguration = (struct tt__VideoEncoderConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoEncoderConfiguration));
    memset(Profile->VideoEncoderConfiguration, '\0', sizeof(struct tt__VideoEncoderConfiguration));
    Profile->VideoEncoderConfiguration->Name = (char *)soap_malloc(soap, sizeof(char) * 32);
    memset(Profile->VideoEncoderConfiguration->Name, '\0', sizeof(char) * 32);
    strcpy(Profile->VideoEncoderConfiguration->Name, VIDEO_ENCODE_NAME);
    Profile->VideoEncoderConfiguration->token = (char *)soap_malloc(soap, sizeof(char) * 32);
    memset(Profile->VideoEncoderConfiguration->token, '\0', sizeof(char) * 32);
    strcpy(Profile->VideoEncoderConfiguration->token, VIDEO_ENCODE_TOKEN);
    Profile->VideoEncoderConfiguration->UseCount = 1;
    Profile->VideoEncoderConfiguration->Encoding = tt__VideoEncoding__H264;

    //<VideoEncoderConfiguration><Resolution>、<RateControl>
    Profile->VideoEncoderConfiguration->Resolution = (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));
    memset(Profile->VideoEncoderConfiguration->Resolution, '\0', sizeof(struct tt__VideoResolution));
    Profile->VideoEncoderConfiguration->Resolution->Width = ONVIF_FRAME_WIDTH;
    Profile->VideoEncoderConfiguration->Resolution->Height = ONVIF_FRAME_HEIGHT;
    Profile->VideoEncoderConfiguration->Quality = 6;
    //<VideoEncoderConfiguration><RateControl>
    Profile->VideoEncoderConfiguration->RateControl = (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));
    memset(Profile->VideoEncoderConfiguration->RateControl, '\0', sizeof(struct tt__VideoRateControl));
    Profile->VideoEncoderConfiguration->RateControl->FrameRateLimit = H264_FRAMERATE;
    Profile->VideoEncoderConfiguration->RateControl->EncodingInterval = 1;
    Profile->VideoEncoderConfiguration->RateControl->BitrateLimit = 4096;

    //<VideoEncoderConfiguration><H264>
    Profile->VideoEncoderConfiguration->H264 = (struct tt__H264Configuration *)soap_malloc(soap, sizeof(struct tt__H264Configuration));
    memset(Profile->VideoEncoderConfiguration->H264, '\0', sizeof(struct tt__H264Configuration));
    Profile->VideoEncoderConfiguration->H264->GovLength = H264_GOVLENGTH;
    Profile->VideoEncoderConfiguration->H264->H264Profile = H264_PROFILE;

    return 0;
}
/** Web service operation '__trt__GetProfile' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetProfile(struct soap *soap, struct _trt__GetProfile *trt__GetProfile, struct _trt__GetProfileResponse *trt__GetProfileResponse)
{
    printf("call:%s\n", __func__);
    printf("trt__GetProfile=%s\n", trt__GetProfile->ProfileToken);

    trt__GetProfileResponse->Profile = (struct tt__Profile *)soap_malloc(soap, sizeof(struct tt__Profile));
    memset(trt__GetProfileResponse->Profile, '\0', sizeof(struct tt__Profile));

    get_profile(soap, 0, trt__GetProfileResponse->Profile);
    return 0;
}
/** Web service operation '__trt__GetProfiles' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetProfiles(struct soap *soap, struct _trt__GetProfiles *trt__GetProfiles, struct _trt__GetProfilesResponse *trt__GetProfilesResponse)
{
    printf("call:%s\n", __func__);
    // int i;
    // trt__GetProfilesResponse->__sizeProfiles = PROFILE_NUM;
    // trt__GetProfilesResponse->Profiles = (struct tt__Profile *)soap_malloc(soap, sizeof(struct tt__Profile) * trt__GetProfilesResponse->__sizeProfiles);
    // for (i = 0; i < trt__GetProfilesResponse->__sizeProfiles; i++)
    // {
    //     get_profile(soap, i, trt__GetProfilesResponse->Profiles + i);
    // }
    return SOAP_OK;
}
/** Web service operation '__trt__AddVideoEncoderConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoEncoderConfiguration(struct soap *soap, struct _trt__AddVideoEncoderConfiguration *trt__AddVideoEncoderConfiguration, struct _trt__AddVideoEncoderConfigurationResponse *trt__AddVideoEncoderConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__AddVideoSourceConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoSourceConfiguration(struct soap *soap, struct _trt__AddVideoSourceConfiguration *trt__AddVideoSourceConfiguration, struct _trt__AddVideoSourceConfigurationResponse *trt__AddVideoSourceConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__AddAudioEncoderConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioEncoderConfiguration(struct soap *soap, struct _trt__AddAudioEncoderConfiguration *trt__AddAudioEncoderConfiguration, struct _trt__AddAudioEncoderConfigurationResponse *trt__AddAudioEncoderConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__AddAudioSourceConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioSourceConfiguration(struct soap *soap, struct _trt__AddAudioSourceConfiguration *trt__AddAudioSourceConfiguration, struct _trt__AddAudioSourceConfigurationResponse *trt__AddAudioSourceConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__AddPTZConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddPTZConfiguration(struct soap *soap, struct _trt__AddPTZConfiguration *trt__AddPTZConfiguration, struct _trt__AddPTZConfigurationResponse *trt__AddPTZConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__AddVideoAnalyticsConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoAnalyticsConfiguration(struct soap *soap, struct _trt__AddVideoAnalyticsConfiguration *trt__AddVideoAnalyticsConfiguration, struct _trt__AddVideoAnalyticsConfigurationResponse *trt__AddVideoAnalyticsConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__AddMetadataConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddMetadataConfiguration(struct soap *soap, struct _trt__AddMetadataConfiguration *trt__AddMetadataConfiguration, struct _trt__AddMetadataConfigurationResponse *trt__AddMetadataConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__AddAudioOutputConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioOutputConfiguration(struct soap *soap, struct _trt__AddAudioOutputConfiguration *trt__AddAudioOutputConfiguration, struct _trt__AddAudioOutputConfigurationResponse *trt__AddAudioOutputConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__AddAudioDecoderConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioDecoderConfiguration(struct soap *soap, struct _trt__AddAudioDecoderConfiguration *trt__AddAudioDecoderConfiguration, struct _trt__AddAudioDecoderConfigurationResponse *trt__AddAudioDecoderConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__RemoveVideoEncoderConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoEncoderConfiguration(struct soap *soap, struct _trt__RemoveVideoEncoderConfiguration *trt__RemoveVideoEncoderConfiguration, struct _trt__RemoveVideoEncoderConfigurationResponse *trt__RemoveVideoEncoderConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__RemoveVideoSourceConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoSourceConfiguration(struct soap *soap, struct _trt__RemoveVideoSourceConfiguration *trt__RemoveVideoSourceConfiguration, struct _trt__RemoveVideoSourceConfigurationResponse *trt__RemoveVideoSourceConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__RemoveAudioEncoderConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioEncoderConfiguration(struct soap *soap, struct _trt__RemoveAudioEncoderConfiguration *trt__RemoveAudioEncoderConfiguration, struct _trt__RemoveAudioEncoderConfigurationResponse *trt__RemoveAudioEncoderConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__RemoveAudioSourceConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioSourceConfiguration(struct soap *soap, struct _trt__RemoveAudioSourceConfiguration *trt__RemoveAudioSourceConfiguration, struct _trt__RemoveAudioSourceConfigurationResponse *trt__RemoveAudioSourceConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__RemovePTZConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemovePTZConfiguration(struct soap *soap, struct _trt__RemovePTZConfiguration *trt__RemovePTZConfiguration, struct _trt__RemovePTZConfigurationResponse *trt__RemovePTZConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__RemoveVideoAnalyticsConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoAnalyticsConfiguration(struct soap *soap, struct _trt__RemoveVideoAnalyticsConfiguration *trt__RemoveVideoAnalyticsConfiguration, struct _trt__RemoveVideoAnalyticsConfigurationResponse *trt__RemoveVideoAnalyticsConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__RemoveMetadataConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveMetadataConfiguration(struct soap *soap, struct _trt__RemoveMetadataConfiguration *trt__RemoveMetadataConfiguration, struct _trt__RemoveMetadataConfigurationResponse *trt__RemoveMetadataConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__RemoveAudioOutputConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioOutputConfiguration(struct soap *soap, struct _trt__RemoveAudioOutputConfiguration *trt__RemoveAudioOutputConfiguration, struct _trt__RemoveAudioOutputConfigurationResponse *trt__RemoveAudioOutputConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__RemoveAudioDecoderConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioDecoderConfiguration(struct soap *soap, struct _trt__RemoveAudioDecoderConfiguration *trt__RemoveAudioDecoderConfiguration, struct _trt__RemoveAudioDecoderConfigurationResponse *trt__RemoveAudioDecoderConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__DeleteProfile' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__DeleteProfile(struct soap *soap, struct _trt__DeleteProfile *trt__DeleteProfile, struct _trt__DeleteProfileResponse *trt__DeleteProfileResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}

static int get_videosource_config(struct soap *soap, int id, struct tt__VideoSourceConfiguration *vs_config)
{
    vs_config->UseCount = 1;
    vs_config->Name = (char *)soap_malloc(soap, sizeof(char) * 32);
    memset(vs_config->Name, '\0', sizeof(char) * 32);
    strcpy(vs_config->Name, VIDEO_SOURCE_CONFIG_NAME);

    vs_config->token = (char *)soap_malloc(soap, sizeof(char) * 32);
    memset(vs_config->token, '\0', sizeof(char) * 32);
    strcpy(vs_config->token, VIDEO_SOURCE_CONFIG_TOKEN);

    vs_config->SourceToken = (char *)soap_malloc(soap, sizeof(char) * 32);
    memset(vs_config->SourceToken, '\0', sizeof(char) * 32);
    strcpy(vs_config->SourceToken, VIDEO_SOURCE_CONFIG_SOURCETOKEN);

    vs_config->Bounds = (struct tt__IntRectangle *)soap_malloc(soap, sizeof(struct tt__IntRectangle));
    memset(vs_config->Bounds, 0, sizeof(struct tt__IntRectangle));
    vs_config->Bounds->x = 0;
    vs_config->Bounds->y = 0;
    vs_config->Bounds->width = ONVIF_FRAME_WIDTH;
    vs_config->Bounds->height = ONVIF_FRAME_HEIGHT;

    return 0;
}
/** Web service operation '__trt__GetVideoSourceConfigurations' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfigurations(struct soap *soap, struct _trt__GetVideoSourceConfigurations *trt__GetVideoSourceConfigurations, struct _trt__GetVideoSourceConfigurationsResponse *trt__GetVideoSourceConfigurationsResponse)
{
    printf("call:%s\n", __func__);
    trt__GetVideoSourceConfigurationsResponse->__sizeConfigurations = VIDEO_SOURCE_CONFIG_NUM;
    trt__GetVideoSourceConfigurationsResponse->Configurations =
        (struct tt__VideoSourceConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoSourceConfiguration) * trt__GetVideoSourceConfigurationsResponse->__sizeConfigurations);
    memset(trt__GetVideoSourceConfigurationsResponse->Configurations, '\0', sizeof(struct tt__VideoSourceConfiguration) * trt__GetVideoSourceConfigurationsResponse->__sizeConfigurations);

    for (int i = 0; i < trt__GetVideoSourceConfigurationsResponse->__sizeConfigurations; ++i)
    {
        get_videosource_config(soap, i, trt__GetVideoSourceConfigurationsResponse->Configurations + i);
    }
    return 0;
}

static int get_videoencoder_config(struct soap *soap, int id, struct tt__VideoEncoderConfiguration *Configurations)
{
    //<VideoEncoderConfigurations>

    Configurations->Name = (char *)soap_malloc(soap, sizeof(char) * 32);
    memset(Configurations->Name, '\0', sizeof(char) * 32);
    Configurations->token = (char *)soap_malloc(soap, sizeof(char) * 32);
    memset(Configurations->token, '\0', sizeof(char) * 32);

    strcpy(Configurations->Name, VIDEO_ENCODE_NAME);
    strcpy(Configurations->token, VIDEO_ENCODE_TOKEN);
    Configurations->UseCount = 1;
    Configurations->Quality = 1;
    Configurations->Encoding = tt__VideoEncoding__H264; // JPEG = 0 , MPEG = 1, H264 = 2;
    //<Configurations><Resolution>
    Configurations->Resolution = (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution));
    memset(Configurations->Resolution, 0, sizeof(struct tt__VideoResolution));
    Configurations->Resolution->Width = ONVIF_FRAME_WIDTH;
    Configurations->Resolution->Height = ONVIF_FRAME_HEIGHT;
    //<Configurations><RateControl>
    Configurations->RateControl = (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));
    memset(Configurations->RateControl, 0, sizeof(struct tt__VideoRateControl));
    Configurations->RateControl->FrameRateLimit = H264_FRAMERATE;
    Configurations->RateControl->EncodingInterval = 1;
    Configurations->RateControl->BitrateLimit = 4096;
    //<Configurations><H264>
    Configurations->H264 = (struct tt__H264Configuration *)soap_malloc(soap, sizeof(struct tt__H264Configuration));
    memset(Configurations->H264, 0, sizeof(struct tt__H264Configuration));
    Configurations->H264->GovLength = H264_GOVLENGTH;
    Configurations->H264->H264Profile = H264_PROFILE;

    //<Configuration><Multicast>
    Configurations->Multicast = (struct tt__MulticastConfiguration *)soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
    memset(Configurations->Multicast, 0, sizeof(struct tt__MulticastConfiguration));
    Configurations->Multicast->Address = (struct tt__IPAddress *)soap_malloc(soap, sizeof(struct tt__IPAddress));
    memset(Configurations->Multicast->Address, 0, sizeof(struct tt__IPAddress));
    Configurations->Multicast->Address->Type = tt__IPType__IPv4;
    Configurations->Multicast->Address->IPv4Address = (char *)soap_malloc(soap, sizeof(char) * 32);
    memset(Configurations->Multicast->Address->IPv4Address, '\0', sizeof(char) * 32);
    strcpy(Configurations->Multicast->Address->IPv4Address, "224.1.0.0");
    Configurations->Multicast->Port = 5000;
    Configurations->Multicast->TTL = 64;
    Configurations->Multicast->AutoStart = xsd__boolean__true_;
    //<Configuration><SessionTimeout>
    Configurations->SessionTimeout = ONVIF_TIME_OUT;
    return 0;
}
/** Web service operation '__trt__GetVideoEncoderConfigurations' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfigurations(struct soap *soap, struct _trt__GetVideoEncoderConfigurations *trt__GetVideoEncoderConfigurations, struct _trt__GetVideoEncoderConfigurationsResponse *trt__GetVideoEncoderConfigurationsResponse)
{
    printf("call:%s\n", __func__);

    trt__GetVideoEncoderConfigurationsResponse->__sizeConfigurations = VIDEO_ENCODE_NUM;
    trt__GetVideoEncoderConfigurationsResponse->Configurations =
        (struct tt__VideoEncoderConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoEncoderConfiguration) * trt__GetVideoEncoderConfigurationsResponse->__sizeConfigurations);
    memset(trt__GetVideoEncoderConfigurationsResponse->Configurations, '\0', sizeof(struct tt__VideoEncoderConfiguration) * trt__GetVideoEncoderConfigurationsResponse->__sizeConfigurations);
    for (int i = 0; i < trt__GetVideoEncoderConfigurationsResponse->__sizeConfigurations; ++i)
    {
        get_videoencoder_config(soap, i, trt__GetVideoEncoderConfigurationsResponse->Configurations + i);
    }
    return 0;
}
/** Web service operation '__trt__GetAudioSourceConfigurations' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfigurations(struct soap *soap, struct _trt__GetAudioSourceConfigurations *trt__GetAudioSourceConfigurations, struct _trt__GetAudioSourceConfigurationsResponse *trt__GetAudioSourceConfigurationsResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetAudioEncoderConfigurations' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfigurations(struct soap *soap, struct _trt__GetAudioEncoderConfigurations *trt__GetAudioEncoderConfigurations, struct _trt__GetAudioEncoderConfigurationsResponse *trt__GetAudioEncoderConfigurationsResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetVideoAnalyticsConfigurations' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoAnalyticsConfigurations(struct soap *soap, struct _trt__GetVideoAnalyticsConfigurations *trt__GetVideoAnalyticsConfigurations, struct _trt__GetVideoAnalyticsConfigurationsResponse *trt__GetVideoAnalyticsConfigurationsResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetMetadataConfigurations' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfigurations(struct soap *soap, struct _trt__GetMetadataConfigurations *trt__GetMetadataConfigurations, struct _trt__GetMetadataConfigurationsResponse *trt__GetMetadataConfigurationsResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetAudioOutputConfigurations' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfigurations(struct soap *soap, struct _trt__GetAudioOutputConfigurations *trt__GetAudioOutputConfigurations, struct _trt__GetAudioOutputConfigurationsResponse *trt__GetAudioOutputConfigurationsResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetAudioDecoderConfigurations' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfigurations(struct soap *soap, struct _trt__GetAudioDecoderConfigurations *trt__GetAudioDecoderConfigurations, struct _trt__GetAudioDecoderConfigurationsResponse *trt__GetAudioDecoderConfigurationsResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetVideoSourceConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfiguration(struct soap *soap, struct _trt__GetVideoSourceConfiguration *trt__GetVideoSourceConfiguration, struct _trt__GetVideoSourceConfigurationResponse *trt__GetVideoSourceConfigurationResponse)
{
    printf("call:%s\n", __func__);
    get_videosource_config(soap, 0, trt__GetVideoSourceConfigurationResponse->Configuration);
    return 0;
}
/** Web service operation '__trt__GetVideoEncoderConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfiguration(struct soap *soap, struct _trt__GetVideoEncoderConfiguration *trt__GetVideoEncoderConfiguration, struct _trt__GetVideoEncoderConfigurationResponse *trt__GetVideoEncoderConfigurationResponse)
{
    printf("call:%s\n", __func__);
    get_videoencoder_config(soap, 0, trt__GetVideoEncoderConfigurationResponse->Configuration);
    return 0;
}
/** Web service operation '__trt__GetAudioSourceConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfiguration(struct soap *soap, struct _trt__GetAudioSourceConfiguration *trt__GetAudioSourceConfiguration, struct _trt__GetAudioSourceConfigurationResponse *trt__GetAudioSourceConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetAudioEncoderConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfiguration(struct soap *soap, struct _trt__GetAudioEncoderConfiguration *trt__GetAudioEncoderConfiguration, struct _trt__GetAudioEncoderConfigurationResponse *trt__GetAudioEncoderConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetVideoAnalyticsConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoAnalyticsConfiguration(struct soap *soap, struct _trt__GetVideoAnalyticsConfiguration *trt__GetVideoAnalyticsConfiguration, struct _trt__GetVideoAnalyticsConfigurationResponse *trt__GetVideoAnalyticsConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetMetadataConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfiguration(struct soap *soap, struct _trt__GetMetadataConfiguration *trt__GetMetadataConfiguration, struct _trt__GetMetadataConfigurationResponse *trt__GetMetadataConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetAudioOutputConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfiguration(struct soap *soap, struct _trt__GetAudioOutputConfiguration *trt__GetAudioOutputConfiguration, struct _trt__GetAudioOutputConfigurationResponse *trt__GetAudioOutputConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetAudioDecoderConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfiguration(struct soap *soap, struct _trt__GetAudioDecoderConfiguration *trt__GetAudioDecoderConfiguration, struct _trt__GetAudioDecoderConfigurationResponse *trt__GetAudioDecoderConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetCompatibleVideoEncoderConfigurations' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoEncoderConfigurations(struct soap *soap, struct _trt__GetCompatibleVideoEncoderConfigurations *trt__GetCompatibleVideoEncoderConfigurations, struct _trt__GetCompatibleVideoEncoderConfigurationsResponse *trt__GetCompatibleVideoEncoderConfigurationsResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetCompatibleVideoSourceConfigurations' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoSourceConfigurations(struct soap *soap, struct _trt__GetCompatibleVideoSourceConfigurations *trt__GetCompatibleVideoSourceConfigurations, struct _trt__GetCompatibleVideoSourceConfigurationsResponse *trt__GetCompatibleVideoSourceConfigurationsResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetCompatibleAudioEncoderConfigurations' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioEncoderConfigurations(struct soap *soap, struct _trt__GetCompatibleAudioEncoderConfigurations *trt__GetCompatibleAudioEncoderConfigurations, struct _trt__GetCompatibleAudioEncoderConfigurationsResponse *trt__GetCompatibleAudioEncoderConfigurationsResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetCompatibleAudioSourceConfigurations' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioSourceConfigurations(struct soap *soap, struct _trt__GetCompatibleAudioSourceConfigurations *trt__GetCompatibleAudioSourceConfigurations, struct _trt__GetCompatibleAudioSourceConfigurationsResponse *trt__GetCompatibleAudioSourceConfigurationsResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetCompatibleVideoAnalyticsConfigurations' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoAnalyticsConfigurations(struct soap *soap, struct _trt__GetCompatibleVideoAnalyticsConfigurations *trt__GetCompatibleVideoAnalyticsConfigurations, struct _trt__GetCompatibleVideoAnalyticsConfigurationsResponse *trt__GetCompatibleVideoAnalyticsConfigurationsResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetCompatibleMetadataConfigurations' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleMetadataConfigurations(struct soap *soap, struct _trt__GetCompatibleMetadataConfigurations *trt__GetCompatibleMetadataConfigurations, struct _trt__GetCompatibleMetadataConfigurationsResponse *trt__GetCompatibleMetadataConfigurationsResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetCompatibleAudioOutputConfigurations' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioOutputConfigurations(struct soap *soap, struct _trt__GetCompatibleAudioOutputConfigurations *trt__GetCompatibleAudioOutputConfigurations, struct _trt__GetCompatibleAudioOutputConfigurationsResponse *trt__GetCompatibleAudioOutputConfigurationsResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetCompatibleAudioDecoderConfigurations' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioDecoderConfigurations(struct soap *soap, struct _trt__GetCompatibleAudioDecoderConfigurations *trt__GetCompatibleAudioDecoderConfigurations, struct _trt__GetCompatibleAudioDecoderConfigurationsResponse *trt__GetCompatibleAudioDecoderConfigurationsResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__SetVideoSourceConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoSourceConfiguration(struct soap *soap, struct _trt__SetVideoSourceConfiguration *trt__SetVideoSourceConfiguration, struct _trt__SetVideoSourceConfigurationResponse *trt__SetVideoSourceConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__SetVideoEncoderConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoEncoderConfiguration(struct soap *soap, struct _trt__SetVideoEncoderConfiguration *trt__SetVideoEncoderConfiguration, struct _trt__SetVideoEncoderConfigurationResponse *trt__SetVideoEncoderConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__SetAudioSourceConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioSourceConfiguration(struct soap *soap, struct _trt__SetAudioSourceConfiguration *trt__SetAudioSourceConfiguration, struct _trt__SetAudioSourceConfigurationResponse *trt__SetAudioSourceConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__SetAudioEncoderConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioEncoderConfiguration(struct soap *soap, struct _trt__SetAudioEncoderConfiguration *trt__SetAudioEncoderConfiguration, struct _trt__SetAudioEncoderConfigurationResponse *trt__SetAudioEncoderConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__SetVideoAnalyticsConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoAnalyticsConfiguration(struct soap *soap, struct _trt__SetVideoAnalyticsConfiguration *trt__SetVideoAnalyticsConfiguration, struct _trt__SetVideoAnalyticsConfigurationResponse *trt__SetVideoAnalyticsConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__SetMetadataConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetMetadataConfiguration(struct soap *soap, struct _trt__SetMetadataConfiguration *trt__SetMetadataConfiguration, struct _trt__SetMetadataConfigurationResponse *trt__SetMetadataConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__SetAudioOutputConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioOutputConfiguration(struct soap *soap, struct _trt__SetAudioOutputConfiguration *trt__SetAudioOutputConfiguration, struct _trt__SetAudioOutputConfigurationResponse *trt__SetAudioOutputConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__SetAudioDecoderConfiguration' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioDecoderConfiguration(struct soap *soap, struct _trt__SetAudioDecoderConfiguration *trt__SetAudioDecoderConfiguration, struct _trt__SetAudioDecoderConfigurationResponse *trt__SetAudioDecoderConfigurationResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetVideoSourceConfigurationOptions' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfigurationOptions(struct soap *soap, struct _trt__GetVideoSourceConfigurationOptions *trt__GetVideoSourceConfigurationOptions, struct _trt__GetVideoSourceConfigurationOptionsResponse *trt__GetVideoSourceConfigurationOptionsResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
static int get_veconfig_options(struct soap *soap, const char *ve_token, const char *profile_token, struct tt__VideoEncoderConfigurationOptions *ve_options)
{

    ve_options->QualityRange = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
    ve_options->QualityRange->Min = 1;
    ve_options->QualityRange->Max = 10;

    ve_options->H264 = (struct tt__H264Options *)soap_malloc(soap, sizeof(struct tt__H264Options));

    ve_options->H264->__sizeResolutionsAvailable = 1;
    ve_options->H264->ResolutionsAvailable = (struct tt__VideoResolution *)soap_malloc(soap, sizeof(struct tt__VideoResolution) * ve_options->H264->__sizeResolutionsAvailable);

    ve_options->H264->ResolutionsAvailable->Width = ONVIF_FRAME_WIDTH;
    ve_options->H264->ResolutionsAvailable->Height = ONVIF_FRAME_HEIGHT;

    ve_options->H264->GovLengthRange = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
    ve_options->H264->GovLengthRange->Min = 1;
    ve_options->H264->GovLengthRange->Max = H264_GOVLENGTH;

    ve_options->H264->FrameRateRange = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
    ve_options->H264->FrameRateRange->Min = 1;
    ve_options->H264->FrameRateRange->Max = H264_FRAMERATE;

    ve_options->H264->EncodingIntervalRange = (struct tt__IntRange *)soap_malloc(soap, sizeof(struct tt__IntRange));
    ve_options->H264->EncodingIntervalRange->Min = 1;
    ve_options->H264->EncodingIntervalRange->Max = 10;

    ve_options->H264->__sizeH264ProfilesSupported = 1;
    ve_options->H264->H264ProfilesSupported = (enum tt__H264Profile *)soap_malloc(soap, sizeof(enum tt__H264Profile) * ve_options->H264->__sizeH264ProfilesSupported);

    *ve_options->H264->H264ProfilesSupported = H264_PROFILE;
    return 0;
}
/** Web service operation '__trt__GetVideoEncoderConfigurationOptions' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfigurationOptions(struct soap *soap, struct _trt__GetVideoEncoderConfigurationOptions *trt__GetVideoEncoderConfigurationOptions, struct _trt__GetVideoEncoderConfigurationOptionsResponse *trt__GetVideoEncoderConfigurationOptionsResponse)
{
    printf("call:%s\n", __func__);
    trt__GetVideoEncoderConfigurationOptionsResponse->Options = (struct tt__VideoEncoderConfigurationOptions *)soap_malloc(soap, sizeof(struct tt__VideoEncoderConfigurationOptions));
    memset(trt__GetVideoEncoderConfigurationOptionsResponse->Options, '\0', sizeof(struct tt__VideoEncoderConfigurationOptions));
    get_veconfig_options(soap, trt__GetVideoEncoderConfigurationOptions->ConfigurationToken,
                         trt__GetVideoEncoderConfigurationOptions->ProfileToken,
                         trt__GetVideoEncoderConfigurationOptionsResponse->Options);
    return 0;
}
/** Web service operation '__trt__GetAudioSourceConfigurationOptions' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfigurationOptions(struct soap *soap, struct _trt__GetAudioSourceConfigurationOptions *trt__GetAudioSourceConfigurationOptions, struct _trt__GetAudioSourceConfigurationOptionsResponse *trt__GetAudioSourceConfigurationOptionsResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetAudioEncoderConfigurationOptions' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfigurationOptions(struct soap *soap, struct _trt__GetAudioEncoderConfigurationOptions *trt__GetAudioEncoderConfigurationOptions, struct _trt__GetAudioEncoderConfigurationOptionsResponse *trt__GetAudioEncoderConfigurationOptionsResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetMetadataConfigurationOptions' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfigurationOptions(struct soap *soap, struct _trt__GetMetadataConfigurationOptions *trt__GetMetadataConfigurationOptions, struct _trt__GetMetadataConfigurationOptionsResponse *trt__GetMetadataConfigurationOptionsResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetAudioOutputConfigurationOptions' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfigurationOptions(struct soap *soap, struct _trt__GetAudioOutputConfigurationOptions *trt__GetAudioOutputConfigurationOptions, struct _trt__GetAudioOutputConfigurationOptionsResponse *trt__GetAudioOutputConfigurationOptionsResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetAudioDecoderConfigurationOptions' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfigurationOptions(struct soap *soap, struct _trt__GetAudioDecoderConfigurationOptions *trt__GetAudioDecoderConfigurationOptions, struct _trt__GetAudioDecoderConfigurationOptionsResponse *trt__GetAudioDecoderConfigurationOptionsResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetGuaranteedNumberOfVideoEncoderInstances' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetGuaranteedNumberOfVideoEncoderInstances(struct soap *soap, struct _trt__GetGuaranteedNumberOfVideoEncoderInstances *trt__GetGuaranteedNumberOfVideoEncoderInstances, struct _trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse *trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetStreamUri' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetStreamUri(struct soap *soap, struct _trt__GetStreamUri *trt__GetStreamUri, struct _trt__GetStreamUriResponse *trt__GetStreamUriResponse)
{
    printf("call:%s\n", __func__);
    trt__GetStreamUriResponse->MediaUri = (struct tt__MediaUri *)soap_malloc(soap, sizeof(struct tt__MediaUri));
    memset(trt__GetStreamUriResponse->MediaUri, 0, sizeof(struct tt__MediaUri));

    trt__GetStreamUriResponse->MediaUri->Uri = (char *)soap_malloc(soap, sizeof(char) * 100);
    memset(trt__GetStreamUriResponse->MediaUri->Uri, '\0', sizeof(char) * 100);
    //根据各个设备的rtsp协议的uri不同填写对应的值

    char ip[18] = {0};
    getNetworkIp(ETH, ip, sizeof(ip));
    sprintf(trt__GetStreamUriResponse->MediaUri->Uri, "rtsp://%s:%d/h264Live", ip, RTSP_PORT);

    trt__GetStreamUriResponse->MediaUri->InvalidAfterConnect = xsd__boolean__true_;
    trt__GetStreamUriResponse->MediaUri->InvalidAfterReboot = xsd__boolean__true_;
    //超时时间
    trt__GetStreamUriResponse->MediaUri->Timeout = 0;

    return 0;
}
/** Web service operation '__trt__StartMulticastStreaming' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__StartMulticastStreaming(struct soap *soap, struct _trt__StartMulticastStreaming *trt__StartMulticastStreaming, struct _trt__StartMulticastStreamingResponse *trt__StartMulticastStreamingResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__StopMulticastStreaming' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__StopMulticastStreaming(struct soap *soap, struct _trt__StopMulticastStreaming *trt__StopMulticastStreaming, struct _trt__StopMulticastStreamingResponse *trt__StopMulticastStreamingResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__SetSynchronizationPoint' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetSynchronizationPoint(struct soap *soap, struct _trt__SetSynchronizationPoint *trt__SetSynchronizationPoint, struct _trt__SetSynchronizationPointResponse *trt__SetSynchronizationPointResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetSnapshotUri' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetSnapshotUri(struct soap *soap, struct _trt__GetSnapshotUri *trt__GetSnapshotUri, struct _trt__GetSnapshotUriResponse *trt__GetSnapshotUriResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetVideoSourceModes' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceModes(struct soap *soap, struct _trt__GetVideoSourceModes *trt__GetVideoSourceModes, struct _trt__GetVideoSourceModesResponse *trt__GetVideoSourceModesResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__SetVideoSourceMode' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoSourceMode(struct soap *soap, struct _trt__SetVideoSourceMode *trt__SetVideoSourceMode, struct _trt__SetVideoSourceModeResponse *trt__SetVideoSourceModeResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetOSDs' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetOSDs(struct soap *soap, struct _trt__GetOSDs *trt__GetOSDs, struct _trt__GetOSDsResponse *trt__GetOSDsResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetOSD' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetOSD(struct soap *soap, struct _trt__GetOSD *trt__GetOSD, struct _trt__GetOSDResponse *trt__GetOSDResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__GetOSDOptions' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetOSDOptions(struct soap *soap, struct _trt__GetOSDOptions *trt__GetOSDOptions, struct _trt__GetOSDOptionsResponse *trt__GetOSDOptionsResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__SetOSD' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetOSD(struct soap *soap, struct _trt__SetOSD *trt__SetOSD, struct _trt__SetOSDResponse *trt__SetOSDResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__CreateOSD' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__CreateOSD(struct soap *soap, struct _trt__CreateOSD *trt__CreateOSD, struct _trt__CreateOSDResponse *trt__CreateOSDResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}
/** Web service operation '__trt__DeleteOSD' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __trt__DeleteOSD(struct soap *soap, struct _trt__DeleteOSD *trt__DeleteOSD, struct _trt__DeleteOSDResponse *trt__DeleteOSDResponse)
{
    printf("call:%s\n", __func__);
    return 0;
}