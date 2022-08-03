// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "HuaweiVRController.h"
#include <math.h>
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "ImageUtils.h"
#include "HAL/PlatformFileManager.h"
#include "HuaweiVRSDK_API.h"

#if HVRCONTROLLER_SUPPORTED_ANDROID_PLATFORMS
#include <sys/time.h>
#include "Android/AndroidJNI.h"
#endif
#include <stdio.h>

#define SKIP_FRAME          10
#define LOCTEXT_NAMESPACE "HuaweiVRInput"

struct FHuaweiVRKey
{
	static const FKey HuaweiVR_Touch_Left_Thumbstick;
	static const FKey HuaweiVR_Touch_Left_Trigger;
	static const FKey HuaweiVR_Touch_Left_FaceButton1; // X or A
	static const FKey HuaweiVR_Touch_Left_Shoulder; // Y or B

	static const FKey HuaweiVR_Touch_Right_Thumbstick;
	static const FKey HuaweiVR_Touch_Right_Trigger;
	static const FKey HuaweiVR_Touch_Right_FaceButton1; // X or A
	static const FKey HuaweiVR_Touch_Right_Shoulder; // Y or B
};

struct FHuaweiVRKeyNames
{
	typedef FName Type;

	static const FName HuaweiVR_Touch_Left_Thumbstick;
	static const FName HuaweiVR_Touch_Left_Trigger;
	static const FName HuaweiVR_Touch_Left_FaceButton1; // X or A
	static const FName HuaweiVR_Touch_Left_Shoulder; // Y or B

	static const FName HuaweiVR_Touch_Right_Thumbstick;
	static const FName HuaweiVR_Touch_Right_Trigger;
	static const FName HuaweiVR_Touch_Right_FaceButton1; // X or A
	static const FName HuaweiVR_Touch_Right_Shoulder; // Y or B

};

const FKey FHuaweiVRKey::HuaweiVR_Touch_Left_Thumbstick("HuaweiVR_Touch_Left_Thumbstick");
const FKey FHuaweiVRKey::HuaweiVR_Touch_Left_Trigger("HuaweiVR_Touch_Left_Trigger");
const FKey FHuaweiVRKey::HuaweiVR_Touch_Left_FaceButton1("HuaweiVR_Touch_Left_FaceButton1");
const FKey FHuaweiVRKey::HuaweiVR_Touch_Left_Shoulder("HuaweiVR_Touch_Left_Shoulder");

const FKey FHuaweiVRKey::HuaweiVR_Touch_Right_Thumbstick("HuaweiVR_Touch_Right_Thumbstick");
const FKey FHuaweiVRKey::HuaweiVR_Touch_Right_Trigger("HuaweiVR_Touch_Right_Trigger");
const FKey FHuaweiVRKey::HuaweiVR_Touch_Right_FaceButton1("HuaweiVR_Touch_Right_FaceButton1");
const FKey FHuaweiVRKey::HuaweiVR_Touch_Right_Shoulder("HuaweiVR_Touch_Right_Shoulder");

const FHuaweiVRKeyNames::Type FHuaweiVRKeyNames::HuaweiVR_Touch_Left_Thumbstick("HuaweiVR_Touch_Left_Thumbstick");
const FHuaweiVRKeyNames::Type FHuaweiVRKeyNames::HuaweiVR_Touch_Left_Trigger("HuaweiVR_Touch_Left_Trigger");
const FHuaweiVRKeyNames::Type FHuaweiVRKeyNames::HuaweiVR_Touch_Left_FaceButton1("HuaweiVR_Touch_Left_FaceButton1");
const FHuaweiVRKeyNames::Type FHuaweiVRKeyNames::HuaweiVR_Touch_Left_Shoulder("HuaweiVR_Touch_Left_Shoulder");

const FHuaweiVRKeyNames::Type FHuaweiVRKeyNames::HuaweiVR_Touch_Right_Thumbstick("HuaweiVR_Touch_Right_Thumbstick");
const FHuaweiVRKeyNames::Type FHuaweiVRKeyNames::HuaweiVR_Touch_Right_Trigger("HuaweiVR_Touch_Right_Trigger");
const FHuaweiVRKeyNames::Type FHuaweiVRKeyNames::HuaweiVR_Touch_Right_FaceButton1("HuaweiVR_Touch_Right_FaceButton1");
const FHuaweiVRKeyNames::Type FHuaweiVRKeyNames::HuaweiVR_Touch_Right_Shoulder("HuaweiVR_Touch_Right_Shoulder");

static FHuaweiVRController*  gHvrController;
void controllerStatusCallBack(int index, HVR_CONTROLLER_STATUS status)
{
#if HVRCONTROLLER_SUPPORTED_ANDROID_PLATFORMS
	if (index < 0 || NULL == gHvrController)
	{
		return;
	}
	if (HVR_CONTROLLER_CONNECTED == status)
	{
		UE_LOG(LogHuaweiVRSDK, Log, TEXT(" Controller is connected "));
		if (NULL != gHvrController && (NULL != gHvrController->mController || NULL != gHvrController->mLeftController || NULL != gHvrController->mRightController))
		{
			gHvrController->mMessage.Empty();
			gHvrController->mIsDetectMessage = true;
			gHvrController->mMessageType = (int)NotifyEvent_ControllerConnected;
			if (1 == gHvrController->mLanguage) {
				gHvrController->mMessage.Add(HuaweiVRMessageKey_Title, m_text_ControllerConnect_zh);
			}
			else if (0 == gHvrController->mLanguage) {
				gHvrController->mMessage.Add(HuaweiVRMessageKey_Title, m_text_ControllerConnect_en);
			}
		}
	}
	if (HVR_CONTROLLER_DISCONNECTED == status)
	{
		UE_LOG(LogHuaweiVRSDK, Log, TEXT(" Controller is disconnected "));
		if (NULL != gHvrController && (NULL != gHvrController->mController || NULL != gHvrController->mLeftController || NULL != gHvrController->mRightController))
		{
			gHvrController->mMessage.Empty();
			gHvrController->mIsDetectMessage = true;
			gHvrController->mMessageType = (int)NotifyEvent_ControllerDisconnect;
			if (1 == gHvrController->mLanguage) {
				gHvrController->mMessage.Add(HuaweiVRMessageKey_Title, m_text_ControllerDisconnect_zh);
			}
			else if (0 == gHvrController->mLanguage) {
				gHvrController->mMessage.Add(HuaweiVRMessageKey_Title, m_text_ControllerDisconnect_en);
			}

		}
	}
#endif
}

