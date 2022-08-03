// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
//为了跟蓝图的交互才定义的方法
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HuaweiVRSDKBPFunctionLibrary.generated.h"

class FHuaweiVRSDK;

UENUM(BlueprintType, Category = "HuaweiVRSDK")
enum class HuaweiVRMessageType : uint8 {
    HuaweiVRMessage_NetWorkState UMETA(DisplayName = "NetWorkState"),
    HuaweiVRMessage_ThermalWarning UMETA(DisplayName = "ThermalWarning"),
    HuaweiVRMessage_NewSMS UMETA(DisplayName = "NewSMS"),
    HuaweiVRMessage_PhoneLowPower UMETA(DisplayName = "PhoneLowPower"),
    HuaweiVRMessage_HelmetLowPower UMETA(DisplayName = "HelmetLowPower"),
    HuaweiVRMessage_Common UMETA(DisplayName = "Common"),
};

UENUM(BlueprintType, Category = "HuaweiVRSDK")
enum class HuaweiVRHelmetModel : uint8 {
    HuaweiVRHelmet_FIRST_GEN UMETA(DisplayName = "FirstGeneration"),
    HuaweiVRHelmet_SECOND_GEN UMETA(DisplayName = "SecondGeneration"),
	HuaweiVRHelmet_Third_GEN UMETA(DisplayName = "ThirdGeneration"),
    HuaweiVRHelmet_NOT_FOUND UMETA(DisplayName = "NotFound"),
    HuaweiVRHelmet_UNKNOWN UMETA(DisplayName = "Unknown"),
};

/* *
 *
 */
UCLASS()
class HUAWEIVRSDK_API UHuaweiVRSDKBPFunctionLibrary : public UBlueprintFunctionLibrary {
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "HuaweiVRSDK|Private")
    static bool GetHuaweiVRMessage(HuaweiVRMessageType& type, int& priority, int& boxType,
        TMap<FString, FString>& message);

    UFUNCTION(BlueprintCallable, Category = "HuaweiVRSDK|Common")
    static FString GetHuaweiVRSDKVersion();

    UFUNCTION(BlueprintCallable, Category = "HuaweiVRSDK|Helmet")
    static int GetHMDInfo(HuaweiVRHelmetModel& helmetModel);

    UFUNCTION(BlueprintCallable, Category = "HuaweiVRSDK|Helmet")
    static void GetHMDOrientationAndPosition(FQuat& Orientation, FVector& Position);

    UFUNCTION(BlueprintCallable, Category = "HuaweiVRSDK|Helmet")
    static int ResetHMDYaw();

    UFUNCTION(BlueprintCallable, Category = "HuaweiVRSDK|Helmet")
    static int SetHMDPoseLock(bool enable);

    UFUNCTION(BlueprintCallable, Category = "HuaweiVRSDK|Render")
    static int SetChromaticAberration(bool enable);

    UFUNCTION(BlueprintCallable, Category = "HuaweiVRSDK|Audio")
    static int SetSvsEffect(bool enable);

};