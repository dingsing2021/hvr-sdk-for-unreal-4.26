// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "HuaweiVRControllerPrivatePCH.h"
#include "IInputDevice.h"
#include "XRMotionControllerBase.h"
#include "HuaweiVRSDK_API.h"
#include <string>
using namespace std;

#define CONTROLLERS_PLAYER	2
DEFINE_LOG_CATEGORY_STATIC(LogHuaweiVRSDK, Log, All);

enum MessageType {
	NotifyEvent_CaptureImageSucceed = 0x0,
	NotifyEvent_NoControllerJoin = 0x1,
	NotifyEvent_ControllerDisconnect = 0x2,
	NotifyEvent_CaptureImageFailed = 0x4,
	NotifyEvent_BluetoothDisable = 0x8,
	NotifyEvent_ControllerConnected = 0x10
};

static const FString  m_text_NoAvailableControllerTitle_zh = TEXT("手柄连接异常");
static const FString  m_text_NoAvailableControllerTitle_en = TEXT("Controller connection error");
static const FString  m_text_NoAvailableControllerMsg_zh = TEXT("请断开头盔与手机的连接，在手机上的手柄配置应用中确保手柄连接正常");
static const FString  m_text_NoAvailableControllerMsg_en = TEXT("Please disconnect the phone from the headset and check the controller connection status in the controller app.");
static const FString  m_text_ControllerConnect_zh = TEXT("手柄已连接");
static const FString  m_text_ControllerConnect_en = TEXT("Controller connected");
static const FString  m_text_ControllerDisconnect_zh = TEXT("手柄已断开");
static const FString  m_text_ControllerDisconnect_en = TEXT("Controller disconnected");
static const FString  m_text_BluetoothDisableTitle_zh = TEXT("蓝牙未开启");
static const FString  m_text_BluetoothDisableTitle_en = TEXT("Bluetooth is not enabled");
static const FString  m_text_BluetoothDisableMsg_zh = TEXT("请断开头盔与手机的连接，并检查手机蓝牙是否已经开启");
static const FString  m_text_BluetoothDisableMsg_en = TEXT("Please disconnect the phone from the headset and check whether your phone's Bluetooth is enabled.");
static const FString  m_text_CaptureImageSucceed_zh = TEXT("截图成功");
static const FString  m_text_CaptureImageSucceed_en = TEXT("Screenshot succeed");
static const FString  m_text_CaptureImageFailedTitle_zh = TEXT("截图失败");
static const FString  m_text_CaptureImageFailedMsg_zh = TEXT("请检查当前应用存储权限是否开启");
static const FString  m_text_CaptureImageFailedTitle_en = TEXT("Screenshot failed");
static const FString  m_text_CaptureImageFailedMsg_en = TEXT("Please check whether the current application storage permission is turned on");
static const FString HuaweiVRMessageKey_Title = "title";
static const FString HuaweiVRMessageKey_Content = "content";
//UCLASS()
class FHuaweiVRController : public IInputDevice, public FXRMotionControllerBase
{
public:
	FHuaweiVRController(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler);
	virtual ~FHuaweiVRController();
public:
	struct EHVRControllerButton
	{
		enum Type
		{
			Home,
			Back,
			Confirm,
			Trigger,
			TouchPadLeft,
			TouchPadRight,
			TouchPadUp,
			TouchPadDown,
			TouchPadTouch,
			TouchPadTouching,
			AX6DoF,
			BY6DoF,
			HomeMenu6DoF,
			Thumbstick6DoF,
			Trigger6DoF,
			Grip6DoF,
			/** Max number of controller buttons.  Must be < 256 */
			TotalButtonCount
		};
	};

private:
	double getCurrentTime();
	string getCurrentTimeStr();
	void ProcessControllerButtonEvents();
	void UpdateControllerData(float DeltaTime);
	FVector ConvertHvrVectorToUnreal(float x, float y, float z, float WorldToMetersScale) const;
	FQuat ConvertHvrQuaternionToUnreal(float w, float x, float y, float z) const;
	int GetLanguage();

public:
	bool IsAvailable() const;
	bool IsAvailable(const EControllerHand DeviceHand) const;
	float GetWorldToMetersScale() const;
	bool DetectHVRControllerMessage(int& type, int& priority, TMap<FString, FString>& message);
	int StartHVRControllerVibration(const EControllerHand DeviceHand, float amplitude, uint16_t durationTimeMs, float frequency, HVR_Controller_WaveformType waveform = HVRControllerStandardWaveform);
	int StopHVRControllerVibration(const EControllerHand DeviceHand);
	HVR_CONTROLLER_STATUS GetHVRControllerStatus(const EControllerHand DeviceHand);
	FVector2D GetHVRControllerTouchpadTouchPos(const EControllerHand DeviceHand);
	FVector2D GetHVRControllerThumbstickPos(const EControllerHand DeviceHand);
	HVR_CONTROLLER_TYPE GetHVRControllerType(const EControllerHand DeviceHand);


public:	// IInputDevice