class FHuaweiVRControllerPlugin : public IHuaweiVRControllerPlugin
{
public:

	virtual TSharedPtr< class IInputDevice > CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) override
	{
		// Register the FKeys
		EKeys::AddKey(FKeyDetails(FHuaweiVRKey::HuaweiVR_Touch_Left_Thumbstick, LOCTEXT("HuaweiVR_Touch_Left_Thumbstick", "HuaweiVR Touch (L) Thumbstick CapTouch"), FKeyDetails::GamepadKey | FKeyDetails::FloatAxis));
		EKeys::AddKey(FKeyDetails(FHuaweiVRKey::HuaweiVR_Touch_Left_FaceButton1, LOCTEXT("HuaweiVR_Touch_Left_FaceButton1", "HuaweiVR Touch (L) X Button CapTouch"), FKeyDetails::GamepadKey | FKeyDetails::FloatAxis));
		EKeys::AddKey(FKeyDetails(FHuaweiVRKey::HuaweiVR_Touch_Left_Trigger, LOCTEXT("HuaweiVR_Touch_Left_Trigger", "HuaweiVR Touch (L) Trigger CapTouch"), FKeyDetails::GamepadKey | FKeyDetails::FloatAxis));
		EKeys::AddKey(FKeyDetails(FHuaweiVRKey::HuaweiVR_Touch_Left_Shoulder, LOCTEXT("HuaweiVR_Touch_Left_FaceButton2", "HuaweiVR Touch (L) Y Button CapTouch"), FKeyDetails::GamepadKey | FKeyDetails::FloatAxis));

		EKeys::AddKey(FKeyDetails(FHuaweiVRKey::HuaweiVR_Touch_Right_Thumbstick, LOCTEXT("HuaweiVR_Touch_Right_Thumbstick", "HuaweiVR Touch (R) Thumbstick CapTouch"), FKeyDetails::GamepadKey | FKeyDetails::FloatAxis));
		EKeys::AddKey(FKeyDetails(FHuaweiVRKey::HuaweiVR_Touch_Right_FaceButton1, LOCTEXT("HuaweiVR_Touch_Right_FaceButton1", "HuaweiVR Touch (R) A Button CapTouch"), FKeyDetails::GamepadKey | FKeyDetails::FloatAxis));
		EKeys::AddKey(FKeyDetails(FHuaweiVRKey::HuaweiVR_Touch_Right_Trigger, LOCTEXT("HuaweiVR_Touch_Right_Trigger", "HuaweiVR Touch (R) Trigger CapTouch"), FKeyDetails::GamepadKey | FKeyDetails::FloatAxis));
		EKeys::AddKey(FKeyDetails(FHuaweiVRKey::HuaweiVR_Touch_Right_Shoulder, LOCTEXT("HuaweiVR_Touch_Right_FaceButton2", "HuaweiVR Touch (R) B Button CapTouch"), FKeyDetails::GamepadKey | FKeyDetails::FloatAxis));
#if HVRCONTROLLER_SUPPORTED_PLATFORMS

		UE_LOG(LogHuaweiVRSDK, Log, TEXT("In CreateInputDevice."));
		gHvrController = new FHuaweiVRController(InMessageHandler);
		return TSharedPtr< class IInputDevice >(gHvrController);

#else
		return nullptr;
#endif
	}
};

IMPLEMENT_MODULE(FHuaweiVRControllerPlugin, HuaweiVRController)

FName FHuaweiVRController::DeviceTypeName(TEXT("HuaweiVRController"));

