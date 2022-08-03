// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HVRControllerFunctionLibrary.generated.h"

UENUM(BlueprintType, Category = "HuaweiVRController")
enum class HVRControllerMessageType : uint8
{
	HVRControllerMessage_CaptureImageSucceed          UMETA(DisplayName = "CaptureImageSucceed"),
	HVRControllerMessage_NoControllerJoin             UMETA(DisplayName = "NoControllerJoin"),
	HVRControllerMessage_ControllerDisconnect         UMETA(DisplayName = "ControllerDisconnect"),
	HVRControllerMessage_CaptureImageFailed           UMETA(DisplayName = "CaptureImageFailed"),
	HVRControllerMessage_BluetoothDisable             UMETA(DisplayName = "BluetoothDisable"),
	HVRControllerMessage_ControllerConnected          UMETA(DisplayName = "ControllerConnected"),
};

UENUM(BlueprintType, Category = "HuaweiVRController")
enum class HVRControllerType : uint8
{
	HVRController_3DOF           UMETA(DisplayName = "3DOF"),
	HVRController_6DOF           UMETA(DisplayName = "6DOF"),
	HVRController_Gaming         UMETA(DisplayName = "Gaming"),
	HVRController_SysStd         UMETA(DisplayName = "SystemStandard"),
	HVRController_Other          UMETA(DisplayName = "Other"),
};

UENUM(BlueprintType, Category = "HuaweiVRController")
enum class HVRControllerWaveformType  : uint8
{
	HVRControllerStandardWaveform    UMETA(DisplayName =" Standard "),
};


/**
 *
 */
UCLASS()
class HUAWEIVRCONTROLLER_API UHVRControllerFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "HuaweiVRController", meta = (Keywords = "HuaweiVR"))
        static bool IsLeftHandMode();

	UFUNCTION(BlueprintPure, Category = "HuaweiVRController", meta = (Keywords = "HuaweiVR"))
		static FRotator GetHVRControllerOrientation(const EControllerHand DeviceHand);

	UFUNCTION(BlueprintCallable, Category = "HuaweiVRController", meta = (Keywords = "HuaweiVR"))
		static void GetHVRControllerOrientationAndPosition(const EControllerHand DeviceHand, FRotator& Orientation, FVector& Position);

	UFUNCTION(BlueprintPure, Category = "HuaweiVRController", meta = (Keywords = "HuaweiVR"))
		static FVector2D GetHVRControllerTouchpadTouchPos(const EControllerHand DeviceHand);

	UFUNCTION(BlueprintPure, Category = "HuaweiVRController", meta = (Keywords = "HuaweiVR"))
		static FVector2D GetHVRControllerThumbstickPos(const EControllerHand DeviceHand);

	UFUNCTION(BlueprintPure, Category = "HuaweiVRController", meta = (Keywords = "HuaweiVR"))
		static bool IsAvailable(const EControllerHand DeviceHand);

	UFUNCTION(BlueprintCallable, Category = "HuaweiVRController", meta = (Keywords = "HuaweiVR"))
		static bool GetHVRControllerMessage(HVRControllerMessageType& type, int& priority, TMap<FString, FString>& message);

	UFUNCTION(BlueprintCallable, Category = "HuaweiVRController", meta = (Keywords = "HuaweiVR"))
		static bool GetHVRControllerType(const EControllerHand DeviceHand, HVRControllerType& type);
	
	UFUNCTION(BlueprintCallable, Category = "HuaweiVRController", meta = (Keywords = "HuaweiVR"))
		static int StartControllerVibration(const EControllerHand DeviceHand, float amplitude, int durationTimeMs, float frequency, HVRControllerWaveformType waveform);//

	UFUNCTION(BlueprintCallable, Category = "HuaweiVRController", meta = (Keywords = "HuaweiVR"))
		static int StopControllerVibration(const EControllerHand DeviceHand);
	
};
