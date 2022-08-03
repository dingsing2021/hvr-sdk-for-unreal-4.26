// Fill out your copyright notice in the Description page of Project Settings.

#include "HVRControllerFunctionLibrary.h"
#include "InputCoreTypes.h"
#include "HuaweiVRController.h"

#if HVRCONTROLLER_SUPPORTED_ANDROID_PLATFORMS
#include <sys/system_properties.h>
#endif

//UHVRControllerFunctionLibrary::UHVRControllerFunctionLibrary(const FObjectInitializer& ObjectInitializer)
//	: Super(ObjectInitializer)
//{
//}

FHuaweiVRController* GetHVRController()
{

	TArray<IMotionController*> MotionControllers = IModularFeatures::Get().GetModularFeatureImplementations<IMotionController>(IMotionController::GetModularFeatureName());
	for (auto MotionController : MotionControllers)
	{
		if (MotionController != nullptr && MotionController->GetMotionControllerDeviceTypeName() == FHuaweiVRController::DeviceTypeName)
		{
			return static_cast<FHuaweiVRController*>(MotionController);
		}
	}

	return nullptr;
}

FRotator UHVRControllerFunctionLibrary::GetHVRControllerOrientation(const EControllerHand DeviceHand)
{
	if (DeviceHand == EControllerHand::Right || DeviceHand == EControllerHand::Left) {
		FHuaweiVRController* HVRController = GetHVRController();
		if (HVRController != nullptr)
		{
			FRotator orientation;
			FVector position;
			HVRController->GetControllerOrientationAndPosition(0, DeviceHand, orientation, position, HVRController->GetWorldToMetersScale());
			return orientation;
		}
		return FRotator::ZeroRotator;
	}
	else {
		return FRotator::ZeroRotator;
	}

}

void UHVRControllerFunctionLibrary::GetHVRControllerOrientationAndPosition(const EControllerHand DeviceHand, FRotator& Orientation, FVector& Position)
{
	if (DeviceHand == EControllerHand::Right || DeviceHand == EControllerHand::Left) {
		FHuaweiVRController* HVRController = GetHVRController();
		if (HVRController != nullptr)
		{
			HVRController->GetControllerOrientationAndPosition(0, DeviceHand, Orientation, Position, HVRController->GetWorldToMetersScale());
			return;
		}
	}
	Orientation = FRotator(0.0f, 0.0f, 0.0f);
	Position = FVector(0.0f, 0.0f, 0.0f);
}

FVector2D UHVRControllerFunctionLibrary::GetHVRControllerTouchpadTouchPos(const EControllerHand DeviceHand) {
	if (DeviceHand == EControllerHand::Right || DeviceHand == EControllerHand::Left) {
		FHuaweiVRController* HVRController = GetHVRController();
		if (HVRController != nullptr)
		{
			return HVRController->GetHVRControllerTouchpadTouchPos(DeviceHand);
		}
		return FVector2D::ZeroVector;
	}
	else {	
		return FVector2D::ZeroVector;
	}
	
}

FVector2D UHVRControllerFunctionLibrary::GetHVRControllerThumbstickPos(const EControllerHand DeviceHand) {
	if (DeviceHand == EControllerHand::Right || DeviceHand == EControllerHand::Left) {
		FHuaweiVRController* HVRController = GetHVRController();
		if (HVRController != nullptr)
		{
			return HVRController->GetHVRControllerThumbstickPos(DeviceHand);
		}
		return FVector2D::ZeroVector;
	}
	else {
		return FVector2D::ZeroVector;
	}

}

bool UHVRControllerFunctionLibrary::IsLeftHandMode() {
#if HVRCONTROLLER_SUPPORTED_ANDROID_PLATFORMS
    const char keyName[PROP_NAME_MAX] = "persist.sys.vr_left_hand_mode";
    char value[PROP_VALUE_MAX] = { 0 };
    if (0 == __system_property_get(keyName, value)) {
        UE_LOG(LogHuaweiVRSDK, Log, TEXT("UHVRControllerFunctionLibrary::IsLeftHandMode __system_property_get failed"));
        return false;
    }
    if (0 == strcmp("1", value)) {
        UE_LOG(LogHuaweiVRSDK, Log, TEXT("UHVRControllerFunctionLibrary::IsLeftHandMode true"));
        return true;
    }
#endif
    UE_LOG(LogHuaweiVRSDK, Log, TEXT("UHVRControllerFunctionLibrary::IsLeftHandMode false"));
    return false;
}