FHuaweiVRController::FHuaweiVRController(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
	: mControllerHandle(nullptr),
	MessageHandler(InMessageHandler),
	mNotifyOnce(false),
	mIsRightController(false),
	mIsHomePressed(false),
	mIsHomeUp(false),
	mIsBackPressed(false),
	mIsTriggerPressed(false),
	mIsCapture(true),
	mIsResetOrientation(true),
	mIsJumpToSetting(true),
	mController(nullptr),
	mLanguage(1),
	mMessageType(-1),
	mIsDetectMessage(false) {

	mFrameNumber = 0;

#if HVRCONTROLLER_SUPPORTED_PLATFORMS
	IModularFeatures::Get().RegisterModularFeature(GetModularFeatureName(), this);
#endif
	// Setup button mappings
	Buttons[(int32)EControllerHand::Right][EHVRControllerButton::Home] = FGamepadKeyNames::SpecialRight;
	Buttons[(int32)EControllerHand::Left][EHVRControllerButton::Home] = FGamepadKeyNames::SpecialLeft;

	Buttons[(int32)EControllerHand::Right][EHVRControllerButton::Back] = FGamepadKeyNames::RightAnalogX; // MotionController_Right_Shoulder;
	Buttons[(int32)EControllerHand::Left][EHVRControllerButton::Back] = FGamepadKeyNames::LeftAnalogX; // MotionController_Left_Shoulder;

	Buttons[(int32)EControllerHand::Right][EHVRControllerButton::Confirm] = FGamepadKeyNames::RightAnalogY; // MotionController_Right_FaceButton1;
	Buttons[(int32)EControllerHand::Left][EHVRControllerButton::Confirm] = FGamepadKeyNames::LeftAnalogY; // MotionController_Left_FaceButton1;

	Buttons[(int32)EControllerHand::Right][EHVRControllerButton::Trigger] = FGamepadKeyNames::RightTriggerAnalog; // MotionController_Right_Trigger;
	Buttons[(int32)EControllerHand::Left][EHVRControllerButton::Trigger] = FGamepadKeyNames::LeftTriggerAnalog; // MotionController_Left_Trigger;

	Buttons[(int32)EControllerHand::Right][EHVRControllerButton::Trigger6DoF] = FGamepadKeyNames::RightTriggerAnalog; // MotionController_Right_Trigger;
	Buttons[(int32)EControllerHand::Left][EHVRControllerButton::Trigger6DoF] = FGamepadKeyNames::LeftTriggerAnalog; // MotionController_Left_Trigger;

	Buttons[(int32)EControllerHand::Right][EHVRControllerButton::Grip6DoF] = FGamepadKeyNames::RightShoulder; // MotionController_Right_Grip1;
	Buttons[(int32)EControllerHand::Left][EHVRControllerButton::Grip6DoF] = FGamepadKeyNames::LeftShoulder;  // MotionController_Left_Grip1;

	Buttons[(int32)EControllerHand::Right][EHVRControllerButton::AX6DoF] = FGamepadKeyNames::RightAnalogY; // MotionController_Right_FaceButton1;
	Buttons[(int32)EControllerHand::Left][EHVRControllerButton::AX6DoF] = FGamepadKeyNames::LeftAnalogY; // MotionController_Left_FaceButton1;

	Buttons[(int32)EControllerHand::Right][EHVRControllerButton::BY6DoF] = FGamepadKeyNames::RightAnalogX; // MotionController_Right_Shoulder;
	Buttons[(int32)EControllerHand::Left][EHVRControllerButton::BY6DoF] = FGamepadKeyNames::LeftAnalogX; // MotionController_Left_Shoulder;

	Buttons[(int32)EControllerHand::Right][EHVRControllerButton::Thumbstick6DoF] = FGamepadKeyNames::RightThumb; //MotionController_Right_Thumbstick;
	Buttons[(int32)EControllerHand::Left][EHVRControllerButton::Thumbstick6DoF] = FGamepadKeyNames::LeftThumb;  //MotionController_Left_Thumbstick;

	Buttons[(int32)EControllerHand::Right][EHVRControllerButton::HomeMenu6DoF] = FGamepadKeyNames::SpecialRight;
	Buttons[(int32)EControllerHand::Left][EHVRControllerButton::HomeMenu6DoF] = FGamepadKeyNames::SpecialLeft;

	Buttons[(int32)EControllerHand::Right][EHVRControllerButton::TouchPadLeft] = FGamepadKeyNames::DPadLeft;     // MotionController_Right_FaceButton2;
	Buttons[(int32)EControllerHand::Left][EHVRControllerButton::TouchPadLeft] = FGamepadKeyNames::LeftStickLeft; // MotionController_Left_FaceButton2;

	Buttons[(int32)EControllerHand::Right][EHVRControllerButton::TouchPadRight] = FGamepadKeyNames::DPadRight;         // MotionController_Right_FaceButton3;
	Buttons[(int32)EControllerHand::Left][EHVRControllerButton::TouchPadRight] = FGamepadKeyNames::LeftStickRight;	   // MotionController_Left_FaceButton3;

	Buttons[(int32)EControllerHand::Right][EHVRControllerButton::TouchPadUp] = FGamepadKeyNames::DPadUp;     // MotionController_Right_FaceButton4;
	Buttons[(int32)EControllerHand::Left][EHVRControllerButton::TouchPadUp] = FGamepadKeyNames::LeftStickUp; // MotionController_Left_FaceButton4;

	Buttons[(int32)EControllerHand::Right][EHVRControllerButton::TouchPadDown] = FGamepadKeyNames::DPadDown;     // MotionController_Right_FaceButton5;
	Buttons[(int32)EControllerHand::Left][EHVRControllerButton::TouchPadDown] = FGamepadKeyNames::LeftStickDown; // MotionController_Left_FaceButton5;

	Buttons[(int32)EControllerHand::Right][EHVRControllerButton::TouchPadTouch] = FGamepadKeyNames::FaceButtonRight; // MotionController_Right_FaceButton6;
	Buttons[(int32)EControllerHand::Left][EHVRControllerButton::TouchPadTouch] = FGamepadKeyNames::FaceButtonLeft;   // MotionController_Right_FaceButton6;

	Buttons[(int32)EControllerHand::Right][EHVRControllerButton::TouchPadTouching] = FGamepadKeyNames::FaceButtonTop;   // MotionController_Right_FaceButton7;
	Buttons[(int32)EControllerHand::Left][EHVRControllerButton::TouchPadTouching] = FGamepadKeyNames::FaceButtonBottom; // MotionController_Left_FaceButton7;

	mLanguage = GetLanguage();
	UE_LOG(LogHuaweiVRSDK, Log, TEXT("System language is: %d"), mLanguage);

#if HVRCONTROLLER_SUPPORTED_ANDROID_PLATFORMS
	HVR_SetInitVariables(FJavaWrapper::GameActivityThis);
	HVR_SendMessage(UNREAL_PLUGIN_FLAG, "true");
	mControllerHandle = HVR_Android_GetControllerHandle();
	HVR_Android_RegistControllerStatusCallBack(mControllerHandle, controllerStatusCallBack);
	mController = HVR_Android_GetControllerByIndex(mControllerHandle, 0);

	mRightController = HVR_Android_GetControllerByIndex(mControllerHandle, 0);
	mLeftController = HVR_Android_GetControllerByIndex(mControllerHandle, 1);
	if (NULL != mLeftController) {
		mController = mLeftController;
		mIsRightController = false;
	}
	else {
		mController = mRightController;
		mIsRightController = true;
	}

#endif // HVRCONTROLLER_SUPPORTED_ANDROID_PLATFORMS
}

FHuaweiVRController::~FHuaweiVRController() {
#if HVRCONTROLLER_SUPPORTED_PLATFORMS
	IModularFeatures::Get().UnregisterModularFeature(GetModularFeatureName(), this);
#endif
}

double  FHuaweiVRController::getCurrentTime()
{
#if HVRCONTROLLER_SUPPORTED_ANDROID_PLATFORMS
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec / (double)1000000;
#else
	return 0;
#endif
}

string  FHuaweiVRController::getCurrentTimeStr()
{
#if HVRCONTROLLER_SUPPORTED_ANDROID_PLATFORMS
	time_t t = time(NULL);
	char ch[64] = { 0 };
	strftime(ch, sizeof(ch) - 1, "%Y%m%d%H%M%S", localtime(&t));     //年-月-日 时-分-秒  
	return ch;
#else
	return 0;
#endif
}

bool FHuaweiVRController::IsAvailable() const
{
	bool isAvailable = false;
#if HVRCONTROLLER_SUPPORTED_ANDROID_PLATFORMS
	bool isLeftAvailable = NULL != mLeftController && HVR_Android_IsControllerAvailable(mLeftController);
	bool isRightAvailable = NULL != mRightController && HVR_Android_IsControllerAvailable(mRightController);
	isAvailable = isLeftAvailable || isRightAvailable;
#endif // HVRCONTROLLER_SUPPORTED_ANDROID_PLATFORMS	

	return isAvailable;
}

bool FHuaweiVRController::IsAvailable(const EControllerHand DeviceHand) const
{
	bool isAvailable = false;
#if HVRCONTROLLER_SUPPORTED_ANDROID_PLATFORMS
	if (DeviceHand == EControllerHand::Left)
	{
		isAvailable = NULL != mLeftController && HVR_Android_IsControllerAvailable(mLeftController);
	}
	if (DeviceHand == EControllerHand::Right)
	{
		isAvailable = NULL != mRightController && HVR_Android_IsControllerAvailable(mRightController);
	}
#endif // HVRCONTROLLER_SUPPORTED_ANDROID_PLATFORMS	

	return isAvailable;
}


void FHuaweiVRController::ProcessControllerButtonEvents()
{
	for (int CIndex = 0; CIndex < 2; CIndex++)
	{
#if HVRCONTROLLER_SUPPORTED_PLATFORMS
		void* mCurrentController = NULL;
		if (CIndex == 0)
		{
			mCurrentController = mLeftController;
		}
		else
		{
			mCurrentController = mRightController;
		}

		if (mCurrentController == NULL)
		{
			continue;
		}
		// Capture our current button states
		bool CurrentButtonStates[EHVRControllerButton::TotalButtonCount] = { 0 };
		FVector2D TranslatedLocation = FVector2D::ZeroVector;
		float fTriggerAxis = 0.0f;
		float fGripAxis = 0.0f;
#if HVRCONTROLLER_SUPPORTED_ANDROID_PLATFORMS
		if (HVR_Android_IsControllerAvailable(mCurrentController))
		{
			//// Process our known set of buttons
			if (HVR_Android_IsControllerButtonDown(mCurrentController, ButtonType::ButtonHome) || (HVR_Android_IsControllerButtonDown(mCurrentController, ButtonType::ButtonHomeMenu6DoF) && (mCurrentController == mRightController))) 
			{
				mIsHomePressed = false;
				mIsHomeUp = false;
				mIsCapture = true;
				mIsResetOrientation = true;
				mHomeDownTime = getCurrentTime();
			}
			if (HVR_Android_IsControllerButtonPressed(mCurrentController, ButtonType::ButtonHome) || (HVR_Android_IsControllerButtonDown(mCurrentController, ButtonType::ButtonHomeMenu6DoF) && (mCurrentController == mRightController)))
			{
				CurrentButtonStates[EHVRControllerButton::Home] = true;
				CurrentButtonStates[EHVRControllerButton::HomeMenu6DoF] = true;
				mIsHomePressed = true;
				mIsHomeUp = false;
			}
			else if (HVR_Android_IsControllerButtonUp(mCurrentController, ButtonType::ButtonHome) || (HVR_Android_IsControllerButtonDown(mCurrentController, ButtonType::ButtonHomeMenu6DoF) && (mCurrentController == mRightController))) 
			{
				//UE_LOG(LogHuaweiVRSDK, Log, TEXT("HVR  ButtonHome Up"));
				CurrentButtonStates[EHVRControllerButton::Home] = false;
				CurrentButtonStates[EHVRControllerButton::HomeMenu6DoF] = false;
				mIsHomePressed = false;
				mIsHomeUp = true;
				mIsCapture = true;
				mIsResetOrientation = true;
			}
			else {
				mIsHomePressed = false;
				mIsHomeUp = false;
				mIsCapture = true;
				mIsResetOrientation = true;
			}

			if (HVR_Android_IsControllerButtonDown(mCurrentController, ButtonType::ButtonBack) || HVR_Android_IsControllerButtonDown(mCurrentController, ButtonType::ButtonBY6DoF)) {
				mIsBackPressed = false;
				mIsJumpToSetting = true;
				mBackDownTime = getCurrentTime();
			}

			if (HVR_Android_IsControllerButtonPressed(mCurrentController, ButtonType::ButtonBack) || HVR_Android_IsControllerButtonPressed(mCurrentController, ButtonType::ButtonBY6DoF))
			{
				CurrentButtonStates[EHVRControllerButton::Back] = true;
				CurrentButtonStates[EHVRControllerButton::BY6DoF] = true;
				mIsBackPressed = true;

			}
			else if (HVR_Android_IsControllerButtonUp(mCurrentController, ButtonType::ButtonBack) || HVR_Android_IsControllerButtonUp(mCurrentController, ButtonType::ButtonBY6DoF)) {
				CurrentButtonStates[EHVRControllerButton::Back] = false;
				CurrentButtonStates[EHVRControllerButton::BY6DoF] = false;
				mIsBackPressed = false;
				mIsJumpToSetting = true;
			}
			else {
				mIsBackPressed = false;
				mIsJumpToSetting = true;
			}

			if (HVR_Android_IsControllerButtonPressed(mCurrentController, ButtonType::ButtonConfirm) || HVR_Android_IsControllerButtonPressed(mCurrentController, ButtonType::ButtonAX6DoF))
			{
				CurrentButtonStates[EHVRControllerButton::Confirm] = true;
				CurrentButtonStates[EHVRControllerButton::AX6DoF] = true;
			}
			else if (HVR_Android_IsControllerButtonUp(mCurrentController, ButtonType::ButtonConfirm) || HVR_Android_IsControllerButtonUp(mCurrentController, ButtonType::ButtonAX6DoF))
			{
				CurrentButtonStates[EHVRControllerButton::Confirm] = false;
				CurrentButtonStates[EHVRControllerButton::AX6DoF] = false;
			}

			if (HVR_Android_IsControllerButtonPressed(mCurrentController, ButtonType::ButtonThumbstick6DoF))
			{
				CurrentButtonStates[EHVRControllerButton::Thumbstick6DoF] = true;
			}
			else if ( HVR_Android_IsControllerButtonUp(mCurrentController, ButtonType::ButtonThumbstick6DoF))
			{
				CurrentButtonStates[EHVRControllerButton::Thumbstick6DoF] = false;
			}

			if (HVR_Android_IsControllerButtonPressed(mCurrentController, ButtonType::ButtonGrip6DoF))
			{
				CurrentButtonStates[EHVRControllerButton::Grip6DoF] = true;
			}
			else if (HVR_Android_IsControllerButtonUp(mCurrentController, ButtonType::ButtonGrip6DoF))
			{
				CurrentButtonStates[EHVRControllerButton::Grip6DoF] = false;
			}

			if (HVR_Android_IsControllerButtonDown(mCurrentController, ButtonType::ButtonVolumeInc))
			{
				mVolumeIncDownTime = getCurrentTime();
				HVR_Android_AdjustSystemVolume(1);
			}

			if (HVR_Android_IsControllerButtonDown(mCurrentController, ButtonType::ButtonVolumeDec))
			{
				mVolumeDecDownTime = getCurrentTime();
				HVR_Android_AdjustSystemVolume(0);
			}

			if (HVR_Android_IsControllerButtonPressed(mCurrentController, ButtonType::ButtonVolumeInc))
			{
				double now = getCurrentTime();
				if (now - mVolumeIncDownTime > 0.5)
				{
					if (now - mLastVolumeIncTime > 0.1)
					{
						mLastVolumeIncTime = now;
						HVR_Android_AdjustSystemVolume(1);
					}

				}
			}

			if (HVR_Android_IsControllerButtonPressed(mCurrentController, ButtonType::ButtonVolumeDec))
			{
				double now = getCurrentTime();
				if (now - mVolumeDecDownTime > 0.5)
				{
					if (now - mLastVolumeDecTime > 0.1)
					{
						mLastVolumeDecTime = now;
						HVR_Android_AdjustSystemVolume(0);
					}

				}
			}

			if (HVR_Android_IsControllerButtonDown(mCurrentController, ButtonType::ButtonTrigger) || HVR_Android_IsControllerButtonDown(mCurrentController, ButtonType::ButtonTrigger6DoF))
			{
				mIsTriggerPressed = false;
				mIsCapture = true;
				mTriggerDownTime = getCurrentTime();
			}

			if (HVR_Android_IsControllerButtonPressed(mCurrentController, ButtonType::ButtonTrigger) || HVR_Android_IsControllerButtonPressed(mCurrentController, ButtonType::ButtonTrigger6DoF))
			{
				CurrentButtonStates[EHVRControllerButton::Trigger] = true;
				CurrentButtonStates[EHVRControllerButton::Trigger6DoF] = true;
				mIsTriggerPressed = true;
			}
			else if (HVR_Android_IsControllerButtonUp(mCurrentController, ButtonType::ButtonTrigger) || HVR_Android_IsControllerButtonUp(mCurrentController, ButtonType::ButtonTrigger6DoF))
			{
				CurrentButtonStates[EHVRControllerButton::Trigger] = false;
				CurrentButtonStates[EHVRControllerButton::Trigger6DoF] = false;
				mIsTriggerPressed = false;
				mIsCapture = true;
			}
			else {
				mIsTriggerPressed = false;
				mIsCapture = true;
			}

			if (HVR_Android_IsControllerTouchpadTouchDown(mCurrentController))
			{
				CurrentButtonStates[EHVRControllerButton::TouchPadTouch] = true;
			}
			else if (HVR_Android_IsControllerTouchpadTouchUp(mCurrentController))
			{
				CurrentButtonStates[EHVRControllerButton::TouchPadTouch] = false;
			}

			CurrentButtonStates[EHVRControllerButton::TouchPadTouching] = HVR_Android_IsControllerTouchpadTouching(mCurrentController);

			if (HVR_Android_IsControllerTouchpadTouchDown(mCurrentController)) {
				if (0 != HVR_Android_GetControllerTouchpadTouchPos(mCurrentController, mTouchDownPos)) {
					return;
				}
			}
			else if (HVR_Android_IsControllerTouchpadTouchUp(mCurrentController)) {
				if (0 != HVR_Android_GetControllerTouchpadTouchPos(mCurrentController, mTouchUpPos)) {
					return;
				}
				if (pow(mTouchUpPos.y - mTouchDownPos.y, 2) + pow(mTouchUpPos.x - mTouchDownPos.x, 2) < 0.04) {
					return;
				}
				if (mTouchUpPos.x == mTouchDownPos.x) {

					CurrentButtonStates[EHVRControllerButton::TouchPadDown] = (mTouchUpPos.y > mTouchDownPos.y);
					CurrentButtonStates[EHVRControllerButton::TouchPadUp] = (mTouchUpPos.y < mTouchDownPos.y);

				}
				else {
					float rakeRatio = (mTouchUpPos.y - mTouchDownPos.y) / (mTouchUpPos.x - mTouchDownPos.x);
					if (rakeRatio > -1 && rakeRatio < 1) {
						CurrentButtonStates[EHVRControllerButton::TouchPadLeft] = (mTouchUpPos.x < mTouchDownPos.x);
						CurrentButtonStates[EHVRControllerButton::TouchPadRight] = (mTouchUpPos.x > mTouchDownPos.x);
					}
					else {
						CurrentButtonStates[EHVRControllerButton::TouchPadDown] = (mTouchUpPos.y > mTouchDownPos.y);
						CurrentButtonStates[EHVRControllerButton::TouchPadUp] = (mTouchUpPos.y < mTouchDownPos.y);
					}
				}
			}
			if (mIsHomePressed && mIsTriggerPressed) {
				if (mIsCapture) {
					double starttime = mHomeDownTime > mTriggerDownTime ? mHomeDownTime : mTriggerDownTime;
					if (getCurrentTime() - starttime >= 1.0) {
						mIsCapture = false;
						UE_LOG(LogHuaweiVRSDK, Log, TEXT("Shotcut Key captureEyeImage"));
						char filePath[256] = { 0 };
						sprintf(filePath, "/sdcard/Pictures/Screenshots/ScreenShotVR_%s.jpg", getCurrentTimeStr().c_str());
						int length = strlen(filePath);
						int ret = HVR_Android_CaptureEyeImage(filePath, length);
						mMessage.Empty();
						mIsDetectMessage = true;
						if (ret != 0) {
							mMessageType = (int)NotifyEvent_CaptureImageFailed;
							if (1 == mLanguage) {
								mMessage.Add(HuaweiVRMessageKey_Content, m_text_CaptureImageFailedMsg_zh);
								mMessage.Add(HuaweiVRMessageKey_Title, m_text_CaptureImageFailedTitle_zh);
							}
							else if (0 == mLanguage) {
								mMessage.Add(HuaweiVRMessageKey_Content, m_text_CaptureImageFailedMsg_en);
								mMessage.Add(HuaweiVRMessageKey_Title, m_text_CaptureImageFailedTitle_en);
							}
							UE_LOG(LogHuaweiVRSDK, Log, TEXT("CaptureEyeImage Failed "));
						}
						else {
							mMessageType = (int)NotifyEvent_CaptureImageSucceed;
							if (1 == mLanguage) {
								mMessage.Add(HuaweiVRMessageKey_Title, m_text_CaptureImageSucceed_zh);
							}
							else if (0 == mLanguage) {
								mMessage.Add(HuaweiVRMessageKey_Title, m_text_CaptureImageSucceed_en);
							}
						}

					}
				}
			}
			else if (mIsHomePressed) {
				if (mIsResetOrientation) {
					if (getCurrentTime() - mHomeDownTime >= 1.0) {
						mIsResetOrientation = false;
						void* helmet = HVR_Android_GetHelmet();
						if (NULL != helmet && 0 != HVR_Android_ResetYaw(helmet)) {
							UE_LOG(LogHuaweiVRSDK, Log, TEXT("Reset Yaw Failed"));
						}
						if (0 != HVR_Android_ResetControllerCenter(mCurrentController)) {
							UE_LOG(LogHuaweiVRSDK, Log, TEXT("Reset Controller Center Failed"));
						}
					}
				}
			}
			if (mIsHomeUp) {
				if (getCurrentTime() - mHomeDownTime < 1.0) {
					HVR_Android_JumpToLaucher();

				}
			}
			if (mIsBackPressed) {
				if (mIsJumpToSetting) {
					if (getCurrentTime() - mBackDownTime >= 3.0) {
						mIsJumpToSetting = false;
						HVR_Android_JumpToSetting();

					}
				}
			}

			// CapTouch
			bool IsTouchingTrigger6Dof = HVR_Android_IsControllerButtonTouching(mCurrentController, ButtonType::ButtonTrigger6DoF);
			bool IsTouchingThumbstick6DoF = HVR_Android_IsControllerButtonTouching(mCurrentController, ButtonType::ButtonThumbstick6DoF);
			bool IsTouchingAX6DoF = HVR_Android_IsControllerButtonTouching(mCurrentController, ButtonType::ButtonAX6DoF);
			bool IsTouchingBY6Dof = HVR_Android_IsControllerButtonTouching(mCurrentController, ButtonType::ButtonBY6DoF);
			if (CIndex == 0)
			{
				MessageHandler->OnControllerAnalog(FHuaweiVRKeyNames::HuaweiVR_Touch_Left_Thumbstick, 0, IsTouchingThumbstick6DoF ? 1.0f : 0.0f);
				MessageHandler->OnControllerAnalog(FHuaweiVRKeyNames::HuaweiVR_Touch_Left_Trigger, 0, IsTouchingTrigger6Dof ? 1.0f : 0.0f);
				MessageHandler->OnControllerAnalog(FHuaweiVRKeyNames::HuaweiVR_Touch_Left_FaceButton1, 0, IsTouchingAX6DoF ? 1.0f : 0.0f);
				MessageHandler->OnControllerAnalog(FHuaweiVRKeyNames::HuaweiVR_Touch_Left_Shoulder, 0, IsTouchingBY6Dof ? 1.0f : 0.0f);

			}
			else
			{
				MessageHandler->OnControllerAnalog(FHuaweiVRKeyNames::HuaweiVR_Touch_Right_Thumbstick, 0, IsTouchingThumbstick6DoF ? 1.0f : 0.0f);
				MessageHandler->OnControllerAnalog(FHuaweiVRKeyNames::HuaweiVR_Touch_Right_Trigger, 0, IsTouchingTrigger6Dof ? 1.0f : 0.0f);
				MessageHandler->OnControllerAnalog(FHuaweiVRKeyNames::HuaweiVR_Touch_Right_FaceButton1, 0, IsTouchingAX6DoF ? 1.0f : 0.0f);
				MessageHandler->OnControllerAnalog(FHuaweiVRKeyNames::HuaweiVR_Touch_Right_Shoulder, 0, IsTouchingBY6Dof ? 1.0f : 0.0f);
			}
			// CapTouch

			// Trigger and Grip Data
			if (0 != HVR_Android_GetControllerTriggerData(mCurrentController, fTriggerAxis))
			{
				return;
			}
			if (0 != HVR_Android_GetControllerGripData(mCurrentController, fGripAxis))
			{
				return;
			}
			// Trigger and Grip Data
		}
#endif // HVRCONTROLLER_SUPPORTED_ANDROID_PLATFORMS
		TranslatedLocation = GetHVRControllerThumbstickPos(CIndex == 0 ? EControllerHand::Left : EControllerHand::Right);
		//MessageHandler->OnControllerAnalog(CIndex == 0 ? FGamepadKeyNames::MotionController_Left_Thumbstick_X : FGamepadKeyNames::MotionController_Right_Thumbstick_X, 0, TranslatedLocation.X);
		//MessageHandler->OnControllerAnalog(CIndex == 0 ? FGamepadKeyNames::MotionController_Left_Thumbstick_Y : FGamepadKeyNames::MotionController_Right_Thumbstick_Y, 0, TranslatedLocation.Y);
		//MessageHandler->OnControllerAnalog(CIndex == 0 ? FGamepadKeyNames::MotionController_Left_TriggerAxis : FGamepadKeyNames::MotionController_Right_TriggerAxis, 0, fTriggerAxis);
		//MessageHandler->OnControllerAnalog(CIndex == 0 ? FGamepadKeyNames::MotionController_Left_Grip1Axis : FGamepadKeyNames::MotionController_Right_Grip1Axis, 0, fGripAxis);

		//// Process buttons for both hands at the same time
		for (int32 ButtonIndex = 0; ButtonIndex < (int32)EHVRControllerButton::TotalButtonCount; ++ButtonIndex)
		{
			bool LastButtonStates = CIndex == 0 ? Left_LastButtonStates[ButtonIndex] : Right_LastButtonStates[ButtonIndex];
			if (CurrentButtonStates[ButtonIndex] != LastButtonStates)
			{
				// OnDown
				if (CurrentButtonStates[ButtonIndex])
				{
					if (CIndex == 0)
					{
						MessageHandler->OnControllerButtonPressed(Buttons[(int32)EControllerHand::Left][ButtonIndex], 0, false);
					}
					else
					{
						MessageHandler->OnControllerButtonPressed(Buttons[(int32)EControllerHand::Right][ButtonIndex], 0, false);
					}
				}
				// On Up
				else
				{
					if (CIndex == 0)
					{
						MessageHandler->OnControllerButtonReleased(Buttons[(int32)EControllerHand::Left][ButtonIndex], 0, false);
					}
					else
					{
						MessageHandler->OnControllerButtonReleased(Buttons[(int32)EControllerHand::Right][ButtonIndex], 0, false);
					}
				}
			}

			// update state for next time
			if (CIndex == 0)
			{
				Left_LastButtonStates[ButtonIndex] = CurrentButtonStates[ButtonIndex];
			}
			else
			{
				Right_LastButtonStates[ButtonIndex] = CurrentButtonStates[ButtonIndex];
			}
		}
#endif  
	}
}


void FHuaweiVRController::UpdateControllerData(float DeltaTime) {

#if  HVRCONTROLLER_SUPPORTED_ANDROID_PLATFORMS
	HVR_Android_UpdateControllerData(mControllerHandle);
	if (!mNotifyOnce) {
		mNotifyOnce = true;
		int indice[16];
		if (0 == HVR_IsBluetoothEnable()) {
			mMessage.Empty();
			mIsDetectMessage = true;
			mMessageType = (int)NotifyEvent_BluetoothDisable;
			if (1 == mLanguage) {
				mMessage.Add(HuaweiVRMessageKey_Content, m_text_BluetoothDisableMsg_zh);
				mMessage.Add(HuaweiVRMessageKey_Title, m_text_BluetoothDisableTitle_zh);
			}
			else if (0 == mLanguage) {

				mMessage.Add(HuaweiVRMessageKey_Content, m_text_BluetoothDisableMsg_en);
				mMessage.Add(HuaweiVRMessageKey_Title, m_text_BluetoothDisableTitle_en);
			}
		}
		else {
			int ret = HVR_Android_GetValidControllerIndices(mControllerHandle, indice, 16);
			if (ret <= 0) {
				if (-1 == ret) {
					mNotifyOnce = false;
				}
				else {
					if (-2 == ret) {
						UE_LOG(LogHuaweiVRSDK, Log, TEXT("Start controller service timeout "));
					}
					UE_LOG(LogHuaweiVRSDK, Log, TEXT("Can not find valid controller "));
					mMessage.Empty();
					mIsDetectMessage = true;
					mMessageType = (int)NotifyEvent_NoControllerJoin;
					if (1 == mLanguage) {
						mMessage.Add(HuaweiVRMessageKey_Content, m_text_NoAvailableControllerMsg_zh);
						mMessage.Add(HuaweiVRMessageKey_Title, m_text_NoAvailableControllerTitle_zh);
					}
					else if (0 == mLanguage) {
						mMessage.Add(HuaweiVRMessageKey_Content, m_text_NoAvailableControllerMsg_en);
						mMessage.Add(HuaweiVRMessageKey_Title, m_text_NoAvailableControllerTitle_en);
					}
					UE_LOG(LogHuaweiVRSDK, Log, TEXT(" Can not find valid controller  "));
				}
			}
		}
}
#endif //  HVRCONTROLLER_SUPPORTED_ANDROID_PLATFORMS	

}

FVector FHuaweiVRController::ConvertHvrVectorToUnreal(float x, float y, float z, float WorldToMetersScale) const
{
	FVector Result;

	// Hvr: Negative Z is Forward, UE: Positive X is Forward.
	Result.X = z * WorldToMetersScale;
	// Hvr: Positive X is Right, UE: Positive Y is Right.
	Result.Y = x * WorldToMetersScale;
	// Hvr: Positive Y is Up, UE: Positive Z is Up
	Result.Z = y * WorldToMetersScale;

	return Result;

}

FQuat FHuaweiVRController::ConvertHvrQuaternionToUnreal(float w, float x, float y, float z) const
{
	FQuat Result = FQuat(z, x, y, w);
	return Result;
}

int FHuaweiVRController::GetLanguage()
{
	FString language = FGenericPlatformMisc::GetDefaultLanguage();
	if (language.StartsWith("zh_CN")) {
		return 1;
	}
	else if (language.StartsWith("zh_TW") || language.StartsWith("zh_MO") || language.StartsWith("zh_HK"))
	{
		return 2;
	}
	else
	{
		return 0;
	}
}

float FHuaweiVRController::GetWorldToMetersScale() const
{
	if (IsInGameThread() && GWorld != nullptr)
	{
		return GWorld->GetWorldSettings()->WorldToMeters;
	}
	// Default value, assume Unreal units are in centimeters
	return 100.0f;
}

bool FHuaweiVRController::DetectHVRControllerMessage(int& type, int& priority, TMap<FString, FString>& message) {
	if (mFrameNumber < SKIP_FRAME)
	{
		mFrameNumber++;
		return false;
	}

	type = mMessageType;
	message = mMessage;
	return mIsDetectMessage;
}

HVR_CONTROLLER_STATUS  FHuaweiVRController::GetHVRControllerStatus(const EControllerHand DeviceHand) {
#if HVRCONTROLLER_SUPPORTED_ANDROID_PLATFORMS
	if (DeviceHand == EControllerHand::Left && mLeftController != NULL)
	{
		return HVR_Android_GetControllerStatus(mLeftController);
	}
	if (DeviceHand == EControllerHand::Right && mRightController != NULL)
	{
		return HVR_Android_GetControllerStatus(mRightController);
	}
#endif
	return HVR_CONTROLLER_STATUS::HVR_CONTROLLER_DISCONNECTED;
}

FVector2D FHuaweiVRController::GetHVRControllerTouchpadTouchPos(const EControllerHand DeviceHand) {
	FVector2D pos = FVector2D::ZeroVector;
#if HVRCONTROLLER_SUPPORTED_ANDROID_PLATFORMS
	VrVector2f posture;
	if (DeviceHand == EControllerHand::Left && mLeftController != NULL)
	{
		if (0 == HVR_Android_GetControllerTouchpadTouchPos(mLeftController, posture)) {
			pos.X = posture.x;
			pos.Y = posture.y;
		}
	}
	if (DeviceHand == EControllerHand::Right && mRightController != NULL)
	{
		if (0 == HVR_Android_GetControllerTouchpadTouchPos(mRightController, posture)) {
			pos.X = posture.x;
			pos.Y = posture.y;
		}
}
#endif
	return pos;
}

FVector2D FHuaweiVRController::GetHVRControllerThumbstickPos(const EControllerHand DeviceHand) {
	FVector2D pos = FVector2D::ZeroVector;
#if HVRCONTROLLER_SUPPORTED_ANDROID_PLATFORMS
	VrVector2f posture;
	if (DeviceHand == EControllerHand::Left && mLeftController != NULL)
	{
		if (0 == HVR_Android_GetControllerThumbstickPos(mLeftController, posture)) {
			pos.X = posture.x;
			pos.Y = posture.y;
		}
	}
	if (DeviceHand == EControllerHand::Right && mRightController != NULL)
	{
		if (0 == HVR_Android_GetControllerThumbstickPos(mRightController, posture)) {
			pos.X = posture.x;
			pos.Y = posture.y;
		}
}
#endif
	return pos;
}

void FHuaweiVRController::Tick(float DeltaTime)
{
	UpdateControllerData(DeltaTime);
}

void FHuaweiVRController::SendControllerEvents()
{
	ProcessControllerButtonEvents();
}

void FHuaweiVRController::SetMessageHandler(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
{
	MessageHandler = InMessageHandler;
}

bool FHuaweiVRController::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	return false;
}

void FHuaweiVRController::SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value)
{
}

