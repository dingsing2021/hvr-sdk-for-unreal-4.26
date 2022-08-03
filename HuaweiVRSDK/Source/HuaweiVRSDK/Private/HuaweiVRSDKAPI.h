#pragma once

#if PLATFORM_ANDROID

#include "Android/AndroidJNI.h"
#include "Android/AndroidApplication.h"
#endif

/*
 * This enumeration is consistent with the definition in unity
 */
enum EventNotify {
    NotifyEvent_SetVsync = 0x0,
    NotifyEvent_GlassState = 0x1,
    NotifyEvent_ChargeState = 0x2,
    NotifyEvent_NetWorkState = 0x4,
    NotifyEvent_VolumeInfo = 0x8,
    NotifyEvent_BatteryInfo = 0x10,
    NotifyEvent_WifiInfo = 0x20,
    NotifyEvent_CellularInfo = 0x40,
    NotifyEvent_ThermalWarning = 0x80,
    NotifyEvent_NewSMS = 0x100,
    NotifyEvent_LowPowerInfo = 0x200,
    NotifyEvent_PhoneLowPower = 0x400,
    NotifyEvent_HelmetLowPower = 0x800,
    NotifyEvent_CaptureImageSucceed = 0x1000,
    NotifyEvent_NoControllerJoin = 0x2000,
    NotifyEvent_ControllerDisconnect = 0x4000,
    NotifyEvent_GeneralMessage = 0x8000,
    NotifyEvent_ControllerLowPower = 0x10000,
    NotifyEvent_CaptureImageFailed = 0x20000,
    NotifyEvent_BluetoothDisable = 0x10000000,
    NotifyEvent_CommonNotification = 0x20000000,
    NotifyEvent_OtherNotification = 0x40000000
};

/*
 * This enumeration is consistent with the definition in unity
 */
enum MsgIdx {
    MESSAGE_GLASS_BASE = 0x1000,
    GLASS_HEART_BEAT_MESSAGE = (MESSAGE_GLASS_BASE + 6),
    MESSAGE_NOTIFICATION_BASE = (MESSAGE_GLASS_BASE + 0x20),
    NOTIFICATION_GENERAL_MESSAGE = (MESSAGE_NOTIFICATION_BASE + 1),
    NOTIFICATION_SMS_MESSAGE = (MESSAGE_NOTIFICATION_BASE + 2),
    NOTIFICATION_LOWPOER_MESSAGE = (MESSAGE_NOTIFICATION_BASE + 3),
    NOTIFICATION_NETWORKCHANGE_MESSAGE = (MESSAGE_NOTIFICATION_BASE + 4),
    NOTIFICATION_CHARGESTATE_MESSAGE = (MESSAGE_NOTIFICATION_BASE + 5),
    CAMERA_NO_PERMISSION_MESSAGE = 0xabcd
};

typedef enum HelmetModel {
    HelmetFirstGen,
    HelmetSecondGen,
    HelmetThirdGen,
    HelmetNotFound,
    HelmetVRProto,
    HelmetARProto,
    HelmetUnknown
} HelmetModel;

static const char* UNREAL_PLUGIN_FLAG = "HuaweiVRSDK_UnrealPlugin";

#if PLATFORM_ANDROID

#define LOG_TAG "HuaweiVRSDK"

#define LOGI(fmt, ...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##__VA_ARGS__)
#define LOGD(fmt, ...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ##__VA_ARGS__)

#define HUAWEI_EXPORT

typedef int (*HuaweiVRSDKNativeMessageReceiverFunc)(const char* key, const char* value);

#ifdef __cplusplus
extern "C" {
#endif

HUAWEI_EXPORT void HVR_GetSensorState(bool monoscopic, float& w, float& x, float& y, float& z, float& fov,
    int& viewNumber);

HUAWEI_EXPORT bool HVR_GetCameraPositionOrientation(float& px, float& py, float& pz, float& ox, float& oy, float& oz,
    float& ow);

HUAWEI_EXPORT void HVR_SetLogEnabled(bool bEnable);
HUAWEI_EXPORT void HVR_SetEyeParms(int width, int height);

HUAWEI_EXPORT void HVR_InitRenderThread();
HUAWEI_EXPORT void HVR_InitRenderFoveatedThread();
HUAWEI_EXPORT void HVR_ShutdownRenderThread();
HUAWEI_EXPORT void HVR_Pause();
HUAWEI_EXPORT void HVR_Resume();

HUAWEI_EXPORT int HVR_GetVolume();

HUAWEI_EXPORT int HVR_ResetSensorOrientation();

HUAWEI_EXPORT int HVR_ResetSensorYaw();

HUAWEI_EXPORT bool HVR_GetEyeDist(float& fEyeDist);
HUAWEI_EXPORT bool HVR_GetFOVAngle(float& fFOV);

HUAWEI_EXPORT void HVR_TimeWarpEvent(const int viewIndex);

HUAWEI_EXPORT void HVR_CameraEndFrame(int eye, int textureId, int layer);

HUAWEI_EXPORT void HVR_CameraEndFrameFoveated(int eye, int textureId, int layer);

HUAWEI_EXPORT void HVR_SetInitVariables(jobject activity);

HUAWEI_EXPORT int HVR_SensorGetContainer();

HUAWEI_EXPORT bool HVR_GetMsgIdx(int& msgIdx);

HUAWEI_EXPORT int HVR_Android_GetNotifyEvent();

HUAWEI_EXPORT int HVR_Android_GetThermalWarningInfo(int& level, int& frameRate, int& ratioWidth, int& ratioHeight);

HUAWEI_EXPORT int HVR_Android_GetLowpowerFlag(int device, int& flag);

HUAWEI_EXPORT char* HVR_GetNotifiPackage();

HUAWEI_EXPORT char* HVR_GetNotifiTitle();

HUAWEI_EXPORT char* HVR_GetNotifiContent();

HUAWEI_EXPORT char* HVR_GetSmsNumber();

HUAWEI_EXPORT char* HVR_GetSmsContactName();

HUAWEI_EXPORT char* HVR_GetSmsMsg();

HUAWEI_EXPORT bool HVR_GetWifiMsg(bool& state);

HUAWEI_EXPORT int HVR_SendMessage(const char* cmd, const char* value);

HUAWEI_EXPORT void* HVR_Android_GetHelmet();

HUAWEI_EXPORT int HVR_Android_GetHelmetInfo(void* handle, HelmetModel* model);

HUAWEI_EXPORT int HVR_Android_SetPoseLock(void* handle, bool enable);

HUAWEI_EXPORT void HVR_Android_EnableChromaticAberration(bool enable);

HUAWEI_EXPORT int HVR_Android_EnableSvsEffect(bool enable);

HUAWEI_EXPORT void HVR_RegistReceiveMessageCallback(HuaweiVRSDKNativeMessageReceiverFunc callback);

HUAWEI_EXPORT int HVR_SendMessage(const char* cmd, const char* value);
#ifdef __cplusplus
}
#endif

#else
#define LOGD(fmt, ...) UE_LOG(LogHuaweiVRSDK, Log, TEXT(fmt), ##__VA_ARGS__)
#define LOGI(fmt, ...) UE_LOG(LogHuaweiVRSDK, Log, TEXT(fmt), ##__VA_ARGS__)
#define LOGW(fmt, ...) UE_LOG(LogHuaweiVRSDK, Log, TEXT(fmt), ##__VA_ARGS__)
#define LOGE(fmt, ...) UE_LOG(LogHuaweiVRSDK, Log, TEXT(fmt), ##__VA_ARGS__)
#endif