bool UHVRControllerFunctionLibrary::IsAvailable(const EControllerHand DeviceHand) {

	if (DeviceHand == EControllerHand::Right || DeviceHand == EControllerHand::Left) {
		FHuaweiVRController* HVRController = GetHVRController();
		if (HVRController != nullptr)
		{
			return HVRController->IsAvailable(DeviceHand);
		}
		return false;
	}
	else {
		return false;
	}

}

bool UHVRControllerFunctionLibrary::GetHVRControllerMessage(HVRControllerMessageType& type, int& priority, TMap<FString, FString>& message)
{
	int nativeMessageType = -1;
	FHuaweiVRController* HVRController = GetHVRController();
	if (HVRController != nullptr)
	{
		if (HVRController->DetectHVRControllerMessage(nativeMessageType, priority, message))
		{
			if (NotifyEvent_CaptureImageSucceed == nativeMessageType)
			{
				type = HVRControllerMessageType::HVRControllerMessage_CaptureImageSucceed;
				HVRController->mIsDetectMessage = false;
			}
			else if (NotifyEvent_NoControllerJoin == nativeMessageType) {
				type = HVRControllerMessageType::HVRControllerMessage_NoControllerJoin;
				HVRController->mIsDetectMessage = false;
			}
			else if (NotifyEvent_ControllerDisconnect == nativeMessageType) {
				type = HVRControllerMessageType::HVRControllerMessage_ControllerDisconnect;
				HVRController->mIsDetectMessage = false;
			}
			else if (NotifyEvent_CaptureImageFailed == nativeMessageType) {
				type = HVRControllerMessageType::HVRControllerMessage_CaptureImageFailed;
				HVRController->mIsDetectMessage = false;
			}
			else if (NotifyEvent_BluetoothDisable == nativeMessageType) {
				type = HVRControllerMessageType::HVRControllerMessage_BluetoothDisable;
				HVRController->mIsDetectMessage = false;

			}
			else if (NotifyEvent_ControllerConnected == nativeMessageType) {
				type = HVRControllerMessageType::HVRControllerMessage_ControllerConnected;
				HVRController->mIsDetectMessage = false;
			}
			return true;
		}
	}
	return false;
}

bool UHVRControllerFunctionLibrary::GetHVRControllerType(const EControllerHand DeviceHand, HVRControllerType& type)
{
	int controllerType = -1;
	FHuaweiVRController* HVRController = GetHVRController();
	if (HVRController != nullptr) {
		controllerType = HVRController->GetHVRControllerType(DeviceHand);
		if (HVR_CONTROLLER_3DOF == controllerType) {
			type = HVRControllerType::HVRController_3DOF;
		}
		else if (HVR_CONTROLLER_6DOF == controllerType) {
			type = HVRControllerType::HVRController_6DOF;
		}
		else if (HVR_CONTROLLER_Gaming == controllerType) {
			type = HVRControllerType::HVRController_Gaming;
		}
		else if (HVR_CONTROLLER_SysStd == controllerType) {
			type = HVRControllerType::HVRController_SysStd;
		}
		else if (HVR_CONTROLLER_Other == controllerType) {
			type = HVRControllerType::HVRController_Other;
		}
		return true;
	}
	return false;
}

int UHVRControllerFunctionLibrary::StartControllerVibration(const EControllerHand DeviceHand, float amplitude, int durationTimeMs, float frequency, HVRControllerWaveformType waveform)
{
	if (DeviceHand == EControllerHand::Right || DeviceHand == EControllerHand::Left)
	{
		FHuaweiVRController* HVRController = GetHVRController();
		if (HVRController != nullptr)
		{
			HVR_Controller_WaveformType waveformType = (HVR_Controller_WaveformType) waveform;
			return HVRController->StartHVRControllerVibration(DeviceHand, amplitude, durationTimeMs, frequency, waveformType);
		}
		return -1;
	}
	else {
		return -1;
	}
}

int UHVRControllerFunctionLibrary::StopControllerVibration(const EControllerHand DeviceHand)
{
	if (DeviceHand == EControllerHand::Right || DeviceHand == EControllerHand::Left) {
		FHuaweiVRController* HVRController = GetHVRController();
		if (HVRController != nullptr)
		{
			return HVRController->StopHVRControllerVibration(DeviceHand);
		}
		return -1;
	}
	else {
		return -1;
	}
}
