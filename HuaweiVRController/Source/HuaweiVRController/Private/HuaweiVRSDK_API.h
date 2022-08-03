#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidJNI.h"

#define LOG_TAG "HuaweiVRSDK"

#define LOGI(fmt, ...) __android_log_print(ANDROID_LOG_INFO,    LOG_TAG, fmt, ##__VA_ARGS__)
#define LOGD(fmt, ...) __android_log_print(ANDROID_LOG_DEBUG,   LOG_TAG, fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) __android_log_print(ANDROID_LOG_WARN,    LOG_TAG, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) __android_log_print(ANDROID_LOG_ERROR,   LOG_TAG, fmt, ##__VA_ARGS__)

#else
#define LOGD(fmt, ...) UE_LOG(LogHuaweiVRSDK, Log, TEXT(fmt), ##__VA_ARGS__)
#define LOGI(fmt, ...) UE_LOG(LogHuaweiVRSDK, Log, TEXT(fmt), ##__VA_ARGS__)
#define LOGW(fmt, ...) UE_LOG(LogHuaweiVRSDK, Log, TEXT(fmt), ##__VA_ARGS__)
#define LOGE(fmt, ...) UE_LOG(LogHuaweiVRSDK, Log, TEXT(fmt), ##__VA_ARGS__)
#endif

static const char* UNREAL_PLUGIN_FLAG = "HuaweiVRSDK_UnrealPlugin";

typedef enum {
    HVR_CONTROLLER_DISCONNECTED,
    HVR_CONTROLLER_SCANNING,
    HVR_CONTROLLER_CONNECTING,
    HVR_CONTROLLER_CONNECTED,
    HVR_CONTROLLER_ERROR
} HVR_CONTROLLER_STATUS;

typedef enum {
    HVR_CONTROLLER_3DOF,
    HVR_CONTROLLER_6DOF,
    HVR_CONTROLLER_Gaming,
    HVR_CONTROLLER_SysStd,
    HVR_CONTROLLER_Other
} HVR_CONTROLLER_TYPE;

typedef enum {
    ButtonHome,
    ButtonBack,
    ButtonVolumeInc,
    ButtonVolumeDec,
    ButtonConfirm,
    ButtonTrigger,
    ButtonTouchPad,
	ButtonAX6DoF,
	ButtonBY6DoF,
	ButtonHomeMenu6DoF,
	ButtonThumbstick6DoF,
	ButtonTrigger6DoF,
	ButtonGrip6DoF
} ButtonType;

typedef enum {
	HVRControllerStandardWaveform
} HVR_Controller_WaveformType;

typedef struct {
    float x,y;
} VrVector2f;

typedef struct {
    float x,y,z;
} VrVector3f;

typedef struct {
    float x,y,z,w;
} VrQuaternionf;

typedef struct {
    VrVector3f position;//空间位置
    VrQuaternionf rotation; //空间旋转姿态
} VrPosturef;

typedef void (*HvrEventCallback)(int eventId, void* data);
typedef void (*ControllerStatusCallBack)(int index, HVR_CONTROLLER_STATUS status);

#define HUAWEI_EXPORT 

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __ANDROID__
	HUAWEI_EXPORT void* HVR_Android_GetControllerHandle();
	HUAWEI_EXPORT void* HVR_Android_GetHelmet();
	HUAWEI_EXPORT HVR_CONTROLLER_STATUS HVR_Android_GetControllerStatus(void* handle);

	HUAWEI_EXPORT bool HVR_Android_IsControllerAvailable(void* handle) ;
	HUAWEI_EXPORT HVR_CONTROLLER_TYPE HVR_Android_GetControllerType(void* handle);
	HUAWEI_EXPORT int HVR_Android_ResetControllerCenter(void* handle);
	HUAWEI_EXPORT int HVR_Android_GetControllerPosture(void* handle, VrPosturef& pose);
	HUAWEI_EXPORT int HVR_Android_GetControllerTriggerData(void* handle, float& data);
	HUAWEI_EXPORT int HVR_Android_GetControllerGripData(void* handle, float& data);
	HUAWEI_EXPORT int HVR_Android_GetControllerTouchpadTouchPos(void* handle, VrVector2f& pos);
	HUAWEI_EXPORT int HVR_Android_GetControllerThumbstickPos(void* handle, VrVector2f& pos);
	HUAWEI_EXPORT int HVR_Android_GetControllerGyroscope(void* handle, VrVector3f& gyroscope);
    HUAWEI_EXPORT int HVR_Android_GetControllerAccelerometer(void* handle, VrVector3f& accelerometer);
    HUAWEI_EXPORT bool HVR_Android_IsControllerTouchpadTouching(void* handle);
    HUAWEI_EXPORT bool HVR_Android_IsControllerTouchpadTouchDown(void* handle);
    HUAWEI_EXPORT bool HVR_Android_IsControllerTouchpadTouchUp(void* handle);
    HUAWEI_EXPORT bool HVR_Android_IsControllerButtonNearTouch(void* handle, ButtonType button);
	HUAWEI_EXPORT bool HVR_Android_IsControllerButtonTouching(void* handle, ButtonType button);
	HUAWEI_EXPORT bool HVR_Android_IsControllerButtonLeaveTouch(void* handle, ButtonType button);
	HUAWEI_EXPORT bool HVR_Android_IsControllerButtonPressed(void* handle, ButtonType button);
    HUAWEI_EXPORT bool HVR_Android_IsControllerButtonDown(void* handle, ButtonType button);
    HUAWEI_EXPORT bool HVR_Android_IsControllerButtonUp(void* handle, ButtonType button);
    HUAWEI_EXPORT void HVR_Android_UpdateControllerData(void* handle);
    HUAWEI_EXPORT int HVR_Android_GetValidControllerIndices(void* handle, int* indices, int len);
    HUAWEI_EXPORT void* HVR_Android_GetControllerByIndex(void* handle, int index);
    HUAWEI_EXPORT void HVR_Android_RegistControllerStatusCallBack(void* handle, ControllerStatusCallBack callback);
    HUAWEI_EXPORT void HVR_Android_RegisterControllerCallBack(void* handle, HvrEventCallback callback);
	HUAWEI_EXPORT void HVR_SetInitVariables(jobject activity);
	HUAWEI_EXPORT int HVR_Android_CaptureEyeImage(char* path , int Length);
	HUAWEI_EXPORT void HVR_Android_JumpToLaucher();
	HUAWEI_EXPORT void HVR_Android_JumpToSetting();
	HUAWEI_EXPORT int HVR_Android_ResetYaw(void * handle);
	HUAWEI_EXPORT bool HVR_IsBluetoothEnable();
	HUAWEI_EXPORT void HVR_Android_AdjustSystemVolume(int flag);
    HUAWEI_EXPORT int HVR_SendMessage(const char *cmd, const char *value);
	HUAWEI_EXPORT int HVR_Android_StartControllerVibration(void* handle,float amplitude, int durationTimeMs,float frequency, HVR_Controller_WaveformType waveform);
	HUAWEI_EXPORT int HVR_Android_StopControllerVibration(void* handle);
	//HUAWEI_EXPORT float HVR_GetTimeWarpFPS();
#endif
#ifdef __cplusplus
}
#endif