void FHuaweiVRController::SetChannelValues(int32 ControllerId, const FForceFeedbackValues &values)
{
}

bool FHuaweiVRController::GetControllerOrientationAndPosition(const int32 ControllerIndex, const EControllerHand DeviceHand, FRotator& OutOrientation, FVector& OutPosition, float WorldToMetersScale) const
{
#if HVRCONTROLLER_SUPPORTED_ANDROID_PLATFORMS
	if (IsAvailable(DeviceHand))
	{
		/*
		UWorld* World = GWorld ? GWorld->GetWorld() : nullptr;
		APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
		APawn* pawn = PlayerController ? PlayerController->GetPawn() : nullptr;
		if (pawn == nullptr) {
			LOGE("Fail to get pawn");
			return false;
		}*/
		OutPosition = FVector::ZeroVector;
		OutOrientation = FRotator::ZeroRotator;
		VrPosturef ControllerPos;
		if (DeviceHand == EControllerHand::Left && mLeftController != NULL)
		{
			HVR_Android_GetControllerPosture(mLeftController, ControllerPos);
		}
		if (DeviceHand == EControllerHand::Right && mRightController != NULL)
		{
			HVR_Android_GetControllerPosture(mRightController, ControllerPos);
		}
		FVector Position = ConvertHvrVectorToUnreal(ControllerPos.position.x, ControllerPos.position.y, ControllerPos.position.z, WorldToMetersScale);
		FQuat Orientation = ConvertHvrQuaternionToUnreal(ControllerPos.rotation.w, ControllerPos.rotation.x, ControllerPos.rotation.y, ControllerPos.rotation.z);		
		OutOrientation = Orientation.Rotator() ;
		//Position = Position - pawn->GetActorLocation();
		OutPosition = Position;
		return true;
	}
#endif
	return false;
}