		/** Tick the interface (e.g. check for new controllers) */
	virtual void Tick(float DeltaTime);

	/** Poll for controller state and send events if needed */
	virtual void SendControllerEvents();

	/** Set which MessageHandler will get the events from SendControllerEvents. */
	virtual void SetMessageHandler(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler);

	/** Exec handler to allow console commands to be passed through for debugging */
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar);

	/**
	* IForceFeedbackSystem pass through functions
	*/
	virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value);
	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues &values);

public: // IMotionController

	static FName DeviceTypeName;
	virtual FName GetMotionControllerDeviceTypeName() const override
	{
		return DeviceTypeName;
	}
	/**
	* Returns the calibration-space orientation of the requested controller's hand.
	*
	* @param ControllerIndex	The Unreal controller (player) index of the contoller set
	* @param DeviceHand		Which hand, within the controller set for the player, to get the orientation and position for
	* @param OutOrientation	(out) If tracked, the orientation (in calibrated-space) of the controller in the specified hand
	* @param OutPosition		(out) If tracked, the position (in calibrated-space) of the controller in the specified hand
	* @return					True if the device requested is valid and tracked, false otherwise
	*/
	virtual bool GetControllerOrientationAndPosition(const int32 ControllerIndex, const EControllerHand DeviceHand, FRotator& OutOrientation, FVector& OutPosition, float WorldToMetersScale) const;

	/**
	* Returns the tracking status (e.g. not tracked, intertial-only, fully tracked) of the specified controller
	*
	* @return	Tracking status of the specified controller, or ETrackingStatus::NotTracked if the device is not found
	*/
	virtual ETrackingStatus GetControllerTrackingStatus(const int32 ControllerIndex, const EControllerHand DeviceHand) const;

private:

	struct timespec {
		time_t tv_sec;
		long tv_usec;
	};
	FGamepadKeyNames::Type Buttons[CONTROLLERS_PLAYER][EHVRControllerButton::TotalButtonCount];
	void *mControllerHandle;
	TSharedRef<FGenericApplicationMessageHandler> MessageHandler;
	VrVector2f mTouchDownPos;
	VrVector2f mTouchUpPos;
	bool Left_LastButtonStates[EHVRControllerButton::TotalButtonCount] = { 0 };
	bool Right_LastButtonStates[EHVRControllerButton::TotalButtonCount] = { 0 };
	bool mNotifyOnce;
	bool mIsRightController;
	bool mIsHomePressed;
	bool mIsHomeUp;
	bool mIsBackPressed;
	bool mIsTriggerPressed;
	bool mIsGripPressed;
	bool mIsCapture;
	bool mIsResetOrientation;
	bool mIsJumpToSetting;
	double mHomeDownTime;
	double mBackDownTime;
	double mTriggerDownTime;
	double mVolumeIncDownTime;
	double mVolumeDecDownTime;
	double mLastVolumeIncTime;
	double mLastVolumeDecTime;

	int mFrameNumber;
public:
	void *mController;
	void *mLeftController;
	void *mRightController;
	int mLanguage;
	FString mMessageTitle;
	TMap<FString, FString> mMessage;
	int mMessageType;
	bool mIsDetectMessage;
};