ETrackingStatus FHuaweiVRController::GetControllerTrackingStatus(const int32 ControllerIndex, const EControllerHand DeviceHand) const
{
#if HVRCONTROLLER_SUPPORTED_ANDROID_PLATFORMS
	bool isAvailable = false;
	void* mCurrentController = NULL;
	if (DeviceHand == EControllerHand::Left)
	{
		mCurrentController = mLeftController;
	}
	if (DeviceHand == EControllerHand::Right)
	{
		mCurrentController = mRightController;
	}
	if (NULL != mCurrentController) {
		isAvailable = HVR_Android_IsControllerAvailable(mCurrentController);
	}
	if (isAvailable)
	{
		return ETrackingStatus::Tracked;
	}
#endif

	return ETrackingStatus::NotTracked;
}

HVR_CONTROLLER_TYPE FHuaweiVRController::GetHVRControllerType(const EControllerHand DeviceHand)
{
#if HVRCONTROLLER_SUPPORTED_ANDROID_PLATFORMS
	void* mCurrentController = NULL;
	if (DeviceHand == EControllerHand::Left)
	{
		mCurrentController = mLeftController;
	}
	if (DeviceHand == EControllerHand::Right)
	{
		mCurrentController = mRightController;
	}
	if (NULL != mCurrentController)
	{
		return HVR_Android_GetControllerType(mCurrentController);
	}
#endif

	return HVR_CONTROLLER_3DOF;
}


int FHuaweiVRController::StartHVRControllerVibration(const EControllerHand DeviceHand, float amplitude, uint16_t durationTimeMs, float frequency, HVR_Controller_WaveformType waveform)
{
#if HVRCONTROLLER_SUPPORTED_ANDROID_PLATFORMS
	void* mCurrentController = NULL;
	if (DeviceHand == EControllerHand::Left)
	{
		mCurrentController = mLeftController;
	}
	if (DeviceHand == EControllerHand::Right)
	{
		mCurrentController = mRightController;
	}
	if (NULL != mCurrentController)
	{
		return HVR_Android_StartControllerVibration(mCurrentController, amplitude, durationTimeMs, frequency, waveform);
	}
#endif

	return -1;
}


int FHuaweiVRController::StopHVRControllerVibration(const EControllerHand DeviceHand)
{
#if HVRCONTROLLER_SUPPORTED_ANDROID_PLATFORMS
	void* mCurrentController = NULL;
	if (DeviceHand == EControllerHand::Left)
	{
		mCurrentController = mLeftController;
	}
	if (DeviceHand == EControllerHand::Right)
	{
		mCurrentController = mRightController;
	}
	if (NULL != mCurrentController)
	{
		return HVR_Android_StopControllerVibration(mCurrentController);
	}
#endif

	return -1;
}
